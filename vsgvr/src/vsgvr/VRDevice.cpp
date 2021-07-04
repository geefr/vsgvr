#include <vsgvr/VRDevice.h>

namespace vsgvr
{
    VRDevice::VRDevice(DeviceType type, std::string deviceName, std::string deviceSerial)
      : deviceType(type)
      , name(deviceName)
      , serial(deviceSerial)
    {}

    VRDevice::~VRDevice() {}
}
