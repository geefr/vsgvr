#include <vsgvr/VRDevice.h>

namespace vsgvr
{
    VRDevice::VRDevice()
      : deviceType(DeviceType::Invalid)
      , id(0)
    {}

    VRDevice::VRDevice(DeviceType type, uint32_t deviceId, std::string deviceName, std::string deviceSerial)
      : deviceType(type)
      , id(deviceId)
      , name(deviceName)
      , serial(deviceSerial)
    {}

    VRDevice::~VRDevice() {}
}
