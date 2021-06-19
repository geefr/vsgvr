#pragma once

#ifdef VR_SUBMIT_OPENGL
#ifdef _WIN32
#include <GL/glew.h>
#endif
#include <GL/gl.h>
#elif VR_SUBMIT_VULKAN
#include <vulkan/vulkan.h>
#else
#error                                                                         \
    "No VR Frame submission mode selected, define either VR_SUBMIT_OPENGL or VR_SUBMIT_VULKAN"
#endif

#include "controller.h"
#include "device.h"
#include "glm/glm.hpp"

#include "openvr.h"

#include <list>
#include <map>
#include <string>
#include <vector>

namespace vrhelp {
/**
 * VR Env class
 *
 * So this is like a helper class around OpenVR
 */
class Env {
public:
  using DeviceList = std::map<uint32_t, std::unique_ptr<vrhelp::Device>>;

  Env(vr::ETrackingUniverseOrigin trackingType);
  ~Env();

  /**
   * Update the state of the vr environment
   *
   * Will update positions of each device,
   * handle device addition/removal and similar stuff
   */
  void update();

  vr::IVRSystem *context() const;
  const DeviceList &devices() const;
  vrhelp::Device *device(uint32_t id);

  vrhelp::Controller *leftHand() const;
  vrhelp::Controller *rightHand() const;
  vrhelp::Device *hmd() const;

  std::vector<glm::dmat4x4> getProjectionMatrices(float nearZ, float farZ);
  std::vector<glm::dmat4x4> getEyeToHeadTransforms();

  glm::dmat4x4 getProjectionMatrix(vr::EVREye eye, float nearZ, float farZ);
  // glm::dmat4x4 getProjectionMatrixZup(vr::EVREye eye);
  glm::dmat4x4 getEyeToHeadTransform(vr::EVREye eye);

  uint32_t numberOfHmdImages() const { return 2; }

#ifdef VR_SUBMIT_OPENGL
  /**
   * Submit OpenGL textures to OpenVR
   *
   * @param left The ID of the left eye texture
   * @param right The ID of the right eye texture
   */
  void submitFrames(GLuint leftTex, GLuint rightTex);
#elif VR_SUBMIT_VULKAN

  std::list<std::string> instanceExtensionsRequired() const;
  std::list<std::string>
  deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const;

  /**
   * Submit Vulkan images to OpenVR
   */
  void submitFrames(const std::vector<VkImage> images, VkDevice device,
                    VkPhysicalDevice physDevice, VkInstance instance,
                    VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                    uint32_t height, VkFormat format, int msaaSamples);
#endif

  /// TODO
  void waitGetPoses();

  /// Enable/disable verbose logging
  void logging(bool enabled);

  static std::string to_string(vr::ETrackedDeviceClass devClass);

  void getRecommendedTargetSize(uint32_t &width, uint32_t &height);

private:
  vr::IVRSystem *mContext = nullptr;
  vr::TrackedDevicePose_t mTrackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
  DeviceList mDevices;
  vr::ETrackingUniverseOrigin mTrackingType;
  bool mLogging = false;
};

inline vr::IVRSystem *Env::context() const { return mContext; }
inline const Env::DeviceList &Env::devices() const { return mDevices; }
inline void Env::logging(bool enabled) { mLogging = enabled; }
} // namespace vrhelp
