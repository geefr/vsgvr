#pragma once

#include <vsg/core/Inherit.h>
#include <vsgvr/openxr/OpenXR.h>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRSwapchain : public vsg::Inherit<vsg::Object, OpenXRSwapchain>
    {
        public:
            OpenXRSwapchain() = delete;
            OpenXRSwapchain(XrSession session, VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs);
            ~OpenXRSwapchain();

            VkImage acquireImage();
            bool waitImage(XrDuration timeout);
            void releaseImage();

            VkFormat format() const { return _swapchainFormat; }
            XrSwapchain getSwapchain() const { return _swapchain; }

        private:
            void validateFormat(XrSession session);
            void createSwapchain(XrSession session, std::vector<XrViewConfigurationView> viewConfigs);
            void destroySwapchain();

            VkFormat _swapchainFormat = VK_FORMAT_UNDEFINED;
            XrSwapchain _swapchain = nullptr;
            std::vector<VkImage> _swapchainImages;
    };
}
