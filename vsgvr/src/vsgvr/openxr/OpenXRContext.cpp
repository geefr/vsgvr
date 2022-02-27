
/*
Copyright(c) 2022 Gareth Francis

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

#include <vsgvr/openxr/OpenXRContext.h>
#include <vsg/core/Exception.h>
#include <vsg/vk/PhysicalDevice.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <cstring>
#include <sstream>

namespace {
  const char* to_string(XrResult result) {
    switch(result) {
#define RESULT_CASE(V, _) \
      case V: return #V;
XR_LIST_ENUM_XrResult(RESULT_CASE);
      default: return "Unknown";
    }
  }
  void xr_check(XrResult result, std::string msg) {
    if( XR_SUCCEEDED(result) ) return;
    vsg::Exception e;
    e.message = msg + " (" + to_string(result) + ")";
    throw e;
  }
  PFN_xrVoidFunction xr_pfn(XrInstance instance, std::string name) {
    PFN_xrVoidFunction fn = nullptr;
    xr_check(xrGetInstanceProcAddr(instance, name.c_str(), &fn),
      "Failed to look up function: " + name);
    return fn;
  }


  XrInstance createInstance(std::vector<const char*> extensions, std::vector<const char*> layers, std::string applicationName, uint32_t applicationVersion, std::string engineName, uint32_t engineVersion,XrVersion apiVersion) {
    XrInstanceCreateInfo info;
    info.type = XR_TYPE_INSTANCE_CREATE_INFO;
    info.next = nullptr;
    info.createFlags = 0;
    info.enabledApiLayerCount = layers.size();
    info.enabledApiLayerNames = layers.empty() ? nullptr : layers.data();
    info.enabledExtensionCount = extensions.size();
    info.enabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

    strncpy(info.applicationInfo.applicationName, applicationName.c_str(), std::min(static_cast<int>(applicationName.size()), XR_MAX_APPLICATION_NAME_SIZE));
    info.applicationInfo.applicationVersion = applicationVersion;
    strncpy(info.applicationInfo.engineName, engineName.c_str(), std::min(static_cast<int>(engineName.size()), XR_MAX_ENGINE_NAME_SIZE));
    info.applicationInfo.apiVersion = apiVersion;
    info.applicationInfo.engineVersion = engineVersion;

    XrInstance instance;
    xr_check( xrCreateInstance(&info, &instance), "Failed to create instance" );
    return instance;
  }
  void destroyInstance(XrInstance instance) { xrDestroyInstance(instance); }
  std::pair<XrSystemId, XrSystemProperties> getSystem(XrInstance instance, XrFormFactor formFactor) {
    XrSystemGetInfo info;
    info.type = XR_TYPE_SYSTEM_GET_INFO;
    info.next = nullptr;
    info.formFactor = formFactor;

    XrSystemId system;
    xr_check( xrGetSystem(instance, &info, &system), "Failed to get OpenXR system");

    XrSystemProperties props;
    props.type = XR_TYPE_SYSTEM_PROPERTIES;
    props.next = nullptr;
    props.graphicsProperties = { 0 };
    props.trackingProperties = { 0 };
    xr_check(xrGetSystemProperties(instance, system, &props), "Failed to get OpenXR system properties");

    return std::make_pair(system, props);
  }
  XrGraphicsRequirementsVulkanKHR getVulkanGraphicsRequirements(XrInstance instance, XrSystemId system) {
    XrGraphicsRequirementsVulkanKHR reqs;
    reqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
    reqs.next = nullptr;
    reqs.minApiVersionSupported = 0;
    reqs.maxApiVersionSupported = 0;

    auto fn = (PFN_xrGetVulkanGraphicsRequirementsKHR)xr_pfn(instance, "xrGetVulkanGraphicsRequirementsKHR");
    xr_check(fn(instance, system, &reqs), "Failed to get Vulkan requirements");
    return reqs;
  }
  std::list<std::string> getVulkanInstanceExtensionsRequired(XrInstance instance, XrSystemId system) {
    auto fn = (PFN_xrGetVulkanInstanceExtensionsKHR)xr_pfn(instance, "xrGetVulkanInstanceExtensionsKHR");
    uint32_t size = 0;
    xr_check(fn(instance, system, 0, &size, nullptr), "Failed to get instance extensions (num)");

    std::string names;
    names.reserve(size);
    xr_check(fn(instance, system, size, &size, names.data()), "Failed to get instance extensions");
    
    // Single-space delimited
    std::list<std::string> extensions;
    std::stringstream s(names);
    std::string name;
    while(std::getline(s, name)) extensions.push_back(name);

    return extensions;
  }
  void checkVulkanPhysicalDeviceIsValid(XrInstance instance, XrSystemId system, vsg::ref_ptr<vsg::Instance> vulkanInstance, vsg::ref_ptr<vsg::PhysicalDevice> vulkanPhysicalDevice) {
    auto fn = (PFN_xrGetVulkanGraphicsDeviceKHR)xr_pfn(instance, "xrGetVulkanGraphicsDeviceKHR");
    VkPhysicalDevice expectedDevice;
    xr_check(fn(instance, system, *vulkanInstance, &expectedDevice), "Failed to get vulkan graphics device");

    if( *vulkanPhysicalDevice != expectedDevice )
    {
      vsg::Exception e;
      e.message = "VSGVR TODO: The physical device selected by window initialisation is not the same as selected by OpenXR. Restart and select the correct gpu via WindowTraits::deviceTypePreference";
      throw e;
    }
  }
}

namespace vsgvr
{
  class OpenXRContextImpl
  {
  public:
    XrInstance instance;
    XrSystemId systemID;
    XrSystemProperties systemProperties;
    bool intialised = false;

    void init() {
      if( intialised ) return;

      // TODO: Expose application settings to app, correct engine version from some version header / vsg base
      instance = createInstance(
        {"XR_KHR_vulkan_enable"},
        {},
        "VSGVR Application",
        0,
        "VSGVR",
        0,
        XR_VERSION_1_0
      );

      {
        // TODO: Expose display type to application
        auto sys = getSystem(instance, XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY);
        systemID = sys.first;
        systemProperties = sys.second;
      }

      intialised = true;
    }

    void deinit() {
      if( !intialised ) return;

      destroyInstance(instance);

      intialised = false;
    }

    std::list<std::string> instanceExtensionsRequired(uint32_t vkVersion) {
      // Viewer will always need to check for extensions, so also perform the api version check here
      auto graphicsRequirements = getVulkanGraphicsRequirements(instance, systemID);
      if( vkVersion < graphicsRequirements.minApiVersionSupported || vkVersion > graphicsRequirements.maxApiVersionSupported )
      {
        vsg::Exception e;
        e.message = "Vulkan version not supported by OpenXR runtime";
      }
  
      return getVulkanInstanceExtensionsRequired(instance, systemID);
    }

    std::list<std::string> deviceExtensionsRequired(vsg::ref_ptr<vsg::Instance> vulkanInstance, vsg::ref_ptr<vsg::PhysicalDevice> vulkanPhysicalDevice) {
      // TODO: This is actually not checking for extensions, rather checking that the physical device
      //       OpenXR requires is the same as the one the window has selected.
      // Device selection needs refactoring for the OpenXR approach, context needs to be able to instruct
      // the window which device to use (likely core vsg change needed)
      checkVulkanPhysicalDeviceIsValid(instance, systemID, vulkanInstance, vulkanPhysicalDevice);
      return {};
    }
  };

  OpenXRContext::OpenXRContext(vsgvr::VRContext::TrackingOrigin origin)
    : m(new OpenXRContextImpl())
  {
    m->init();
  }

  OpenXRContext::~OpenXRContext()
  {
    m->deinit();
  }

  void OpenXRContext::update()
  {

  }

  void OpenXRContext::waitGetPoses()
  {
    // TODO: Doesn't apply to XR?
  }

  std::vector<vsg::dmat4> OpenXRContext::getProjectionMatrices(float nearZ, float farZ)
  {
    // TODO
  }

  std::vector<vsg::dmat4> OpenXRContext::getEyeToHeadTransforms()
  {
    // TODO
  }

  uint32_t OpenXRContext::numberOfHmdImages() const
  {
    // TODO: Doesn't apply to XR?
  }

  bool OpenXRContext::getRecommendedTargetSize(uint32_t &width, uint32_t &height)
  {
    // TODO: Doesn't apply to XR?
  }

  void OpenXRContext::submitFrames(const std::vector<VkImage> images, VkDevice device,
                      VkPhysicalDevice physDevice, VkInstance instance,
                      VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                      uint32_t height, VkFormat format, int msaaSamples)
  {
    // TODO: Doesn't apply to XR?
  }

  std::list<std::string> OpenXRContext::instanceExtensionsRequired(uint32_t vkVersion) const
  {
    return m->instanceExtensionsRequired(vkVersion);
  }

  std::list<std::string> OpenXRContext::deviceExtensionsRequired(vsg::ref_ptr<vsg::Instance> instance, vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice) const
  {
    return m->deviceExtensionsRequired(instance, physicalDevice);
  }
}
