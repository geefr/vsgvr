#pragma once

#include <vsgvr/openxr/OpenXR.h>

#include <string>
#include <vector>

namespace vsgvr {
  class OpenXrTraits {
  public:
    OpenXrTraits();
    std::vector<std::string> xrExtensions;
    std::vector<std::string> xrLayers;
    XrVersion apiVersion = XR_MAKE_VERSION(1, 0, 0);
    std::string applicationName = "VSGVR Application";
    std::string engineName = "VSGVR";
    uint32_t engineVersion = 0;
    uint32_t applicationVersion = 0;

    void setApplicationVersion(uint32_t maj, uint32_t min, uint32_t patch);
    void setEngineVersion(uint32_t maj, uint32_t min, uint32_t patch);

    /// Overall mode selection
    /// TODO: For now, application chooses one mode, which is validated. Abstracting
    ///       this to a more general set of view preferences may be useful.
    XrFormFactor formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrViewConfigurationType viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    XrEnvironmentBlendMode environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
  };

  class OpenXrVulkanTraits {
  public:
    std::vector<std::string> vulkanInstanceExtensions;
    std::vector<std::string> vulkanLayers;
    uint32_t vulkanVersion = VK_API_VERSION_1_0;
    bool vulkanDebugLayer = false;
    bool vulkanApiDumpLayer = false;
  };
}