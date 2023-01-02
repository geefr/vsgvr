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
#include <vsg/state/ImageView.h>
#include <vsgvr/xr/Common.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>

namespace vsgvr
{
  /// Binding between an OpenXR swapchain image and vsg::Image
  /// 
  /// These images are managed by OpenXR itself - When destroyed this class
  /// does not call vkDestroyImage, and should otherwise be considered read-only.
  class VSGVR_DECLSPEC SwapchainImage : public vsg::Inherit<vsg::Image, SwapchainImage>
  {
  public:
    SwapchainImage(VkImage image, vsg::Device *device);

  protected:
    virtual ~SwapchainImage();
  };
  using SwapchainImages = std::vector<vsg::ref_ptr<vsgvr::SwapchainImage>>;

  /// Wrapper around XrSwapchain, mapping images to vsg::Image
  ///
  /// * Swapchains manage a series of images within OpenXR
  /// * Multiple swapchains may be created by the application - Typically one per XrView or composition layer 
  /// * Swapchains belong to the active session, and are destroyed during session shutdown  
  /// * Likewise, swapchain images belong to a swapchain, and are destroyed as part of this
  /// * Each frame images are acquired from the swapchain, rendered to, and then released to the OpenXR compositor
  class VSGVR_DECLSPEC Swapchain : public vsg::Inherit<vsg::Object, Swapchain>
  {
  public:
    Swapchain() = delete;
    Swapchain(XrSession session, VkFormat swapchainFormat, uint32_t width, uint32_t height, uint32_t sampleCount, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding);

    ~Swapchain();

    VkImage acquireImage(uint32_t &index);
    bool waitImage(XrDuration timeout);
    void releaseImage();

    VkFormat getSwapchainFormat() const { return _swapchainFormat; }
    XrSwapchain getSwapchain() const { return _swapchain; }
    VkExtent2D getExtent() const { return _extent; }

    vsg::ImageViews getImageViews() const { return _imageViews; }

  private:
    void createSwapchain(XrSession session, uint32_t width, uint32_t height, uint32_t sampleCount, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding);

    void destroySwapchain();

    VkFormat _swapchainFormat = VK_FORMAT_UNDEFINED;
    XrSwapchain _swapchain = 0;
    VkExtent2D _extent;

    std::vector<VkImage> _images;
    vsg::ImageViews _imageViews;
  };
}

EVSG_type_name(vsgvr::SwapchainImage);
EVSG_type_name(vsgvr::Swapchain);
