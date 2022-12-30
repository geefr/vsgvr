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

#include <vsgvr/app/CompositionLayer.h>

namespace vsgvr {
  /// XrCompositionLayerProjection - Planar projected images rendered for each eye, displaying the required view within the scene
  ///
  /// When this layer is used cameras do not need to be provided when calling createCommandGraphsForView.
  /// If a camera is provided it will be used, but any cameras used by this layer will be modified each frame, in order to
  /// render the scene to the headset.
  /// 
  /// Cameras used with this layer should not be directly shared for any other use - Doing so can cause deadlocks when running
  /// under SteamVR (Instead a separate camera should be constructed and synced to the user's head position).
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
      XrCompositionLayerProjection _compositionLayer;
      std::vector<XrCompositionLayerProjectionView> _layerProjectionViews;
      // Details of chosen _xrTraits->viewConfigurationType
      // Details of individual views - recommended size / sampling
      XrViewConfigurationProperties _viewConfigurationProperties;
      std::vector<XrViewConfigurationView> _viewConfigurationViews;
  };
}

EVSG_type_name(vsgvr::CompositionLayerProjection);
