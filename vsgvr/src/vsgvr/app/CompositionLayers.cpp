/*
Copyright(c) 2022 Gareth Francis

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vsgvr/app/CompositionLayers.h>
#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/Swapchain.h>
#include <vsgvr/xr/Session.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>

#include <vsg/core/Exception.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/vk/SubmitCommands.h>

#include "../xr/Macros.cpp"

namespace vsgvr {
  CompositionLayer::CompositionLayer() {}
  CompositionLayer::~CompositionLayer() {}


  void CompositionLayer::createSwapchains(vsg::ref_ptr<vsgvr::Session> session,vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> graphicsBinding, vsg::ref_ptr<vsgvr::Traits> xrTraits, std::vector< SwapchainImageRequirements> imageRequirements)
  {
    if (!_viewData.empty()) throw vsg::Exception({ "Swapchain already initialised" });

    for (auto& viewConfig : imageRequirements)
    {
      PerViewData v;
      v.swapchain = Swapchain::create(session->getSession(), xrTraits->swapchainFormat, viewConfig.width, viewConfig.height, viewConfig.sampleCount, graphicsBinding);

      auto extent = v.swapchain->getExtent();
      VkSampleCountFlagBits framebufferSamples;
      switch (viewConfig.sampleCount)
      {
      case 1:
        framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
      case 2:
        framebufferSamples = VK_SAMPLE_COUNT_2_BIT;
        break;
      case 4:
        framebufferSamples = VK_SAMPLE_COUNT_4_BIT;
        break;
      case 8:
        framebufferSamples = VK_SAMPLE_COUNT_8_BIT;
        break;
      case 16:
        framebufferSamples = VK_SAMPLE_COUNT_16_BIT;
        break;
      case 32:
        framebufferSamples = VK_SAMPLE_COUNT_32_BIT;
        break;
      case 64:
        framebufferSamples = VK_SAMPLE_COUNT_64_BIT;
        break;
      default:
        framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
      }

      bool multisampling = framebufferSamples != VK_SAMPLE_COUNT_1_BIT;
      if (multisampling)
      {
        v.multisampleImage = vsg::Image::create();
        v.multisampleImage->imageType = VK_IMAGE_TYPE_2D;
        v.multisampleImage->format = xrTraits->swapchainFormat;
        v.multisampleImage->extent.width = extent.width;
        v.multisampleImage->extent.height = extent.height;
        v.multisampleImage->extent.depth = 1;
        v.multisampleImage->mipLevels = 1;
        v.multisampleImage->arrayLayers = 1;
        v.multisampleImage->samples = framebufferSamples;
        v.multisampleImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        v.multisampleImage->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        v.multisampleImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        v.multisampleImage->flags = 0;
        v.multisampleImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        v.multisampleImage->compile(graphicsBinding->getVkDevice());
        v.multisampleImage->allocateAndBindMemory(graphicsBinding->getVkDevice());

        v.multisampleImageView = vsg::ImageView::create(v.multisampleImage, VK_IMAGE_ASPECT_COLOR_BIT);
        v.multisampleImageView->compile(graphicsBinding->getVkDevice());
      }

      bool requiresDepthRead = false; // (_traits->depthImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;

      // Window::_initRenderPass
      // TODO: Test single & multisampled rendering
      {
        if (framebufferSamples == VK_SAMPLE_COUNT_1_BIT)
        {
          v.renderPass = vsg::createRenderPass(graphicsBinding->getVkDevice(), xrTraits->swapchainFormat, VK_FORMAT_D32_SFLOAT, requiresDepthRead);
        }
        else
        {
          v.renderPass = vsg::createMultisampledRenderPass(graphicsBinding->getVkDevice(), xrTraits->swapchainFormat, VK_FORMAT_D32_SFLOAT, framebufferSamples, requiresDepthRead);
        }
      }

      bool requiresDepthResolve = (multisampling && requiresDepthRead);

      // create depth buffer
      v.depthImage = vsg::Image::create();
      v.depthImage->imageType = VK_IMAGE_TYPE_2D;
      v.depthImage->extent.width = extent.width;
      v.depthImage->extent.height = extent.height;
      v.depthImage->extent.depth = 1;
      v.depthImage->mipLevels = 1;
      v.depthImage->arrayLayers = 1;
      v.depthImage->format = VK_FORMAT_D32_SFLOAT; // _depthFormat;
      v.depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
      v.depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      v.depthImage->samples = framebufferSamples;
      v.depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      v.depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // _traits->depthImageUsage;

      v.depthImage->compile(graphicsBinding->getVkDevice());
      v.depthImage->allocateAndBindMemory(graphicsBinding->getVkDevice());

      v.depthImageView = vsg::ImageView::create(v.depthImage);
      v.depthImageView->compile(graphicsBinding->getVkDevice());

      if (requiresDepthResolve)
      {
        v.multisampleDepthImage = v.depthImage;
        v.multisampleDepthImageView = v.depthImageView;

        // create depth buffer
        v.depthImage = vsg::Image::create();
        v.depthImage->imageType = VK_IMAGE_TYPE_2D;
        v.depthImage->extent.width = extent.width;
        v.depthImage->extent.height = extent.height;
        v.depthImage->extent.depth = 1;
        v.depthImage->mipLevels = 1;
        v.depthImage->arrayLayers = 1;
        v.depthImage->format = VK_FORMAT_D32_SFLOAT; // _depthFormat;
        v.depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        v.depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        v.depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // _traits->depthImageUsage;
        v.depthImage->samples = VK_SAMPLE_COUNT_1_BIT;
        v.depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        v.depthImage->compile(graphicsBinding->getVkDevice());
        v.depthImage->allocateAndBindMemory(graphicsBinding->getVkDevice());

        v.depthImageView = vsg::ImageView::create(v.depthImage);
        v.depthImageView->compile(graphicsBinding->getVkDevice());
      }

      int graphicsFamily = graphicsBinding->getVkPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);

      // set up framebuffer and associated resources
      const auto& imageViews = v.swapchain->getImageViews();

      for (size_t i = 0; i < imageViews.size(); ++i)
      {
        vsg::ImageViews attachments;
        if (v.multisampleImageView)
        {
          attachments.push_back(v.multisampleImageView);
        }
        attachments.push_back(imageViews[i]);

        if (v.multisampleDepthImageView)
        {
          attachments.push_back(v.multisampleDepthImageView);
        }
        attachments.push_back(v.depthImageView);

        vsg::ref_ptr<vsg::Framebuffer> fb = vsg::Framebuffer::create(v.renderPass, attachments, extent.width, extent.height, 1);

        v.frames.push_back({ imageViews[i], fb });
      }

      {
        // ensure image attachments are setup on GPU.
        vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(graphicsBinding->getVkDevice(), graphicsFamily);
        submitCommandsToQueue(commandPool, graphicsBinding->getVkDevice()->getQueue(graphicsFamily), [&](vsg::CommandBuffer& commandBuffer) {
          auto depthImageBarrier = vsg::ImageMemoryBarrier::create(
            0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            v.depthImage,
            v.depthImageView->subresourceRange);

          auto pipelineBarrier = vsg::PipelineBarrier::create(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0, depthImageBarrier);
          pipelineBarrier->record(commandBuffer);

          if (multisampling)
          {
            auto msImageBarrier = vsg::ImageMemoryBarrier::create(
              0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
              VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
              v.multisampleImage,
              VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
            auto msPipelineBarrier = vsg::PipelineBarrier::create(
              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              0, msImageBarrier);
            msPipelineBarrier->record(commandBuffer);
          }
          });
      }

      _viewData.push_back(std::move(v));
    }
  }

  void CompositionLayer::destroySwapchains()
  {
    _viewData.clear();
  }


  CompositionLayerProjection::CompositionLayerProjection() {}
  CompositionLayerProjection::CompositionLayerProjection(XrCompositionLayerFlags inFlags)
    : flags(inFlags)
  {}
  CompositionLayerProjection::~CompositionLayerProjection() {}
  XrCompositionLayerProjection CompositionLayerProjection::getCompositionLayer() const
  {
    XrCompositionLayerProjection layer;
    layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    layer.next = nullptr;
    layer.layerFlags = flags;
    return std::move(layer);
  }

  void CompositionLayerProjection::populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
  {
    if (!_viewConfigurationViews.empty())
    {
      // This is fine - User may have called manually
      // TODO: Could have more flexible layer width/height selection here, etc.
      return;
    }

    _viewConfigurationProperties = XrViewConfigurationProperties();
    _viewConfigurationProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
    _viewConfigurationProperties.next = nullptr;

    xr_check(xrGetViewConfigurationProperties(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, &_viewConfigurationProperties));
    uint32_t count = 0;
    xr_check(xrEnumerateViewConfigurationViews(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, 0, &count, nullptr));
    _viewConfigurationViews.resize(count, XrViewConfigurationView());
    for (auto& v : _viewConfigurationViews) {
      v.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
      v.next = nullptr;
    }
    xr_check(xrEnumerateViewConfigurationViews(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, static_cast<uint32_t>(_viewConfigurationViews.size()), &count, _viewConfigurationViews.data()));
  }

  std::vector<CompositionLayer::SwapchainImageRequirements> CompositionLayerProjection::getSwapchainImageRequirements()
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    for (auto& config : _viewConfigurationViews)
    {
      reqs.push_back({
        config.recommendedImageRectHeight,
        config.recommendedImageRectHeight,
        config.recommendedSwapchainSampleCount
      });
    }
    return reqs;
  }

  CompositionLayerQuad::CompositionLayerQuad() {}
  CompositionLayerQuad::CompositionLayerQuad(XrPosef inPose, XrExtent2Df inSize, XrCompositionLayerFlags inFlags, XrEyeVisibility inEyeVisibility)
    : pose(inPose)
    , size(inSize)
    , flags(inFlags)
    , eyeVisibility(inEyeVisibility)
  {}
  CompositionLayerQuad::~CompositionLayerQuad() {}
  XrCompositionLayerQuad CompositionLayerQuad::getCompositionLayer() const
  {
    XrCompositionLayerQuad layer;
    layer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
    layer.next = nullptr;
    layer.layerFlags = flags;
    layer.eyeVisibility = eyeVisibility;
    layer.pose = pose;
    layer.size = size;
    return std::move(layer);
  }

  void CompositionLayerQuad::populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
  {
    // TODO: What to put here? Perhaps it's entirely up to the user in this case, since they're just rendering <a quad> in space
    //       Image size for swapchain of course matters, but that'll just be the image resolution that's passed to OpenXR, before
    //       composition - There's nothing in OpenXR to recommend a size for us here.
  }

  std::vector<CompositionLayer::SwapchainImageRequirements> CompositionLayerQuad::getSwapchainImageRequirements()
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    
    reqs.push_back({1920, 1080, 1});
    return reqs;
  }
}
