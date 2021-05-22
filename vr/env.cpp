
#include "glm/glm.hpp"

#include "env.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <memory>

namespace vrhelp
{

  Env::Env(vr::ETrackingUniverseOrigin trackingType)
      : mTrackingType(trackingType)
  {
    if (!vr::VR_IsHmdPresent())
      throw std::runtime_error("No HMD present");
    if (!vr::VR_IsRuntimeInstalled())
      throw std::runtime_error("OpenVR runtime not installed");

    auto err = vr::VRInitError_None;
    mContext = vr::VR_Init(&err, vr::EVRApplicationType::VRApplication_Scene);
    if (!mContext || err != vr::VRInitError_None)
    {
      std::stringstream str;
      str << "Failed to create OpenVR context: " << vr::VR_GetVRInitErrorAsEnglishDescription(err);
      throw std::runtime_error(str.str());
    }

    for (auto id = vr::k_unTrackedDeviceIndex_Hmd; id < vr::k_unMaxTrackedDeviceCount; ++id)
    {
      auto dev = Device::enumerate(mContext, id);
      if (!dev)
        continue;
      mDevices[id] = std::move(dev);
    }
  }

  Env::~Env()
  {
    vr::VR_Shutdown();
  }

  void Env::update()
  {
    if (!mContext)
      return;

    // Pump the event queue
    vr::VREvent_t ev;
    while (mContext->PollNextEvent(&ev, sizeof(ev)))
    {
      if (ev.eventType == vr::VREvent_TrackedDeviceActivated ||
          ev.eventType == vr::VREvent_TrackedDeviceUpdated)
      {
        // (re)enumerate the device
        auto dev = Device::enumerate(mContext, ev.trackedDeviceIndex);
        if (!dev)
          continue;
        mDevices[ev.trackedDeviceIndex] = std::move(dev);

        if (mLogging)
          std::cout << "EVENT: Device activated/Updated: TYPE: " << ev.eventType << " DEV: " << ev.trackedDeviceIndex << std::endl;
      }
      else if (ev.eventType == vr::VREvent_TrackedDeviceDeactivated)
      {
        mDevices.erase(ev.trackedDeviceIndex);
        if (mLogging)
          std::cout << "EVENT: Device Deactivated DEV: " << ev.trackedDeviceIndex << std::endl;
      }
      else
      {
        auto dev = mDevices.find(ev.trackedDeviceIndex);
        if (dev == mDevices.end())
          continue;
        auto controller = dynamic_cast<Controller *>(dev->second.get());
        if (!controller)
          continue;

        switch (ev.eventType)
        {
        case vr::VREvent_ButtonPress:
          controller->buttonPress(ev.data.controller.button);
          if (mLogging)
            std::cout << "EVENT: ButtonPress: DEV: " << ev.trackedDeviceIndex << " BUTTON: " << ev.data.controller.button << std::endl;
          break;
        case vr::VREvent_ButtonUnpress:
          controller->buttonUnPress(ev.data.controller.button);
          if (mLogging)
            std::cout << "EVENT: ButtonUnPress: DEV: " << ev.trackedDeviceIndex << " BUTTON: " << ev.data.controller.button << std::endl;
          break;
        case vr::VREvent_ButtonTouch:
          controller->buttonTouch(ev.data.controller.button);
          if (mLogging)
            std::cout << "EVENT: ButtonTouch: DEV: " << ev.trackedDeviceIndex << " BUTTON: " << ev.data.controller.button << std::endl;
          break;
        case vr::VREvent_ButtonUntouch:
          controller->buttonUntouch(ev.data.controller.button);
          if (mLogging)
            std::cout << "EVENT: ButtonUntouch: DEV: " << ev.trackedDeviceIndex << " BUTTON: " << ev.data.controller.button << std::endl;
          break;
        default:
          break;
        }
      }
    }
  }

  vrhelp::Controller *Env::leftHand() const
  {
    // TODO: For now left is the first controller
    // Should make better use of openvr here
    auto dev = std::find_if(mDevices.begin(), mDevices.end(), [&](const auto &d) { return dynamic_cast<vrhelp::Controller *>(d.second.get()); });
    if (dev == mDevices.end())
      return nullptr;
    return dynamic_cast<vrhelp::Controller *>(dev->second.get());
  }

  vrhelp::Controller *Env::rightHand() const
  {
    // TODO: For now right is the second controller
    // Should make better use of openvr here
    auto dev = std::find_if(mDevices.rbegin(), mDevices.rend(), [&](const auto &d) { return dynamic_cast<vrhelp::Controller *>(d.second.get()); });
    if (dev == mDevices.rend())
      return nullptr;
    return dynamic_cast<vrhelp::Controller *>(dev->second.get());
  }

  vrhelp::Device *Env::hmd() const
  {
    auto dev = std::find_if(mDevices.begin(), mDevices.end(), [&](const auto &d) {
      return d.second->mDevClass == vr::TrackedDeviceClass_HMD;
    });
    if (dev == mDevices.end())
      return nullptr;
    return dev->second.get();
  }

  vrhelp::Device *Env::device(uint32_t id)
  {
    auto it = std::find_if(mDevices.begin(), mDevices.end(), [&](const auto &d) {
      return d.second->mID == id;
    });
    if (it == mDevices.end())
      return nullptr;
    return it->second.get();
  }

  glm::dmat4x4 Env::getProjectionMatrix(vr::EVREye eye, float nearZ, float farZ)
  {
    auto projMat = mContext->GetProjectionMatrix(eye, nearZ, farZ);
    auto m = projMat.m;
    return {
        m[0][0],
        m[1][0],
        m[2][0],
        m[3][0],
        m[0][1],
        m[1][1],
        m[2][1],
        m[3][1],
        m[0][2],
        m[1][2],
        m[2][2],
        m[3][2],
        m[0][3],
        m[1][3],
        m[2][3],
        m[3][3],
    };
  }

  /*
  glm::dmat4x4 Env::getProjectionMatrixZup(vr::EVREye eye) {
    // https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetProjectionRaw
    // Tangents of half-angles from center view axis
    float left = 0.0;
    float right = 0.0;
    float top = 0.0;
    float bottom = 0.0;

    double idx = 1.0f / (fRight - fLeft);
    double idy = 1.0f / (fBottom - fTop);
    double idz = 1.0f / (zFar - zNear);
    double sx = fRight + fLeft;
    double sy = fBottom + fTop;

    // The usual projection matrix, but with x/z swapped, and z inverted
    // https://songho.ca/opengl/gl_projectionmatrix.html
    return {
      2.0 * idx, 0.0, sx * idx, 0.0,
      0.0, 0.0, zFar * idz, zFar * zNear * idz,
      0.0, 2 * idy, sy * idy, 0.0,
      0.0, 0.0, -1.0, 0.0
    };
  }
  */

  glm::dmat4x4 Env::getEyeToHeadTransform(vr::EVREye eye)
  {
    auto transform = mContext->GetEyeToHeadTransform(eye);
    auto m = transform.m;
    return {
        m[0][0],
        m[1][0],
        m[2][0],
        0.0f,
        m[0][1],
        m[1][1],
        m[2][1],
        0.0f,
        m[0][2],
        m[1][2],
        m[2][2],
        0.0f,
        m[0][3],
        m[1][3],
        m[2][3],
        1.0f,
    };
  }

#ifdef VR_SUBMIT_OPENGL
  void Env::submitFrames(GLuint leftTex, GLuint rightTex)
  {
    if (!mContext)
      return;

    vr::EColorSpace cSpace = vr::ColorSpace_Gamma;

    vr::Texture_t leftVRTex{reinterpret_cast<void *>(leftTex), vr::ETextureType::TextureType_OpenGL, cSpace};
    vr::Texture_t rightVRTex{reinterpret_cast<void *>(rightTex), vr::ETextureType::TextureType_OpenGL, cSpace};

    vr::VRCompositor()->Submit(vr::Eye_Left, &leftVRTex);
    vr::VRCompositor()->Submit(vr::Eye_Right, &rightVRTex);

    vr::VRCompositor()->PostPresentHandoff();
  }
#elif defined(VR_SUBMIT_VULKAN)

  std::list<std::string> Env::instanceExtensionsRequired() const
  {
    if( !vr::VRCompositor() ) return {};

    std::list<std::string> extensions;
    uint32_t nBufferSize = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);
    if (nBufferSize > 0)
    {
      // Allocate memory for the space separated list and query for it
      char *pExtensionStr = new char[nBufferSize];
      pExtensionStr[0] = 0;
      vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(pExtensionStr, nBufferSize);

      // Break up the space separated list into entries on the CUtlStringList
      std::string curExtStr;
      uint32_t nIndex = 0;
      while (pExtensionStr[nIndex] != 0 && (nIndex < nBufferSize))
      {
        if (pExtensionStr[nIndex] == ' ')
        {
          extensions.push_back(curExtStr);
          curExtStr.clear();
        }
        else
        {
          curExtStr += pExtensionStr[nIndex];
        }
        nIndex++;
      }
      if (curExtStr.size() > 0)
      {
        extensions.push_back(curExtStr);
      }

      delete[] pExtensionStr;
    }

    return extensions;
  }

  std::list<std::string> Env::deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const
  {
    if (!vr::VRCompositor()) return {};

    std::list<std::string> extensions;
    uint32_t nBufferSize = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalDevice, nullptr, 0);
    if (nBufferSize > 0)
    {
      // Allocate memory for the space separated list and query for it
      char *pExtensionStr = new char[nBufferSize];
      pExtensionStr[0] = 0;
      vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalDevice, pExtensionStr, nBufferSize);

      // Break up the space separated list into entries on the CUtlStringList
      std::string curExtStr;
      uint32_t nIndex = 0;
      while (pExtensionStr[nIndex] != 0 && (nIndex < nBufferSize))
      {
        if (pExtensionStr[nIndex] == ' ')
        {
          extensions.push_back(curExtStr);
          curExtStr.clear();
        }
        else
        {
          curExtStr += pExtensionStr[nIndex];
        }
        nIndex++;
      }
      if (curExtStr.size() > 0)
      {
        extensions.push_back(curExtStr);
      }

      delete[] pExtensionStr;
    }

    return extensions;
  }

  void Env::submitFrames(VkImage leftImg, VkImage rightImg, VkDevice device, VkPhysicalDevice physDevice, 
    VkInstance instance, VkQueue queue, uint32_t queueFamIndex, uint32_t width, uint32_t height, VkFormat format, int msaaSamples)
  {
    // Submit to SteamVR
    vr::VRTextureBounds_t bounds;
    bounds.uMin = 0.0f;
    bounds.uMax = 1.0f;
    bounds.vMin = 0.0f;
    bounds.vMax = 1.0f;

    vr::VRVulkanTextureData_t vulkanData;
    vulkanData.m_nImage = (uint64_t)leftImg;
    vulkanData.m_pDevice = device;
    vulkanData.m_pPhysicalDevice = physDevice;
    vulkanData.m_pInstance = instance;
    vulkanData.m_pQueue = queue;
    vulkanData.m_nQueueFamilyIndex = queueFamIndex;

    vulkanData.m_nWidth = width;
    vulkanData.m_nHeight = height;
    vulkanData.m_nFormat = format;
    vulkanData.m_nSampleCount = msaaSamples;

    vr::Texture_t texture = {&vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto};
    vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds);

    // vulkanData.m_nImage = (uint64_t)rightImg;
    // vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds);
  }
#endif

  void Env::waitGetPoses()
  {
    // TODO: Okay I don't know why this is needed but it is
    // If we don't wait here the compositor doesn't accept the frames we're trying to give it
    // Of course that does mean we're probably setting all the transforms/etc at the wrong point...
    vr::VRCompositor()->WaitGetPoses(mTrackedDevicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

    // Update poses of tracked devices
    for (uint32_t id = 0; id < vr::k_unMaxTrackedDeviceCount; ++id)
    {
      auto dev = mDevices.find(id);
      if (dev == mDevices.end())
        continue;
      auto &pose = mTrackedDevicePoses[id];
      if (!pose.bDeviceIsConnected)
        continue;
      dev->second->mPose = pose;
    }
  }

  std::string Env::to_string(vr::ETrackedDeviceClass devClass)
  {
    switch (devClass)
    {
    case vr::TrackedDeviceClass_Invalid:
      return "Invalid";
    case vr::TrackedDeviceClass_HMD:
      return "HMD";
    case vr::TrackedDeviceClass_Controller:
      return "Controller";
    case vr::TrackedDeviceClass_GenericTracker:
      return "Generic Tracker";
    case vr::TrackedDeviceClass_TrackingReference:
      return "Base Station";
    case vr::TrackedDeviceClass_DisplayRedirect:
      return "Display Redirect";
    case vr::TrackedDeviceClass_Max:
      return "Max";
    }
    return "Unknown";
  }

  void Env::getRecommendedTargetSize(uint32_t &width, uint32_t &height)
  {
    if (!mContext)
      return;
    mContext->GetRecommendedRenderTargetSize(&width, &height);
  }
}
