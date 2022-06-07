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

#include <vsgvr/OpenXRSwapchain.h>

#include <vsg/core/Exception.h>
#include "OpenXRMacros.cpp"

#include <iostream>

using namespace vsg;

namespace vsgvr {
  SwapchainImage::SwapchainImage(VkImage image, Device* device) :
    Inherit(image, device)
  {
  }

  SwapchainImage::~SwapchainImage()
  {
    for (auto& vd : _vulkanData)
    {
      vd.deviceMemory = nullptr;
      vd.image = VK_NULL_HANDLE;
    }
  }

  OpenXRSwapchain::OpenXRSwapchain(XrSession session, VkFormat swapchainFormat, XrViewConfigurationView viewConfig, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding)
    : _swapchainFormat(swapchainFormat)
  {
    validateFormat(session);
    createSwapchain(session, viewConfig, graphicsBinding);
  }

  OpenXRSwapchain::~OpenXRSwapchain()
  {
    destroySwapchain();
  }

  VkImage OpenXRSwapchain::acquireImage(uint32_t& index)
  {
    xr_check(xrAcquireSwapchainImage(_swapchain, nullptr, &index), "Failed to acquire image");
    return _swapchainImages[index];
  }

  bool OpenXRSwapchain::waitImage(XrDuration timeout)
  {
    auto info = XrSwapchainImageWaitInfo();
    info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    info.next = nullptr;
    info.timeout = timeout;
    auto result = xrWaitSwapchainImage(_swapchain, &info);
    if (result == XR_TIMEOUT_EXPIRED)
    {
      return false;
    }
    xr_check(result, "Failed to wait on image");
    return true;
  }

  void OpenXRSwapchain::releaseImage()
  {
    auto info = XrSwapchainImageReleaseInfo();
    info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
    info.next = nullptr;
    xr_check(xrReleaseSwapchainImage(_swapchain, &info), "Failed to release image");
  }


  void OpenXRSwapchain::validateFormat(XrSession session)
  {
    uint32_t count = 0;
    xr_check(xrEnumerateSwapchainFormats(session, 0, &count, nullptr));
    std::vector<int64_t> formats(count);
    xr_check(xrEnumerateSwapchainFormats(session, static_cast<uint32_t>(formats.size()), &count, formats.data()));

    if (std::find(formats.begin(), formats.end(), static_cast<int64_t>(_swapchainFormat)) == formats.end())
    {
      throw Exception({ "OpenXR runtime doesn't support selected swapchain format (TODO: Preference based selection)" });
    }
  }

  void OpenXRSwapchain::createSwapchain(XrSession session, XrViewConfigurationView viewConfig, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding)
  {
    XrSwapchainUsageFlags usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
    XrSwapchainCreateFlags createFlags = 0;

    _extent.width = viewConfig.recommendedImageRectWidth;
    _extent.height = viewConfig.recommendedImageRectHeight;

    auto info = XrSwapchainCreateInfo();
    info.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    info.next = nullptr;
    info.createFlags = createFlags;
    info.usageFlags = usageFlags;
    info.format = _swapchainFormat;
    info.sampleCount = viewConfig.recommendedSwapchainSampleCount;
    info.width = viewConfig.recommendedImageRectWidth;
    info.height = viewConfig.recommendedImageRectHeight;
    info.faceCount = 1;
    info.arraySize = 1;
    info.mipCount = 1;

    xr_check(xrCreateSwapchain(session, &info, &_swapchain));

    uint32_t imageCount = 0;
    xr_check(xrEnumerateSwapchainImages(_swapchain, 0, &imageCount, nullptr));

    std::vector<XrSwapchainImageVulkan2KHR> images(imageCount, XrSwapchainImageVulkan2KHR());
    for (auto& i : images)
    {
      i.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR;
      i.next = nullptr;
    }
    xr_check(xrEnumerateSwapchainImages(_swapchain, static_cast<uint32_t>(images.size()), &imageCount,
      reinterpret_cast<XrSwapchainImageBaseHeader*>(images.data())));

    for (auto& i : images)
    {
      _swapchainImages.push_back(i.image);
    }

    // Create image views, used by OpenXRSession::creatSwapchain to bind to vsg::Framebuffer
    for (std::size_t i = 0; i < _swapchainImages.size(); ++i)
    {
      auto imageView = ImageView::create(SwapchainImage::create(_swapchainImages[i], graphicsBinding->getVkDevice()));
      imageView->viewType = VK_IMAGE_VIEW_TYPE_2D;
      imageView->format = _swapchainFormat;
      imageView->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageView->subresourceRange.baseMipLevel = 0;
      imageView->subresourceRange.levelCount = 1;
      imageView->subresourceRange.baseArrayLayer = 0;
      imageView->subresourceRange.layerCount = 1;
      imageView->compile(graphicsBinding->getVkDevice());

      _imageViews.push_back(imageView);
    }
  }

  void OpenXRSwapchain::destroySwapchain()
  {
    xr_check(xrDestroySwapchain(_swapchain));
  }
}
