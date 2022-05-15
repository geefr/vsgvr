/*
Copyright(c) 2022 Gareth Francis
Based on vsg::Window, Copyright(c) 2018 Robert Osfield

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

#include <vsg/commands/PipelineBarrier.h>
#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/SubmitCommands.h>

#include <vsgvr/openxr/OpenXRWindow.h>

#include <array>
#include <chrono>

using namespace vsg;

namespace vsgvr
{
    OpenXRWindow::OpenXRWindow(ref_ptr<WindowTraits> traits) 
      : Inherit(traits)
    {
    }

    OpenXRWindow::~OpenXRWindow()
    {
    }

    void OpenXRWindow::clear()
    {
        _frames.clear();
        _swapchain = 0;
        _xrSwapchain = 0;

        _depthImage = 0;
        _depthImageView = 0;

        _renderPass = 0;
        _device = 0;
        _physicalDevice = 0;
    }

    void OpenXRWindow::_initSurface()
    {
      // No surface in OpenXR
    }

    void OpenXRWindow::_initFormats()
    {
        auto supportedFormats = _xr->enumerateSwapchainFormats();
        // "Texture formats should be in order from highest to lowest runtime preference. 
        //  The application should use the highest preference format that it supports for optimal performance and quality."
        if (supportedFormats.empty()) throw Exception({ "No swapchain formats supported by OpenXR runtime" });

        // TODO: This is almost certainly wrong
        _xrSwapchainFormat = VK_FORMAT_UNDEFINED;
        std::pair<VkFormat,bool> validFormats [] = {
          {VK_FORMAT_R8G8B8A8_SRGB, true}
        };
        for (auto& p : validFormats)
        {
          if (std::find(supportedFormats.begin(), supportedFormats.end(), p.first) != supportedFormats.end())
          {
            _xrSwapchainFormat = p.first;
            _imageFormat.format = p.first;
            _imageFormat.colorSpace = p.second ? VK_COLORSPACE_SRGB_NONLINEAR_KHR : VK_COLOR_SPACE_PASS_THROUGH_EXT;
            break;
          }
        }
        if (_xrSwapchainFormat == VK_FORMAT_UNDEFINED)
        {
          throw Exception({"Failed to select XR swapchain format"});
        }

        // TODO: Won't be submitting depth to Xr, so choose sensible / from traits
        _depthFormat = VK_FORMAT_D32_SFLOAT;

        // compute the sample bits to use
        if (_traits->samples != VK_SAMPLE_COUNT_1_BIT)
        {
            VkSampleCountFlags deviceColorSamples = _physicalDevice->getProperties().limits.framebufferColorSampleCounts;
            VkSampleCountFlags deviceDepthSamples = _physicalDevice->getProperties().limits.framebufferDepthSampleCounts;
            VkSampleCountFlags satisfied = deviceColorSamples & deviceDepthSamples & _traits->samples;
            if (satisfied != 0)
            {
                uint32_t highest = 1 << static_cast<uint32_t>(floor(log2(satisfied)));
                _framebufferSamples = static_cast<VkSampleCountFlagBits>(highest);
            }
            else
            {
                _framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
            }
        }
        else
        {
            _framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
        }
    }

    void OpenXRWindow::_initInstance()
    {
      // TODO: Currently set explicitly by OpenXRContext
    }

    void OpenXRWindow::_initPhysicalDevice()
    {
      // TODO: Currently set explicitly by OpenXRContext 
    }

    void OpenXRWindow::_initDevice()
    {
      // TODO: Currently set explicitly by OpenXRContext 
    }

    void OpenXRWindow::buildSwapchain()
    {
        if (_xrSwapchain)
        {
            // make sure all operations on the device have stopped before we go deleting associated resources
            vkDeviceWaitIdle(*_device);

            // clean up previous swap chain before we begin creating a new one.
            _frames.clear();
            _indices.clear();

            _depthImageView = 0;
            _depthImage = 0;

            _multisampleImage = 0;
            _multisampleImageView = 0;
        }

        _xrSwapchain = OpenXRSwapchain::create(_xr, _physicalDevice, _device, _xrSwapchainFormat, _framebufferSamples);
        _extent2D = _xrSwapchain->getExtent();

        bool multisampling = _framebufferSamples != VK_SAMPLE_COUNT_1_BIT;
        if (multisampling)
        {
            _multisampleImage = Image::create();
            _multisampleImage->imageType = VK_IMAGE_TYPE_2D;
            _multisampleImage->format = _imageFormat.format;
            _multisampleImage->extent.width = _extent2D.width;
            _multisampleImage->extent.height = _extent2D.height;
            _multisampleImage->extent.depth = 1;
            _multisampleImage->mipLevels = 1;
            _multisampleImage->arrayLayers = 1;
            _multisampleImage->samples = _framebufferSamples;
            _multisampleImage->tiling = VK_IMAGE_TILING_OPTIMAL;
            _multisampleImage->usage = _traits->swapchainPreferences.imageUsage;
            _multisampleImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            _multisampleImage->flags = 0;
            _multisampleImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            _multisampleImage->compile(_device);
            _multisampleImage->allocateAndBindMemory(_device);

            _multisampleImageView = ImageView::create(_multisampleImage, VK_IMAGE_ASPECT_COLOR_BIT);
            _multisampleImageView->compile(_device);
        }

        bool requiresDepthRead = (_traits->depthImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;
        bool requiresDepthResolve = (multisampling && requiresDepthRead);

        // create depth buffer
        _depthImage = Image::create();
        _depthImage->imageType = VK_IMAGE_TYPE_2D;
        _depthImage->extent.width = _extent2D.width;
        _depthImage->extent.height = _extent2D.height;
        _depthImage->extent.depth = 1;
        _depthImage->mipLevels = 1;
        _depthImage->arrayLayers = 1;
        _depthImage->format = _depthFormat;
        _depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        _depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _depthImage->samples = _framebufferSamples;
        _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        _depthImage->usage = _traits->depthImageUsage;

        _depthImage->compile(_device);
        _depthImage->allocateAndBindMemory(_device);

        _depthImageView = ImageView::create(_depthImage);
        _depthImageView->compile(_device);

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
            _depthImage->format = _depthFormat;
            _depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
            _depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            _depthImage->usage = _traits->depthImageUsage;
            _depthImage->samples = VK_SAMPLE_COUNT_1_BIT;
            _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            _depthImage->compile(_device);
            _depthImage->allocateAndBindMemory(_device);

            _depthImageView = ImageView::create(_depthImage);
            _depthImageView->compile(_device);
        }

        int graphicsFamily = -1;
        graphicsFamily = _physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);

        // set up framebuffer and associated resources
        auto &imageViews = _xrSwapchain->getImageViews();

        _availableSemaphore = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

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

            ref_ptr<Semaphore> ias = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

            //_frames.push_back({multisampling ? _multisampleImageView : imageViews[i], fb, ias});
            _frames.push_back({imageViews[i], fb, ias});
            _indices.push_back(initial_indexValue);
        }

        {
            // ensure image attachments are setup on GPU.
            ref_ptr<CommandPool> commandPool = CommandPool::create(_device, graphicsFamily);
            submitCommandsToQueue(_device, commandPool, _device->getQueue(graphicsFamily), [&](CommandBuffer &commandBuffer)
                                  {
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
                    VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
                auto msPipelineBarrier = PipelineBarrier::create(
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0, msImageBarrier);
                msPipelineBarrier->record(commandBuffer);
            } });
        }
    }

    VkResult OpenXRWindow::acquireNextImage(uint64_t timeout)
    {
        if (!_xrSwapchain)
            _initSwapchain();

        if (!_availableSemaphore)
            _availableSemaphore = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

        // TODO: Hack
        if (_xrSwapchainImageAcquired)
        {
          _xrSwapchain->releaseImage(_xrSwapchainImage);
          _xrSwapchainImageAcquired = false;
        }

        if (_xrSwapchain->acquireNextImage(_xrSwapchainImage) && _xrSwapchain->waitImage(timeout, _xrSwapchainImage))
        {
          _xrSwapchainImageAcquired = true;
          // TODO: The semaphores won't be used by the xr swapchain.

          // the acquired image's semaphore must be available now so make it the new _availableSemaphore and set it's entry to the one to use of the next frame by swapping ref_ptr<>'s
          _availableSemaphore.swap(_frames[_xrSwapchainImage].imageAvailableSemaphore);

          // shift up previous frame indices
          for (size_t i = _indices.size() - 1; i > 0; --i)
          {
            _indices[i] = _indices[i - 1];
          }

          // update head of _indices to the new frames imageIndex
          _indices[0] = _xrSwapchainImage;
        }
        else
        {
          // TODO: Need to think about what should happen on failure
          return VK_ERROR_UNKNOWN;
        }

        return VK_SUCCESS;
    }

    // bool OpenXRWindow::pollEvents(vsg::UIEvents &events)
    // {
    //     if (bufferedEvents.size() > 0)
    //     {
    //         events.splice(events.end(), bufferedEvents);
    //         bufferedEvents.clear();
    //         return true;
    //     }

    //     return false;
    // }
}