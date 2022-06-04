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

            // TODO: A regular session update is needed - should handle the session lifecycle and such
            // void update(XrInstance instance, XrSystemID system);

            XrSession getSession() const { return _session; }
            XrSessionState getSessionState() const { return _sessionState; }
            bool getSessionRunning() const { return _sessionRunning; }

            void onEventStateChanged(const XrEventDataSessionStateChanged& event);

            void beginSession(XrViewConfigurationType viewConfigurationType);
            void endSession();

            vsg::ref_ptr<OpenXRSwapchain> getSwapchain() const { return _swapchain; }

            XrSpace getSpace() const { return _space; }

            // TODO: Similarity to vsg::Swapchain and vsg::Window here
            // for attachments / framebuffers
            struct Frame
            {
              vsg::ref_ptr<vsg::ImageView> imageView;
              vsg::ref_ptr<vsg::Framebuffer> framebuffer;
            };

            using Frames = std::vector<Frame>;

            Frame& frame(size_t i) { return _frames[i]; }
            Frames& frames() { return _frames; }
        private:
            void createSession(XrInstance instance, XrSystemId system);
            void createSwapchain(VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs);
            void destroySwapchain();
            void destroySession();

            vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> _graphicsBinding;
            XrSession _session = nullptr;
            XrSessionState _sessionState = XR_SESSION_STATE_UNKNOWN;
            bool _sessionRunning = false;

            vsg::ref_ptr<OpenXRSwapchain> _swapchain;

            XrSpace _space;

            vsg::ref_ptr<vsg::Image> _depthImage;
            vsg::ref_ptr<vsg::ImageView> _depthImageView;
            // only used when multisampling is required
            vsg::ref_ptr<vsg::Image> _multisampleImage;
            vsg::ref_ptr<vsg::ImageView> _multisampleImageView;
            // only used when multisampling and with Traits::requiresDepthRead == true
            vsg::ref_ptr<vsg::Image> _multisampleDepthImage;
            vsg::ref_ptr<vsg::ImageView> _multisampleDepthImageView;
            Frames _frames;
            vsg::ref_ptr<vsg::RenderPass> _renderPass;
    };
}
