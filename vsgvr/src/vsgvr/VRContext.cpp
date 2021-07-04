#include <vsgvr/VRContext.h>

namespace vsgvr
{
  VRContext::VRContext(TrackingOrigin origin)
    : originType(origin)
    {}

  VRContext::~VRContext() {}

  const VRContext::TrackedDevices& VRContext::devices() const { return vrDevices; }
  vsg::ref_ptr<VRController> VRContext::leftHand() const { return vrLeftHand; }
  vsg::ref_ptr<VRController> VRContext::rightHand() const { return vrRightHand; }
  vsg::ref_ptr<VRDevice> VRContext::hmd() const { return vrHmd; }

}
