#include <vsgvr/VRContext.h>

namespace vsgvr
{
  VRContext::VRContext()
    : originType(TrackingOrigin::Standing)
    {}

  VRContext::VRContext(TrackingOrigin origin)
    : originType(origin)
    {}

  VRContext::~VRContext() {}

  const VRContext::TrackedDevices& VRContext::devices() const { return vrDevices; }
  vsg::ref_ptr<VRController> VRContext::leftHand() const { return vrLeftHand; }
  vsg::ref_ptr<VRController> VRContext::rightHand() const { return vrRightHand; }
  vsg::ref_ptr<VRDevice> VRContext::hmd() const { return vrHmd; }

  void VRContext::addDevice(uint32_t id, vsg::ref_ptr<vsgvr::VRDevice> device)
  {
    vrDevices[id] = device;
    if( device->deviceType == vsgvr::VRDevice::DeviceType::HMD ) vrHmd = device;
    else if( device->deviceType == vsgvr::VRDevice::DeviceType::Controller ) 
    {
      if( auto controller = device.cast<VRController>() )
      {
        if( controller->role == VRController::Role::LeftHand ) vrLeftHand = controller;
        else if( controller->role == VRController::Role::RightHand ) vrRightHand = controller;
        else
        {
          if( !vrLeftHand ) vrLeftHand = controller;
          else if( !vrRightHand ) vrRightHand = controller;
        }
      }
    }
  }

  void VRContext::removeDevice(uint32_t id)
  {
    auto devIt = vrDevices.find(id);
    if( devIt == vrDevices.end() ) return;
    auto dev = devIt->second;
    vrDevices.erase(devIt);

    if( vrLeftHand == dev ) vrLeftHand = nullptr;
    if( vrRightHand == dev ) vrRightHand = nullptr;
    if( vrHmd == dev ) vrHmd = nullptr;
  }

  vsg::ref_ptr<vsgvr::VRDevice> VRContext::getDevice(uint32_t id) const
  {
    auto devIt = vrDevices.find(id);
    if( devIt == vrDevices.end() ) return {};
    return devIt->second;
  }
}
