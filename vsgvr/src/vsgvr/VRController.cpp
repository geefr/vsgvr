
#include <vsgvr/VRController.h>

namespace vsgvr
{
  VRController::VRController()
  {
  }

  VRController::VRController(uint32_t deviceId, std::string deviceName, std::string deviceSerial)
  {
      deviceType = VRDevice::DeviceType::Controller;
      id = deviceId;
      name = deviceName;
      serial = deviceSerial;
  }

  VRController::~VRController() {}

  void VRController::buttonPress(uint32_t button)
  {
    buttonPressed[button] = true;
  }

  void VRController::buttonUnPress(uint32_t button)
  {
    buttonPressed[button] = false;
  }

  void VRController::buttonTouch(uint32_t button)
  {
    buttonTouched[button] = true;
  }

  void VRController::buttonUntouch(uint32_t button)
  {
    buttonTouched[button] = false;
  }
}
