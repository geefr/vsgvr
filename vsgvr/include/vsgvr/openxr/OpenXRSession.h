#pragma once

#include <vsg/core/Inherit.h>
#include <vsgvr/openxr/OpenXR.h>

#include <vsgvr/openxr/OpenXRGraphicsBindingVulkan2.h>
#include <vsgvr/openxr/OpenXRSwapchain.h>

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
            XrSpace _space;

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
