#pragma once

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

#include <vsg/state/ImageView.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Surface.h>

#include <vsgvr/openxr/OpenXRContext.h>

namespace vsgvr
{
    class OpenXRSwapchainImage : public vsg::Inherit<vsg::Image, OpenXRSwapchainImage>
    {
    public:
        OpenXRSwapchainImage(VkImage image, vsg::Device* device);

    protected:
        virtual ~OpenXRSwapchainImage();
    };
    // VSG_type_name(vsgvr::OpenXRSwapchainImage);

    class VSG_DECLSPEC OpenXRSwapchain : public vsg::Inherit<vsg::Object, OpenXRSwapchain>
    {
    public:
        OpenXRSwapchain(vsg::ref_ptr<OpenXRContext> xr, vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice, vsg::ref_ptr<vsg::Device> device, VkFormat format, uint32_t sampleCount);

        operator XrSwapchain() const { return _swapchain; }

        VkFormat getImageFormat() const { return _format; }

        const VkExtent2D& getExtent() const { return _extent; }

        vsg::ImageViews& getImageViews() { return _imageViews; }
        const vsg::ImageViews& getImageViews() const { return _imageViews; }

        bool acquireNextImage(uint32_t& imageIndex);
        bool waitImage(uint64_t timeout, uint32_t imageIndex);
        bool releaseImage(uint32_t imageIndex);

    protected:
        virtual ~OpenXRSwapchain();

        vsg::ref_ptr<OpenXRContext> _xr;
        vsg::ref_ptr<vsg::Device> _device;
        XrSwapchain _swapchain;
        VkFormat _format;
        VkExtent2D _extent;
        
        vsg::ImageViews _imageViews;
    };
    // VSG_type_name(vsgvr::OpenXRSwapchain);

}
