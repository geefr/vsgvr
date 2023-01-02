
namespace {
  XrViewConfigurationType selectViewConfigurationType(vsg::ref_ptr<vsgvr::Instance> instance)
  {
    // Most likely will be stereo for a head-mounted display.
    // Mono will be used if supported otherwise.
    // QUAD_VARJO and other extension-based configs could be selected here, but the application would
    // also need to enable these extensions during Instance::create.
    std::vector<XrViewConfigurationType> viewPrefs = {
      XrViewConfigurationType::XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
      XrViewConfigurationType::XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO,
    };
    for (auto& view : viewPrefs)
    {
      if (instance->checkViewConfigurationSupported(view))
      {
        return view;
      }
    }
    // Fall back to first-supported
    return instance->getSupportedViewConfigurationTypes().front();
  }

  XrEnvironmentBlendMode selectEnvironmentBlendMode(vsg::ref_ptr<vsgvr::Instance> instance, XrViewConfigurationType viewConfigurationType)
  {
    // VR headsets should support OPAQUE
    // AR headsets should support ADDITIVE or ALPHA_BLEND
    std::vector<XrEnvironmentBlendMode> blendModes = {
      XrEnvironmentBlendMode::XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
      XrEnvironmentBlendMode::XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
      XrEnvironmentBlendMode::XR_ENVIRONMENT_BLEND_MODE_ADDITIVE,
    };
    for (auto& mode : blendModes)
    {
      if (instance->checkEnvironmentBlendModeSupported(viewConfigurationType, mode))
      {
        return mode;
      }
    }
    // Fall back to first-supported
    return instance->getSupportedEnvironmentBlendModes(viewConfigurationType).front();
  }

  void configureXrVulkanRequirements(vsg::ref_ptr<vsg::WindowTraits> windowTraits, vsgvr::VulkanRequirements& xrVulkanReqs)
  {
    if (windowTraits->vulkanVersion < xrVulkanReqs.minVersion)
    {
      throw std::runtime_error("Vulkan API too low for OpenXR");
    }
    if (windowTraits->vulkanVersion > xrVulkanReqs.maxVersion)
    {
      std::cout << "Warning: Vulkan API higher than OpenXR maximum. Maximum tested version is " << xrVulkanReqs.maxVersionStr << std::endl;
    }

    // Encountered with SteamVR: debug_marker is requested in device extensions but is not present, causing device creation to fail
    // This doesn't actually appear to be required by SteamVR to work, so drop it if present.
    xrVulkanReqs.deviceExtensions.erase("VK_EXT_debug_marker");
    xrVulkanReqs.instanceExtensions.erase("VK_EXT_debug_report");
    // Add any other requirements of OpenXR to the window traits, this mostly includes memory sharing and synchronisation extensions
    // Note: windowsTraits stores vector<const char*> - These are references to xrVulkanReqs
    for (auto& ext : xrVulkanReqs.instanceExtensions)
    {
      if (std::find(windowTraits->instanceExtensionNames.begin(), windowTraits->instanceExtensionNames.end(), ext) == windowTraits->instanceExtensionNames.end()) {
        windowTraits->instanceExtensionNames.push_back(ext.c_str());
      }
    }
    for (auto& ext : xrVulkanReqs.deviceExtensions)
    {
      if (std::find(windowTraits->deviceExtensionNames.begin(), windowTraits->deviceExtensionNames.end(), ext) == windowTraits->deviceExtensionNames.end()) {
        windowTraits->deviceExtensionNames.push_back(ext.c_str());
      }
    }
  }

  VkFormat selectSwapchainFormat(vsg::ref_ptr<vsgvr::Session> vrSession)
  {
    // The specific swapchain format shouldn't impact the application much, but a compatible
    // image format must be selected for OpenXR to operate
    // These two formats are listed in the OpenXR specification, and should be supported by
    // most runtimes.
    std::vector<VkFormat> formats = {
      VkFormat::VK_FORMAT_R8G8B8A8_UNORM,
      VkFormat::VK_FORMAT_R8G8B8A8_SRGB
    };
    for (auto& format : formats) {
      if (vrSession->checkSwapchainFormatSupported(format))
      {
        return format;
      }
    }
    // Fall back to first-supported
    return vrSession->getSupportedSwapchainFormats().front();
  }

  uint32_t selectSwapchainSampleCount(vsg::ref_ptr<vsgvr::Session> vrSession, VkSampleCountFlags samples)
  {
    // Similar to swapchain image format, the number of samples shouldn't matter much, but if
    // possible should match the multisample settings of the vsg rendering.
    // After rendering, swapchain images are handed to the OpenXR compositor, which should
    // have access to the highest-quality image available.
    if (vrSession->checkSwapchainSampleCountSupported(samples))
    {
      return samples;
    }
    return 1;
  }

  XrReferenceSpaceType selectReferenceSpaceType(vsg::ref_ptr<vsgvr::Session> vrSession)
  {
    // The spaces available will depend upon the OpenXR runtime
    // Typically STAGE is supported for room-scale systems,
    // with LOCAL supported for standing-only scale systems.
    // Most runtimes will support multiple however, or support
    // additional extension-based spaces.
    //
    // For common VR headsets, the difference between STAGE and LOCAL may be quite subtle,
    // and have little impact on this application.
    std::vector<XrReferenceSpaceType> spaces = {
      XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_STAGE,
      XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_LOCAL,
    };
    for (auto& space : spaces)
    {
      if (vrSession->checkReferenceSpaceTypeSupported(space))
      {
        return space;
      }
    }
    return vrSession->getSupportedReferenceSpaceTypes().front();
  }
}
