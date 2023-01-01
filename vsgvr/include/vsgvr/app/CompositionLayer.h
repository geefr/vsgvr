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

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/RecordAndSubmitTask.h>

namespace vsgvr {
  class Swapchain;
  class GraphicsBindingVulkan;
  class Instance;
  class Session;
  class Traits;
  class ReferenceSpace;

  /// An OpenXR composition layer, and associated layer-specific render capability
  class VSGVR_DECLSPEC CompositionLayer : public vsg::Inherit<vsg::Object, CompositionLayer>
  {
  protected:
    CompositionLayer(
      vsg::ref_ptr<vsgvr::Instance> instance, 
      vsg::ref_ptr<vsgvr::Session> session,
      vsg::ref_ptr<vsgvr::Traits> xrTraits, 
      vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace
    );

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
    vsg::ref_ptr<vsgvr::ReferenceSpace> getReferenceSpace() const { return _referenceSpace; }

    // TODO: Large duplication between these and vsg::Viewer - In future would be nice to unify the implementations in some way

    /// Build command graphs for rendering a vsg::View to the composition layer
    /// Note: Cameras are not required when building a CompositionLayerProjection
    vsg::CommandGraphs createCommandGraphsForView(vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsg::Node> vsg_scene, std::vector<vsg::ref_ptr<vsg::Camera>>& cameras, bool assignHeadlight = true);

    /// Build command graphs for rendering a vsg::RenderGraph to the composition layer
    vsg::CommandGraphs createCommandGraphsForRenderGraph(vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsg::RenderGraph> renderGraph);

    // TODO: If we can provide a RenderGraph, do we need this capability at all?
    // TODO: Should at least document what the above does, and why it's important to have it in here - The reason being how swapchains are acquired etc.
    vsg::CommandGraphs createCommandGraphsForImage(vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsg::Image> image);

    void assignRecordAndSubmitTask(std::vector<vsg::ref_ptr<vsg::CommandGraph>> in_commandGraphs);
    void compile(vsg::ref_ptr<vsg::ResourceHints> hints = {});

    void advanceToNextFrame();

    VkClearColorValue clearColor = { {0.2f, 0.2f, 0.4f, 1.0f} };
    VkClearDepthStencilValue clearDepthStencil = { 0.0f, 0 };

  protected:
    void init(vsg::ref_ptr<vsgvr::Session> session);

    vsg::ref_ptr<vsgvr::Instance> _instance;
    vsg::ref_ptr<vsgvr::Traits> _xrTraits;
    vsg::ref_ptr<vsgvr::Swapchain> _swapchain;
    std::vector<CompositionLayer::SwapchainImageRequirements> _swapchainImageRequirements;

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

    vsg::ref_ptr<vsgvr::ReferenceSpace> _referenceSpace;

    // Manage the work to do each frame using RecordAndSubmitTasks
    // the vsgvr::Viewer renders to a series of OpenXR composition layers,
    // each of which contains one or more RecordAndSubmitTasks (for each view/eye)
    // Composition layers are rendered in order, following the OpenXR specification.
    using RecordAndSubmitTasks = std::vector<vsg::ref_ptr<vsg::RecordAndSubmitTask>>;
    RecordAndSubmitTasks _recordAndSubmitTasks;
  };
}

EVSG_type_name(vsgvr::CompositionLayer);
