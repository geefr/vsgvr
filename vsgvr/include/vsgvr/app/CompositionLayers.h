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
#include <vsg/core/ref_ptr.h>

#include <vsgvr/xr/Common.h>

#include <vsg/app/RecordAndSubmitTask.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/vk/Framebuffer.h>

namespace vsgvr {
  class Swapchain;
  class GraphicsBindingVulkan;
  class Instance;
  class Session;
  class Traits;

  /// Composition layer definitions used by vsgvr::Viewer
  /// 
  /// These classes only expose the user-facing components such as the pose and scale of a quad layer. Assignment
  /// of spaces and swapchain images are handled by the Viewer automatically. All composition layers are defined in the Session's reference space.
  /// 
  /// Note: As layer definitions are significantly different, any additional layer classes require a corresponding update to vsgvr::Viewer itself.
  class VSGVR_DECLSPEC CompositionLayer : public vsg::Inherit<vsg::Object, CompositionLayer>
  {
  public:
    struct SwapchainImageRequirements
    {
      uint32_t width;
      uint32_t height;
      uint32_t sampleCount;
    };

    virtual ~CompositionLayer();
    virtual void populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) = 0;
    virtual std::vector<SwapchainImageRequirements> getSwapchainImageRequirements() = 0;
    virtual void render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp) = 0;
    virtual XrCompositionLayerBaseHeader* getCompositionLayerBaseHeaderPtr() = 0;

    void createSwapchains(vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> graphicsBinding, std::vector< SwapchainImageRequirements> imageRequirements);
    void destroySwapchains();

    vsg::ref_ptr<Swapchain> getSwapchain(size_t view) const { return _viewData[view].swapchain; }

    // TODO: These methods are required at the moment, and have some small differences from their vsg
    //       counterparts. Ideally these would not be duplicated, but this will likely require chnanges
    //       within vsg to correct. In the long run Viewer should only be concerned with the XR parts
    //       or may even be moved to be a 'Window' class.

    /// Note: Cameras are not required when building a CompositionLayerProjection
    vsg::CommandGraphs createCommandGraphsForView(vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsg::Node> vsg_scene, std::vector<vsg::ref_ptr<vsg::Camera>>& cameras, bool assignHeadlight = true);
    void assignRecordAndSubmitTask(std::vector<vsg::ref_ptr<vsg::CommandGraph>> in_commandGraphs);
    void compile(vsg::ref_ptr<vsg::ResourceHints> hints = {});

    void advanceToNextFrame();

  protected:
    vsg::ref_ptr<vsgvr::Instance> _instance;
    vsg::ref_ptr<vsgvr::Traits> _xrTraits;
    vsg::ref_ptr<vsgvr::Swapchain> _swapchain;

    struct Frame
    {
      vsg::ref_ptr<vsg::ImageView> imageView;
      vsg::ref_ptr<vsg::Framebuffer> framebuffer;
    };
    using Frames = std::vector<Frame>;
    Frame& frame(size_t view, size_t i) { return _viewData[view].frames[i]; }
    Frames& frames(size_t view) { return _viewData[view].frames; }

    struct PerViewData
    {
      vsg::ref_ptr<Swapchain> swapchain;
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


    // Manage the work to do each frame using RecordAndSubmitTasks
    // the vsgvr::Viewer renders to a series of OpenXR composition layers,
    // each of which contains one or more RecordAndSubmitTasks (for each view/eye)
    // Composition layers are rendered in order, following the OpenXR specification.
    using RecordAndSubmitTasks = std::vector<vsg::ref_ptr<vsg::RecordAndSubmitTask>>;
    RecordAndSubmitTasks _recordAndSubmitTasks;

    CompositionLayer(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits);
  };

  /// XrCompositionLayerProjection - Planar projected images rendered for each eye, displaying the required view within the scene
  class VSGVR_DECLSPEC CompositionLayerProjection final : public vsg::Inherit<vsgvr::CompositionLayer, CompositionLayerProjection>
  {
    public:
      CompositionLayerProjection() = delete;
      CompositionLayerProjection(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits);
      CompositionLayerProjection(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, XrCompositionLayerFlags inFlags);
      virtual ~CompositionLayerProjection();
      XrCompositionLayerBaseHeader* getCompositionLayerBaseHeaderPtr() override { return reinterpret_cast<XrCompositionLayerBaseHeader*>(&_compositionLayer); }

      XrCompositionLayerFlags flags;

      void populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) override;
      std::vector<SwapchainImageRequirements> getSwapchainImageRequirements() override;
      void render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp) override;

      double nearPlane = 0.05;
      double farPlane = 100.0;

    protected:
      /// @internal
      XrCompositionLayerProjection _compositionLayer;
      std::vector<XrCompositionLayerProjectionView> _layerProjectionViews;
      // Details of chosen _xrTraits->viewConfigurationType
      // Details of individual views - recommended size / sampling
      XrViewConfigurationProperties _viewConfigurationProperties;
      std::vector<XrViewConfigurationView> _viewConfigurationViews;
  };

  /// XrCompositionLayerQuad - Rendered elements on a 2D quad, positioned in world space.
  /// In vsgvr::Viewer the contents of a scenegraph will be rendered to the quad - This may 
  /// be the main 'world' itself, or additional graphs for GUI elements, images, etc.
  class VSGVR_DECLSPEC CompositionLayerQuad final : public vsg::Inherit<vsgvr::CompositionLayer, CompositionLayerQuad>
  {
    public:
      CompositionLayerQuad() = delete;
      CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits);
      CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, XrPosef inPose, XrExtent2Df inSize, XrCompositionLayerFlags inFlags, XrEyeVisibility inEyeVisibility);
      virtual ~CompositionLayerQuad();
      XrCompositionLayerBaseHeader* getCompositionLayerBaseHeaderPtr() override { return reinterpret_cast<XrCompositionLayerBaseHeader*>(&_compositionLayer); }

      void populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) override;
      std::vector<SwapchainImageRequirements> getSwapchainImageRequirements() override;
      void render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp) override;

      XrPosef pose = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f}
      };
      XrExtent2Df size = { 1.0, 1.0 };
      XrCompositionLayerFlags flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
      XrEyeVisibility eyeVisibility = XrEyeVisibility::XR_EYE_VISIBILITY_BOTH;

    protected:
      XrCompositionLayerQuad _compositionLayer;
  };
}

EVSG_type_name(vsgvr::CompositionLayer);
EVSG_type_name(vsgvr::CompositionLayerProjection);
EVSG_type_name(vsgvr::CompositionLayerQuad);
