#pragma once

#include <vsgvr/openxr/OpenXR.h>

#include <string>
#include <vector>

namespace vsgvr {
  class OpenXrTraits {
  public:
    OpenXrTraits();
    std::vector<std::string> xrExtensions = {
      "XR_EXT_debug_utils"
    };
    std::vector<std::string> xrLayers = {
      // "XR_APILAYER_LUNARG_core_validation"
    };
    XrVersion apiVersion = XR_MAKE_VERSION(1, 0, 0);
    std::string applicationName = "VSGVR Application";
    std::string engineName = "VSGVR";
    uint32_t engineVersion = 0;
    uint32_t applicationVersion = 0;

    void setApplicationVersion(uint32_t maj, uint32_t min, uint32_t patch);
    void setEngineVersion(uint32_t maj, uint32_t min, uint32_t patch);

    /// Mode and format selections
    /// TODO: For now, applications chose a single option, which is validated. Abstracting
    ///       this to a more general set of view preferences would be better.
    XrFormFactor formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrViewConfigurationType viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    XrEnvironmentBlendMode environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    VkFormat swapchainFormat = VK_FORMAT_R8G8B8A8_SRGB; // Runtimes _should_ support this, as a preferred option
    uint32_t swapchainSampleCount = 4;
  };

  class OpenXrVulkanTraits {
  public:
    std::vector<std::string> vulkanInstanceExtensions = {
      "VK_EXT_debug_report"
    };
    std::vector<std::string> vulkanLayers;
    uint32_t vulkanVersion = VK_API_VERSION_1_0;
    // TODO: These do nothing at the moment
    bool vulkanDebugLayer = false;
    bool vulkanApiDumpLayer = false;
  };
}