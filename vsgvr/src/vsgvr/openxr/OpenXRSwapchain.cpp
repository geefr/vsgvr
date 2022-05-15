/*
Copyright(c) 2022 Gareth Francis
Based on vsg::Swapchain, Copyright(c) 2018 Robert Osfield

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

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/viewer/Window.h>
#include <vsg/vk/Device.h>

#include <vsgvr/openxr/OpenXRSwapchain.h>

#include <algorithm>
#include <iostream>
#include <limits>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// SwapchainImage
//
namespace vsgvr {
OpenXRSwapchainImage::OpenXRSwapchainImage(VkImage image, Device* device) :
    Inherit(image, device)
{
}

OpenXRSwapchainImage::~OpenXRSwapchainImage()
{
    for (auto& vd : _vulkanData)
    {
        vd.deviceMemory = nullptr;
        vd.image = VK_NULL_HANDLE;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Swapchain
//
OpenXRSwapchain::OpenXRSwapchain(vsg::ref_ptr<OpenXRContext> xr, vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice, vsg::ref_ptr<Device> device, VkFormat format, uint32_t sampleCount)
  : _xr(xr)
  , _device(device)
  , _format(format)
{
    _swapchain = _xr->createSwapchain(format, sampleCount, _extent);

    auto images = _xr->enumerateSwapchainImages(_swapchain);

    // create the ImageViews
    for (std::size_t i = 0; i < images.size(); ++i)
    {
        auto imageView = ImageView::create(OpenXRSwapchainImage::create(images[i], device));
        imageView->viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageView->format = _format;
        imageView->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageView->subresourceRange.baseMipLevel = 0;
        imageView->subresourceRange.levelCount = 1;
        imageView->subresourceRange.baseArrayLayer = 0;
        imageView->subresourceRange.layerCount = 1;
        imageView->compile(device);

        _imageViews.push_back(imageView);
    }
}

OpenXRSwapchain::~OpenXRSwapchain()
{
    _imageViews.clear();

    if (_swapchain)
    {
        _xr->destroySwapchain(_swapchain);
    }
}

bool OpenXRSwapchain::acquireNextImage(uint32_t& imageIndex)
{
    // TODO: From now on not wrapping things in XRContext class - It made sense with OpenVR but
    //       XR will need closer integration to vsg. Needs rework.
    XrSwapchainImageAcquireInfo info;
    info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
    info.next = nullptr;
    auto result = xrAcquireSwapchainImage(_swapchain, &info, &imageIndex);
    // TODO: Not happy with return bool, but avoiding too much Xr type leakage for now
    return XR_SUCCEEDED(result);
}

bool OpenXRSwapchain::waitImage(uint64_t timeout, uint32_t imageIndex)
{
    XrSwapchainImageWaitInfo info;
    info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    info.next = nullptr;
    info.timeout = timeout;
    auto waitResult = xrWaitSwapchainImage(_swapchain, &info);
    if( waitResult != XR_SUCCESS )
    {
        if( waitResult == XR_TIMEOUT_EXPIRED ) return false;
        throw Exception({"OpenXRSwapchain::waitImage"});
    }
    return true;
}

bool OpenXRSwapchain::releaseImage(uint32_t imageIndex) {
    XrSwapchainImageReleaseInfo info;
    info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
    info.next = nullptr;
    auto result = xrReleaseSwapchainImage(_swapchain, &info);
    return XR_SUCCEEDED(result);
}
}