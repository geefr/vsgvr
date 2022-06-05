#pragma once

#include <vsg/core/Inherit.h>
#include <vsg/state/ImageView.h>
#include <vsgvr/openxr/OpenXR.h>
#include <vsgvr/openxr/OpenXRGraphicsBindingVulkan2.h>

namespace vsgvr {


  class SwapchainImage : public vsg::Inherit<vsg::Image, SwapchainImage>
  {
  public:
    SwapchainImage(VkImage image, vsg::Device* device);

  protected:
    virtual ~SwapchainImage();
  };

    class VSG_DECLSPEC OpenXRSwapchain : public vsg::Inherit<vsg::Object, OpenXRSwapchain>
    {
        public:
            OpenXRSwapchain() = delete;
            OpenXRSwapchain(XrSession session, VkFormat swapchainFormat, XrViewConfigurationView viewConfig, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding);
            ~OpenXRSwapchain();

            VkImage acquireImage(uint32_t& index);
            bool waitImage(XrDuration timeout);
            void releaseImage();

            VkFormat format() const { return _swapchainFormat; }
            XrSwapchain getSwapchain() const { return _swapchain; }
            VkExtent2D getExtent() const { return _extent; }

            vsg::ImageViews getImageViews() const { return _imageViews; }
        private:
            void validateFormat(XrSession session);
            void createSwapchain(XrSession session, XrViewConfigurationView viewConfig, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding);
            void destroySwapchain();

            VkFormat _swapchainFormat = VK_FORMAT_UNDEFINED;
            XrSwapchain _swapchain = nullptr;
            VkExtent2D _extent;

            std::vector<VkImage> _swapchainImages;
            vsg::ImageViews _imageViews;
    };
}
