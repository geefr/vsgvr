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

#include <vsgvr/app/CompositionLayerProjection.h>

#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/Session.h>
#include <vsgvr/xr/ViewMatrix.h>
#include <vsgvr/xr/ProjectionMatrix.h>

#include <vsg/core/Exception.h>
#include "../xr/Macros.cpp"

#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/ui/FrameStamp.h>

namespace vsgvr {

  CompositionLayerProjection::CompositionLayerProjection(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) 
    : Inherit(instance, xrTraits)
    {}
  CompositionLayerProjection::CompositionLayerProjection(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, XrCompositionLayerFlags inFlags)
    : Inherit(instance, xrTraits)
    , flags(inFlags)
  {}
  CompositionLayerProjection::~CompositionLayerProjection() {}

  void CompositionLayerProjection::populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
  {
    if (!_viewConfigurationViews.empty())
    {
      // This is fine - User may have called manually
      // TODO: Could have more flexible layer width/height selection here, etc.
      return;
    }

    _viewConfigurationProperties = XrViewConfigurationProperties();
    _viewConfigurationProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
    _viewConfigurationProperties.next = nullptr;

    xr_check(xrGetViewConfigurationProperties(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, &_viewConfigurationProperties));
    uint32_t count = 0;
    xr_check(xrEnumerateViewConfigurationViews(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, 0, &count, nullptr));
    _viewConfigurationViews.resize(count, XrViewConfigurationView());
    for (auto& v : _viewConfigurationViews) {
      v.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
      v.next = nullptr;
    }
    xr_check(xrEnumerateViewConfigurationViews(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, static_cast<uint32_t>(_viewConfigurationViews.size()), &count, _viewConfigurationViews.data()));
  }

  std::vector<CompositionLayer::SwapchainImageRequirements> CompositionLayerProjection::getSwapchainImageRequirements()
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    for (auto& config : _viewConfigurationViews)
    {
      reqs.push_back({
        config.recommendedImageRectHeight,
        config.recommendedImageRectHeight,
        config.recommendedSwapchainSampleCount
      });
    }
    return reqs;
  }

  void CompositionLayerProjection::render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp)
  {
    // Locate the views (at predicted frame time), to extract view/proj matrices, and fov
    std::vector<XrView> locatedViews(_viewConfigurationViews.size(), XrView());
    auto viewsValid = false;
    for (auto& v : locatedViews)
    {
      v.type = XR_TYPE_VIEW;
      v.next = nullptr;
    }
    {
      auto viewLocateInfo = XrViewLocateInfo();
      viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
      viewLocateInfo.next = nullptr;
      viewLocateInfo.space = session->getSpace();
      viewLocateInfo.viewConfigurationType = _xrTraits->viewConfigurationType;
      viewLocateInfo.displayTime = frameState.predictedDisplayTime;

      auto viewState = XrViewState();
      viewState.type = XR_TYPE_VIEW_STATE;
      viewState.next = nullptr;
      uint32_t numViews = 0;
      xr_check(xrLocateViews(session->getSession(), &viewLocateInfo, &viewState, static_cast<uint32_t>(locatedViews.size()), &numViews, locatedViews.data()), "Failed to locate views");
      if (numViews != locatedViews.size()) throw vsg::Exception({ "Failed to locate views (Incorrect numViews)" });

      viewsValid = (viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) && (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT);
    }

    // Set up projection views, within a composition layer
    _layerProjectionViews.clear();
    for (auto i = 0u; i < _viewConfigurationViews.size(); ++i)
    {
      auto projectionView = XrCompositionLayerProjectionView();
      projectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
      projectionView.next = nullptr;
      projectionView.fov = locatedViews[i].fov;
      projectionView.pose = locatedViews[i].pose;
      projectionView.subImage.swapchain = getSwapchain(i)->getSwapchain();
      auto extent = getSwapchain(i)->getExtent();
      projectionView.subImage.imageRect = XrRect2Di{ {0, 0},
        {static_cast<int>(extent.width), static_cast<int>(extent.height)}
      };
      projectionView.subImage.imageArrayIndex = 0;
      _layerProjectionViews.push_back(projectionView);
    }

    // Render the scene
    for (auto i = 0u; i < _viewConfigurationViews.size(); ++i)
    {
      auto swapchain = getSwapchain(i);
      uint32_t swapChainImageIndex = 0;

      swapchain->acquireImage(swapChainImageIndex);
      XrDuration timeoutNs = 20000000;
      auto waitSuccess = swapchain->waitImage(timeoutNs);

      if (waitSuccess && viewsValid)
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
                renderGraph->framebuffer = frames(i)[swapChainImageIndex].framebuffer;
                if (auto vsgView = renderGraph->children[0].cast<vsg::View>())
                {
                  vsgView->camera->viewMatrix = ViewMatrix::create(locatedViews[i].pose);
                  vsgView->camera->projectionMatrix = ProjectionMatrix::create(locatedViews[i].fov, nearPlane, farPlane);
                }
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
    }

    _compositionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    _compositionLayer.next = nullptr;
    _compositionLayer.layerFlags = flags;
    _compositionLayer.space = session->getSpace();
    _compositionLayer.viewCount = static_cast<uint32_t>(_layerProjectionViews.size());
    _compositionLayer.views = _layerProjectionViews.data();
  }
}
