#pragma once

/*
Copyright(c) 2021 Gareth Francis

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

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>
#include <vsg/maths/mat4.h>

namespace vsgvr
{
  class VSGVR_DECLSPEC VRDevice : public vsg::Inherit<vsg::Object, VRDevice>
  {
  public:
    enum class DeviceType
    {
      Invalid,
      HMD,
      Controller,
      Generic,
      TrackingReference,
      DisplayRedirect,
    };

    VRDevice();
    VRDevice(DeviceType type, uint32_t deviceId, std::string deviceName, std::string deviceSerial);
    virtual ~VRDevice();

    DeviceType deviceType = DeviceType::Invalid;

    uint32_t id;
    std::string name;
    std::string serial;
    
    vsg::dmat4 deviceToAbsoluteMatrix;
  };
}
