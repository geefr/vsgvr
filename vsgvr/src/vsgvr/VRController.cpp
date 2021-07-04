
#include <vsgvr/VRController.h>

namespace vsgvr {
    VRController::VRController(std::string deviceName, std::string deviceSerial)
      : VRDevice(VRDevice::DeviceType::Controller, deviceName, deviceSerial)
    {}

    VRController~VRController() {}
}