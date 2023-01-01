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
  /// XrCompositionLayerEquirectKHR - equirectangular image mapped to the inside of a sphere
  class VSGVR_DECLSPEC KHRCompositionLayerEquirect final : public vsg::Inherit<vsgvr::CompositionLayer, KHRCompositionLayerEquirect>
  {
    public:
      static const char* instanceExtension() { return "XR_KHR_composition_layer_equirect"; }

      KHRCompositionLayerEquirect() = delete;
      KHRCompositionLayerEquirect(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsgvr::Traits> xrTraits, vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace);
      KHRCompositionLayerEquirect(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsgvr::Traits> xrTraits, vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace,
        uint32_t inWidthPixels, uint32_t inHeightPixels);

      virtual ~KHRCompositionLayerEquirect();
      
      XrCompositionLayerBaseHeader* getCompositionLayerBaseHeaderPtr() override { return reinterpret_cast<XrCompositionLayerBaseHeader*>(&_compositionLayer); }
      void populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) override;
      std::vector<SwapchainImageRequirements> getSwapchainImageRequirements() override;
      void render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp) override;

      void setPose(vsg::dvec3 position, vsg::dquat orientation);

      /// Image properties for the rendered quad
      uint32_t widthPixels = 3600;
      uint32_t heightPixels = 1800;
      uint32_t numSamples = 1;

      /// Positioning properties
      XrPosef pose = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f}
      };
      float radius = 100.0f;
      XrVector2f scale = { 1.0f, 1.0f };
      XrVector2f bias = { 0.0f, 0.0f };

      // Composition propertiesw
      XrCompositionLayerFlags flags = 0x00;
      XrEyeVisibility eyeVisibility = XrEyeVisibility::XR_EYE_VISIBILITY_BOTH;
    protected:
      XrCompositionLayerEquirectKHR _compositionLayer;
  };
}

EVSG_type_name(vsgvr::KHRCompositionLayerEquirect);
