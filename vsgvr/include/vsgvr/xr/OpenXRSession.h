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
#include <vsgvr/xr/OpenXRCommon.h>

#include <vsgvr/xr/OpenXRInstance.h>
#include <vsgvr/xr/OpenXRGraphicsBindingVulkan.h>
#include <vsgvr/xr/OpenXRSwapchain.h>

#include <vsg/vk/Framebuffer.h>

namespace vsgvr {
    class VSGVR_DECLSPEC OpenXRSession : public vsg::Inherit<vsg::Object, OpenXRSession>
    {
        public:
            OpenXRSession() = delete;
            OpenXRSession(vsg::ref_ptr<OpenXRInstance> instance, vsg::ref_ptr<OpenXRGraphicsBindingVulkan> graphicsBinding,
                          VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs);

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
        protected:
            virtual ~OpenXRSession();
        private:
            void createSession(vsg::ref_ptr<OpenXRInstance> instance);
            void createSwapchains(VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs);
            void destroySwapchains();
            void destroySession();

            vsg::ref_ptr<OpenXRGraphicsBindingVulkan> _graphicsBinding;
            
            XrSession _session = 0;
            XrSessionState _sessionState = XR_SESSION_STATE_UNKNOWN;
            bool _sessionRunning = false;
            XrSpace _space = 0;

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
