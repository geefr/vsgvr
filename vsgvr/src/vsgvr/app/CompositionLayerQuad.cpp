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

#include <vsgvr/app/CompositionLayerQuad.h>

#include <vsgvr/xr/Swapchain.h>
#include <vsgvr/xr/Session.h>

#include <vsg/core/Exception.h>
#include "../xr/Macros.cpp"

#include <vsg/app/RenderGraph.h>
#include <vsg/ui/FrameStamp.h>

namespace vsgvr {
  CompositionLayerQuad::CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
    : Inherit(instance, xrTraits)
    {}
  CompositionLayerQuad::CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, XrPosef inPose, XrExtent2Df inSize, XrCompositionLayerFlags inFlags, XrEyeVisibility inEyeVisibility)
    : Inherit(instance, xrTraits)
    , pose(inPose)
    , size(inSize)
    , flags(inFlags)
    , eyeVisibility(inEyeVisibility)
  {}
  CompositionLayerQuad::~CompositionLayerQuad() {}

  void CompositionLayerQuad::populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
  {
    // TODO: What to put here? Perhaps it's entirely up to the user in this case, since they're just rendering <a quad> in space
    //       Image size for swapchain of course matters, but that'll just be the image resolution that's passed to OpenXR, before
    //       composition - There's nothing in OpenXR to recommend a size for us here.
  }

  std::vector<CompositionLayer::SwapchainImageRequirements> CompositionLayerQuad::getSwapchainImageRequirements()
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    
    reqs.push_back({1920, 1080, 1});
    return reqs;
  }
  void CompositionLayerQuad::render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp)
  {
    // Render the scene
    auto swapchainI = 0;
    auto swapchain = getSwapchain(swapchainI);
    uint32_t swapChainImageIndex = 0;

    swapchain->acquireImage(swapChainImageIndex);
    XrDuration timeoutNs = 20000000;
    auto waitSuccess = swapchain->waitImage(timeoutNs);

    if (waitSuccess)
    {
      // reset connected ExecuteCommands
      for (auto& recordAndSubmitTask : _recordAndSubmitTasks)
      {
        for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
        {
          commandGraph->reset();
          if (!commandGraph->children.empty())
          {
            // TODO: More reliable way to fetch render graph? Maybe just store a pointer to it if there's no downside?
            if (auto renderGraph = commandGraph->children[0].cast<vsg::RenderGraph>())
            {
              renderGraph->framebuffer = frames(swapchainI)[swapChainImageIndex].framebuffer;
            }
          }
        }
      }

      for (auto& recordAndSubmitTask : _recordAndSubmitTasks)
      {
        recordAndSubmitTask->submit(frameStamp);
      }

      swapchain->releaseImage();
    }

    // Add the composition layer for the frame end info
    _compositionLayer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
    _compositionLayer.next = nullptr;
    _compositionLayer.layerFlags = flags;
    _compositionLayer.eyeVisibility = eyeVisibility;
    _compositionLayer.pose = pose;
    _compositionLayer.size = size;
    _compositionLayer.space = session->getSpace();
    _compositionLayer.subImage.swapchain = swapchain->getSwapchain();
    auto extent = swapchain->getExtent();
    _compositionLayer.subImage.imageRect = XrRect2Di{ {0, 0},
      {static_cast<int>(extent.width), static_cast<int>(extent.height)}
    };
    _compositionLayer.subImage.imageArrayIndex = 0;
  }
}
