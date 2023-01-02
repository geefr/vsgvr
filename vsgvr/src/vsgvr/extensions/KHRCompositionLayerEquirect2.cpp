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

#include <vsgvr/extensions/KHRCompositionLayerEquirect2.h>

#include <vsgvr/xr/Swapchain.h>
#include <vsgvr/xr/Session.h>
#include <vsgvr/xr/Pose.h>

#include <vsg/core/Exception.h>
#include "../xr/Macros.cpp"

#include <vsg/app/RenderGraph.h>
#include <vsg/ui/FrameStamp.h>

namespace vsgvr {
  KHRCompositionLayerEquirect2::KHRCompositionLayerEquirect2(vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace)
    : Inherit(referenceSpace)
  {}
  KHRCompositionLayerEquirect2::KHRCompositionLayerEquirect2(vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace,
    uint32_t inWidthPixels, uint32_t inHeightPixels)
    : Inherit(referenceSpace)
    , widthPixels(inWidthPixels)
    , heightPixels(inHeightPixels)
  {}
  KHRCompositionLayerEquirect2::~KHRCompositionLayerEquirect2() {}

  std::vector<CompositionLayer::SwapchainImageRequirements> KHRCompositionLayerEquirect2::getSwapchainImageRequirements(vsg::ref_ptr<vsgvr::Instance> instance)
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    reqs.push_back({widthPixels, heightPixels, numSamples});
    return reqs;
  }
  void KHRCompositionLayerEquirect2::render(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp)
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
    _compositionLayer.type = XR_TYPE_COMPOSITION_LAYER_EQUIRECT2_KHR;
    _compositionLayer.next = nullptr;
    _compositionLayer.layerFlags = flags;
    _compositionLayer.space = _referenceSpace->getSpace();
    _compositionLayer.eyeVisibility = eyeVisibility;
    _compositionLayer.subImage.swapchain = swapchain->getSwapchain();
    auto extent = swapchain->getExtent();
    _compositionLayer.subImage.imageRect = XrRect2Di{ {0, 0},
      {static_cast<int>(extent.width), static_cast<int>(extent.height)}
    };
    _compositionLayer.subImage.imageArrayIndex = 0;
    _compositionLayer.pose = pose;
    _compositionLayer.radius = radius;

    _compositionLayer.centralHorizontalAngle = centralHorizontalAngle;
    _compositionLayer.lowerVerticalAngle = lowerVerticalAngle;
    _compositionLayer.upperVerticalAngle = upperVerticalAngle;
  }

  void KHRCompositionLayerEquirect2::setPose(vsg::dvec3 position, vsg::dquat orientation)
  {
    pose = Pose(position, orientation).getPose();
  }
}
