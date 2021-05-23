
#include "device.h"
#include "controller.h"
#include "env.h"

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

namespace vrhelp {

  Device::Device(vr::IVRSystem* context, uint32_t id, vr::ETrackedDeviceClass devClass)
    : mID(id), mDevClass(devClass)
  {
    mName = vrhelp::Env::to_string(devClass);
    mSerial = getDeviceString(context, id, vr::Prop_SerialNumber_String);
  }

  Device::~Device() {}


  std::string Device::getDeviceString(vr::IVRSystem* context, vr::TrackedDeviceIndex_t dev,
    vr::TrackedDeviceProperty prop)
  {
    vr::TrackedPropertyError err;
    auto sLen = context->GetStringTrackedDeviceProperty(dev, prop, nullptr, 0, &err);
    if (sLen == 0) return "";
    std::unique_ptr<char[]> res(new char[sLen]);
    if (context->GetStringTrackedDeviceProperty(dev, prop, res.get(), sLen, &err) != sLen || !res) throw std::runtime_error("Failed to read device string");
    return std::string(res.get());
  }


  std::unique_ptr<Device> Device::enumerate(vr::IVRSystem* context, uint32_t id)
  {
    if (!context->IsTrackedDeviceConnected(id)) return {};

    // Query the common stuff
    auto devClass = context->GetTrackedDeviceClass(id);

    // Setup the child class bits
    switch (devClass)
    {
    case vr::TrackedDeviceClass_Controller:
      return std::move(std::unique_ptr<Device>(dynamic_cast<Device*>(new Controller(context, id, devClass))));
    case vr::TrackedDeviceClass_Invalid:
    case vr::TrackedDeviceClass_HMD:
    case vr::TrackedDeviceClass_GenericTracker:
    case vr::TrackedDeviceClass_TrackingReference:
    case vr::TrackedDeviceClass_DisplayRedirect:
    case vr::TrackedDeviceClass_Max:
    default:
      return std::move(std::unique_ptr<Device>(new Device(context, id, devClass)));
      break;
    }
  }

  glm::dmat4x4 Device::deviceToAbsoluteMatrix() const
  {
    auto p = mPose.mDeviceToAbsoluteTracking.m;
    // HmdMatrix34 is row major
    // glm/RenderFramework is column major
    // https://github.com/ValveSoftware/openvr/issues/193
    return {
      p[0][0], p[1][0], p[2][0], 0.f,
      p[0][1], p[1][1], p[2][1], 0.f,
      p[0][2], p[1][2], p[2][2], 0.f,
      p[0][3], p[1][3], p[2][3], 1.f
    };
  }

  glm::dmat4x4 Device::deviceToAbsoluteMatrixZup() const
  {
    auto p = mPose.mDeviceToAbsoluteTracking.m;
    // HmdMatrix34 is row major
    // glm/RenderFramework is column major
    // https://github.com/ValveSoftware/openvr/issues/193
    glm::dmat4x4 tp = {
      p[0][0], p[1][0], p[2][0], 0.f,
      p[0][1], p[1][1], p[2][1], 0.f,
      p[0][2], p[1][2], p[2][2], 0.f,
      p[0][3], p[1][3], p[2][3], 1.f
    };
    // Also need to map the axis from Z-rear to Z-up
    // -ve Z -> +ve Y
    // +ve Y -> -ve Z
    // +ve X -> +ve X
    glm::dmat4 axesMat = {
      {1, 0, 0, 0},
      {0, 0, 1, 0},
      {0, -1, 0, 0},
      {0, 0, 0, 1}
    };
    return axesMat * tp;
  }

  glm::vec3 Device::positionAbsolute() const
  {
    auto m = deviceToAbsoluteMatrix();
    return { m[3][0], m[3][1], m[3][2] };
  }

  glm::vec3 Device::directionForward() const {
    auto m = deviceToAbsoluteMatrix();
    glm::vec4 v = { 0.f,0.f,-1.f,0.f };
    return m * v;
  }

  glm::vec3 Device::directionBackward() const {
    auto m = deviceToAbsoluteMatrix();
    glm::vec4 v = { 0.f,0.f,1.f,0.f };
    return m * v;
  }

  glm::vec3 Device::directionLeft() const {
    auto m = deviceToAbsoluteMatrix();
    glm::vec4 v = { -1.f,0.f,0.f,0.f };
    return m * v;
  }

  glm::vec3 Device::directionRight() const {
    auto m = deviceToAbsoluteMatrix();
    glm::vec4 v = { 1.f,0.f,0.f,0.f };
    return m * v;
  }

  glm::vec3 Device::directionUp() const {
    auto m = deviceToAbsoluteMatrix();
    glm::vec4 v = { 0.f,1.f,0.f,0.f };
    return m * v;
  }

  glm::vec3 Device::directionDown() const {
    auto m = deviceToAbsoluteMatrix();
    glm::vec4 v = { 0.f,-1.f,0.f,0.f };
    return m * v;
  }

}
