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

#pragma once

#include <vsg/core/Inherit.h>
#include <vsgvr/OpenXRCommon.h>

#include <vsgvr/OpenXRGraphicsBindingVulkan2.h>
#include <vsgvr/OpenXRSwapchain.h>

#include <vsg/vk/Framebuffer.h>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRSession : public vsg::Inherit<vsg::Object, OpenXRSession>
    {
        public:
            OpenXRSession() = delete;
            OpenXRSession(XrInstance instance, XrSystemId system, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding,
                          VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs);
            ~OpenXRSession();

            XrSession getSession() const { return _session; }
            XrSessionState getSessionState() const { return _sessionState; }
            bool getSessionRunning() const { return _sessionRunning; }

            void onEventStateChanged(const XrEventDataSessionStateChanged& event);

            void beginSession(XrViewConfigurationType viewConfigurationType);
            void endSession();

            vsg::ref_ptr<OpenXRSwapchain> getSwapchain(size_t view) const { return _viewData[view].swapchain; }

            XrSpace getSpace() const { return _space; }

            struct Frame
            {
              vsg::ref_ptr<vsg::ImageView> imageView;
              vsg::ref_ptr<vsg::Framebuffer> framebuffer;
            };
            using Frames = std::vector<Frame>;

            Frame& frame(size_t view, size_t i) { return _viewData[view].frames[i]; }
            Frames& frames(size_t view) { return _viewData[view].frames; }
        private:
            void createSession(XrInstance instance, XrSystemId system);
            void createSwapchains(VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs);
            void destroySwapchains();
            void destroySession();

            vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> _graphicsBinding;
            XrSession _session = nullptr;
            XrSessionState _sessionState = XR_SESSION_STATE_UNKNOWN;
            bool _sessionRunning = false;
            XrSpace _space = nullptr;

            struct PerViewData
            {
              vsg::ref_ptr<OpenXRSwapchain> swapchain;
              vsg::ref_ptr<vsg::Image> depthImage;
              vsg::ref_ptr<vsg::ImageView> depthImageView;
              // only used when multisampling is required
              vsg::ref_ptr<vsg::Image> multisampleImage;
              vsg::ref_ptr<vsg::ImageView> multisampleImageView;
              // only used when multisampling and with Traits::requiresDepthRead == true
              vsg::ref_ptr<vsg::Image> multisampleDepthImage;
              vsg::ref_ptr<vsg::ImageView> multisampleDepthImageView;
              Frames frames;
              vsg::ref_ptr<vsg::RenderPass> renderPass;
            };
            std::vector<PerViewData> _viewData;
    };
}
