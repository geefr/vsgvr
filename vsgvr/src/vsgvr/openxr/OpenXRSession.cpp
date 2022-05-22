#include <vsgvr/openxr/OpenXRSession.h>

#include <iostream>

#include <vsg/core/Exception.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/vk/SubmitCommands.h>

#include "OpenXRMacros.cpp"

using namespace vsg;

namespace vsgvr {

  OpenXRSession::OpenXRSession(XrInstance instance, XrSystemId system, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding,
                               VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs)
    : _graphicsBinding(graphicsBinding)
  {
    createSession(instance, system);
    createSwapchain(swapchainFormat, viewConfigs);
  }

  OpenXRSession::~OpenXRSession()
  {
    destroySwapchain();
    destroySession();
  }

  // TODO: A regular session update is needed - should handle the session lifecycle and such
  // void update(XrInstance instance, XrSystemId system);

  void OpenXRSession::createSession(XrInstance instance, XrSystemId system)
  {
    auto info = XrSessionCreateInfo();
    info.type = XR_TYPE_SESSION_CREATE_INFO;
    info.next = &_graphicsBinding->getBinding();
    info.systemId = system;

    xr_check(xrCreateSession(instance, &info, &_session), "Failed to create OpenXR session");
    _sessionState = XR_SESSION_STATE_IDLE;
  }

  void OpenXRSession::createSwapchain(VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs)
  {
    if( _swapchain ) throw Exception({"Swapchain already initialised"});
    if( !_session ) throw Exception({"Unable to create swapchain without session"});
    _swapchain = OpenXRSwapchain::create(_session, swapchainFormat, viewConfigs, _graphicsBinding);

    // Window::buildSwapchain
    // TODO: This will need rework..
    if (_swapchain)
    {
      // make sure all operations on the device have stopped before we go deleting associated resources
      vkDeviceWaitIdle(_graphicsBinding->getVkDevice()->getDevice());

      // clean up previous swap chain before we begin creating a new one.
      _frames.clear();

      // _depthImageView = 0;
      _depthImage = 0;

      _multisampleImage = 0;
      _multisampleImageView = 0;
    }

    // is width and height even required here as the surface appear to control it.
    // _swapchain = Swapchain::create(_physicalDevice, _device, _surface, _extent2D.width, _extent2D.height, _traits->swapchainPreferences, _swapchain);

    auto _extent2D = _swapchain->getExtent();
    VkSampleCountFlagBits _framebufferSamples;
    switch (viewConfigs[0].recommendedSwapchainSampleCount)
    {
       case 1:
         _framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
         break;
       case 2:
         _framebufferSamples = VK_SAMPLE_COUNT_2_BIT;
         break;
       case 4:
         _framebufferSamples = VK_SAMPLE_COUNT_4_BIT;
         break;
       case 8:
         _framebufferSamples = VK_SAMPLE_COUNT_8_BIT;
         break;
       case 16:
         _framebufferSamples = VK_SAMPLE_COUNT_16_BIT;
         break;
       case 32:
         _framebufferSamples = VK_SAMPLE_COUNT_32_BIT;
         break;
       case 64:
         _framebufferSamples = VK_SAMPLE_COUNT_64_BIT;
         break;
    }

    bool multisampling = _framebufferSamples != VK_SAMPLE_COUNT_1_BIT;
    if (multisampling)
    {
      _multisampleImage = Image::create();
      _multisampleImage->imageType = VK_IMAGE_TYPE_2D;
      _multisampleImage->format = swapchainFormat;
      _multisampleImage->extent.width = _extent2D.width;
      _multisampleImage->extent.height = _extent2D.height;
      _multisampleImage->extent.depth = 1;
      _multisampleImage->mipLevels = 1;
      _multisampleImage->arrayLayers = 1;
      _multisampleImage->samples = _framebufferSamples;
      _multisampleImage->tiling = VK_IMAGE_TILING_OPTIMAL;
      _multisampleImage->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      _multisampleImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      _multisampleImage->flags = 0;
      _multisampleImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      _multisampleImage->compile(_graphicsBinding->getVkDevice());
      _multisampleImage->allocateAndBindMemory(_graphicsBinding->getVkDevice());

      _multisampleImageView = ImageView::create(_multisampleImage, VK_IMAGE_ASPECT_COLOR_BIT);
      _multisampleImageView->compile(_graphicsBinding->getVkDevice());
    }

    bool requiresDepthRead = false; // (_traits->depthImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;

    // Window::_initRenderPass
    {
      if (_framebufferSamples == VK_SAMPLE_COUNT_1_BIT)
      {
        _renderPass = vsg::createRenderPass(_graphicsBinding->getVkDevice(), swapchainFormat, VK_FORMAT_D32_SFLOAT, requiresDepthRead);
      }
      else
      {
        _renderPass = vsg::createMultisampledRenderPass(_graphicsBinding->getVkDevice(), swapchainFormat, VK_FORMAT_D32_SFLOAT, _framebufferSamples, requiresDepthRead);
      }
    }

    bool requiresDepthResolve = (multisampling && requiresDepthRead);

    // create depth buffer
    _depthImage = Image::create();
    _depthImage->imageType = VK_IMAGE_TYPE_2D;
    _depthImage->extent.width = _extent2D.width;
    _depthImage->extent.height = _extent2D.height;
    _depthImage->extent.depth = 1;
    _depthImage->mipLevels = 1;
    _depthImage->arrayLayers = 1;
    _depthImage->format = VK_FORMAT_D32_SFLOAT; // _depthFormat;
    _depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
    _depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _depthImage->samples = _framebufferSamples;
    _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // _traits->depthImageUsage;

    _depthImage->compile(_graphicsBinding->getVkDevice());
    _depthImage->allocateAndBindMemory(_graphicsBinding->getVkDevice());

    _depthImageView = ImageView::create(_depthImage);
    _depthImageView->compile(_graphicsBinding->getVkDevice());

    if (requiresDepthResolve)
    {
      _multisampleDepthImage = _depthImage;
      _multisampleDepthImageView = _depthImageView;

      // create depth buffer
      _depthImage = Image::create();
      _depthImage->imageType = VK_IMAGE_TYPE_2D;
      _depthImage->extent.width = _extent2D.width;
      _depthImage->extent.height = _extent2D.height;
      _depthImage->extent.depth = 1;
      _depthImage->mipLevels = 1;
      _depthImage->arrayLayers = 1;
      _depthImage->format = VK_FORMAT_D32_SFLOAT; // _depthFormat;
      _depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
      _depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      _depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // _traits->depthImageUsage;
      _depthImage->samples = VK_SAMPLE_COUNT_1_BIT;
      _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      _depthImage->compile(_graphicsBinding->getVkDevice());
      _depthImage->allocateAndBindMemory(_graphicsBinding->getVkDevice());

      _depthImageView = ImageView::create(_depthImage);
      _depthImageView->compile(_graphicsBinding->getVkDevice());
    }

    int graphicsFamily = _graphicsBinding->getVkPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);

    // set up framebuffer and associated resources
    auto& imageViews = _swapchain->getImageViews();

    size_t initial_indexValue = imageViews.size();
    for (size_t i = 0; i < imageViews.size(); ++i)
    {
      vsg::ImageViews attachments;
      if (_multisampleImageView)
      {
        attachments.push_back(_multisampleImageView);
      }
      attachments.push_back(imageViews[i]);

      if (_multisampleDepthImageView)
      {
        attachments.push_back(_multisampleDepthImageView);
      }
      attachments.push_back(_depthImageView);

      ref_ptr<Framebuffer> fb = Framebuffer::create(_renderPass, attachments, _extent2D.width, _extent2D.height, 1);

      _frames.push_back({ imageViews[i], fb });
    }

    {
      // ensure image attachments are setup on GPU.
      ref_ptr<CommandPool> commandPool = CommandPool::create(_graphicsBinding->getVkDevice(), graphicsFamily);
      submitCommandsToQueue(_graphicsBinding->getVkDevice(), commandPool, _graphicsBinding->getVkDevice()->getQueue(graphicsFamily), [&](CommandBuffer& commandBuffer) {
        auto depthImageBarrier = ImageMemoryBarrier::create(
          0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
          _depthImage,
          _depthImageView->subresourceRange);

        auto pipelineBarrier = PipelineBarrier::create(
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
          0, depthImageBarrier);
        pipelineBarrier->record(commandBuffer);

        if (multisampling)
        {
          auto msImageBarrier = ImageMemoryBarrier::create(
            0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            _multisampleImage,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
          auto msPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, msImageBarrier);
          msPipelineBarrier->record(commandBuffer);
        }
        });
    }
  }

  void OpenXRSession::destroySwapchain()
  {
    _frames.clear();
    _depthImage = nullptr;
    _depthImageView = nullptr;
    _multisampleImage = nullptr;
    _multisampleImageView = nullptr;
    _multisampleDepthImage = nullptr;
    _multisampleDepthImageView = nullptr;
    _renderPass = nullptr;
    _swapchain = nullptr;
  }

  void OpenXRSession::destroySession()
  {
    xr_check(xrDestroySession(_session));
    _session = nullptr;
  }

  void OpenXRSession::beginSession(XrViewConfigurationType viewConfigurationType)
  {
    if (_sessionRunning) return;
    auto info = XrSessionBeginInfo();
    info.type = XR_TYPE_SESSION_BEGIN_INFO;
    info.next = nullptr;
    info.primaryViewConfigurationType = viewConfigurationType;
    xr_check(xrBeginSession(_session, &info), "Failed to begin session");
    _sessionRunning = true;
  }

  void OpenXRSession::endSession()
  {
    if (!_sessionRunning) return;
    xr_check(xrEndSession(_session), "Failed to end session");
    _sessionRunning = false;
  }

  void OpenXRSession::onEventStateChanged(const XrEventDataSessionStateChanged& event)
  {
    // TODO: Update _sessionState, handle the state change properly (Will be polled for in update?)
    //       Should the session trasition logic be here, or out in Instance? (Should Instance be renamed? It's more like a Viewer)
    _sessionState = event.state;
    std::cerr << "Session state changed: " << to_string(_sessionState) << std::endl;
  }

}