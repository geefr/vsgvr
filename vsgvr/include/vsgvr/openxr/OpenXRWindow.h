#pragma once

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

#include <vsg/ui/UIEvent.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/Semaphore.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/viewer/Window.h>
#include <vsg/viewer/WindowTraits.h>

#include <vsgvr/openxr/OpenXRContext.h>
#include <vsgvr/openxr/OpenXRSwapchain.h>

namespace vsgvr
{
    class VSG_DECLSPEC OpenXRWindow : public vsg::Inherit<vsg::Window, OpenXRWindow>
    {
    public:
        OpenXRWindow(const OpenXRWindow&) = delete;
        OpenXRWindow& operator=(const OpenXRWindow&) = delete;
        
        OpenXRWindow(vsg::ref_ptr<vsg::WindowTraits> traits);

        bool valid() const { return _device; }
        bool visible() const { return valid(); }

        const char* instanceExtensionSurfaceName() const override {return nullptr;}

        bool pollEvents(vsg::UIEvents& events) override {
          // TODO: OpenXR Event support
          return false;
        }

        /// Acquire the next image from the swapchain
        VkResult acquireNextImage(uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;

        void setOpenXRContext(vsg::ref_ptr<vsgvr::OpenXRContext> ctx) { _xr = ctx; }

    protected:
        virtual ~OpenXRWindow();
        void _initSurface() override;
        void _initFormats() override;
        void _initInstance() override;
        void _initPhysicalDevice() override;
        void _initDevice() override;

        void clear() override;
        void share(Window& window) override { }
        void buildSwapchain() override;

        vsg::ref_ptr<vsgvr::OpenXRContext> _xr;
        vsg::ref_ptr<vsgvr::OpenXRSwapchain> _xrSwapchain;
        VkFormat _xrSwapchainFormat = VK_FORMAT_UNDEFINED;

        // TODO: This is a hack
        bool _xrSwapchainImageAcquired = false;
        uint32_t _xrSwapchainImage = 0;
    };
    // VSG_type_name(vsgvr::OpenXRWindow);
}
