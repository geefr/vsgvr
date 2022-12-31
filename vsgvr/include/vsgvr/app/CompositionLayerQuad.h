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
  /// XrCompositionLayerQuad - Rendered elements on a 2D quad, positioned in world space.
  /// 
  /// This composition layer is typically used for rendering UI panels or other content,
  /// as a plane within the world. Note that this plane will obscure the world, ignoring
  /// depth.
  class VSGVR_DECLSPEC CompositionLayerQuad final : public vsg::Inherit<vsgvr::CompositionLayer, CompositionLayerQuad>
  {
    public:
      CompositionLayerQuad() = delete;
      CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace);
      CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace, uint32_t inWidthPixels, uint32_t inHeightPixels);
      CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace, uint32_t inWidthPixels, uint32_t inHeightPixels, uint32_t inNumSamples, vsg::dvec3 position, vsg::dquat orientation, XrExtent2Df inSizeMeters, XrCompositionLayerFlags inFlags, XrEyeVisibility inEyeVisibility);
      CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, vsg::ref_ptr<vsgvr::ReferenceSpace> referenceSpace, uint32_t inWidthPixels, uint32_t inHeightPixels, uint32_t inNumSamples, XrPosef inPose, XrExtent2Df inSizeMeters, XrCompositionLayerFlags inFlags, XrEyeVisibility inEyeVisibility);
      virtual ~CompositionLayerQuad();
      XrCompositionLayerBaseHeader* getCompositionLayerBaseHeaderPtr() override { return reinterpret_cast<XrCompositionLayerBaseHeader*>(&_compositionLayer); }

      void populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) override;
      std::vector<SwapchainImageRequirements> getSwapchainImageRequirements() override;
      void render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp) override;

      void setPose(vsg::dvec3 position, vsg::dquat orientation);

      /// Image properties for the rendered quad
      uint32_t widthPixels = 1920;
      uint32_t heightPixels = 1080;
      uint32_t numSamples = 1;

      /// Positioning properties
      /// Note: XrPosef is in the OpenXR coordinate system - X-right, Y-up, Z-back
      XrPosef pose = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f}
      };
      XrExtent2Df sizeMeters = { 1.0, 1.0 };
      XrCompositionLayerFlags flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
      XrEyeVisibility eyeVisibility = XrEyeVisibility::XR_EYE_VISIBILITY_BOTH;
    protected:
      XrCompositionLayerQuad _compositionLayer;
  };
}

EVSG_type_name(vsgvr::CompositionLayerQuad);
