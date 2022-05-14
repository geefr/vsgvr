
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
#include <vsgvr/openxr/OpenXRVkInstance.h>
#include <vsgvr/openxr/OpenXRVkPhysicalDevice.h>
#include <vsgvr/openxr/OpenXRVkDevice.h>

#include <vsg/core/Exception.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/PhysicalDevice.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <cstring>
#include <sstream>

namespace {
  const char* to_string(XrResult result) {
    switch (result) {
#define RESULT_CASE(V, _) \
      case V: return #V;
      XR_LIST_ENUM_XrResult(RESULT_CASE);
    default: return "Unknown";
    }
  }
  void xr_check(XrResult result, std::string msg) {
    if (XR_SUCCEEDED(result)) return;
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


  XrInstance createInstance(std::vector<const char*> extensions, std::vector<const char*> layers, std::string applicationName, uint32_t applicationVersion, std::string engineName, uint32_t engineVersion, XrVersion apiVersion) {
    XrInstanceCreateInfo info;
    info.type = XR_TYPE_INSTANCE_CREATE_INFO;
    info.next = nullptr;
    info.createFlags = 0;
    info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
    info.enabledApiLayerNames = layers.empty() ? nullptr : layers.data();
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.enabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

    strncpy(info.applicationInfo.applicationName, applicationName.c_str(), std::min(static_cast<int>(applicationName.size() + 1), XR_MAX_APPLICATION_NAME_SIZE));
    info.applicationInfo.applicationVersion = applicationVersion;
    strncpy(info.applicationInfo.engineName, engineName.c_str(), std::min(static_cast<int>(engineName.size() + 1), XR_MAX_ENGINE_NAME_SIZE));
    info.applicationInfo.apiVersion = apiVersion;
    info.applicationInfo.engineVersion = engineVersion;

    XrInstance instance;
    xr_check(xrCreateInstance(&info, &instance), "Failed to create instance");
    return instance;
  }
  void destroyInstance(XrInstance instance) { xrDestroyInstance(instance); }
  std::pair<XrSystemId, XrSystemProperties> getSystem(XrInstance instance, XrFormFactor formFactor) {
    XrSystemGetInfo info;
    info.type = XR_TYPE_SYSTEM_GET_INFO;
    info.next = nullptr;
    info.formFactor = formFactor;

    XrSystemId system;
    xr_check(xrGetSystem(instance, &info, &system), "Failed to get OpenXR system");

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
    while (std::getline(s, name)) extensions.push_back(name);

    return extensions;
  }
  std::list<std::string> getVulkanDeviceExtensionsRequired(XrInstance instance, XrSystemId system, vsg::ref_ptr<vsg::Instance> vulkanInstance, vsg::ref_ptr<vsg::PhysicalDevice> vulkanPhysicalDevice) {

    auto GetDevice = (PFN_xrGetVulkanGraphicsDeviceKHR)xr_pfn(instance, "xrGetVulkanGraphicsDeviceKHR");
    VkPhysicalDevice expectedDevice;
    xr_check(GetDevice(instance, system, *vulkanInstance, &expectedDevice), "Failed to get vulkan graphics device");

    if (*vulkanPhysicalDevice != expectedDevice)
    {
      vsg::Exception e;
      e.message = "VSGVR TODO: The physical device selected by window initialisation is not the same as selected by OpenXR. Restart and select the correct gpu via WindowTraits::deviceTypePreference";
      throw e;
    }

    auto GetDeviceExtensions = (PFN_xrGetVulkanDeviceExtensionsKHR)xr_pfn(instance, "xrGetVulkanDeviceExtensionsKHR");
    uint32_t size = 0;
    xr_check(GetDeviceExtensions(instance, system, 0, &size, nullptr), "Failed to get device extensions (num)");

    std::string names;
    names.reserve(size);
    xr_check(GetDeviceExtensions(instance, system, size, &size, names.data()), "Failed to get device extensions");

    // Single-space delimited
    std::list<std::string> extensions;
    std::stringstream s(names);
    std::string name;
    while (std::getline(s, name)) extensions.push_back(name);

    return extensions;
  }
  XrSession createSession(XrInstance instance, XrSystemId system, VkInstance vulkanInstance, VkPhysicalDevice vulkanPhysicalDevice, VkDevice vulkanDevice, uint32_t qFamilyIndex, uint32_t qIndex) {
    XrGraphicsBindingVulkanKHR binding;
    binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    binding.next = nullptr;
    binding.instance = vulkanInstance;
    binding.physicalDevice = vulkanPhysicalDevice;
    binding.device = vulkanDevice;
    binding.queueFamilyIndex = qFamilyIndex;
    binding.queueIndex = qIndex;

    // TODO: Support for overlays
    XrSessionCreateInfo info;
    info.type = XR_TYPE_SESSION_CREATE_INFO;
    info.next = &binding;
    info.systemId = system;

    /*
        // HAX HAX HAX
        VkCommandPoolCreateInfo cmd_pool_info = {
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = qFamilyIndex
        };

        VkCommandPool cmd_pool;
        if( VK_SUCCESS != vkCreateCommandPool(vulkanDevice, &cmd_pool_info, nullptr, &cmd_pool))
        {
          throw vsg::Exception("HAX FAILED :(");
        }
        // HAX HAX HAX
    */

    XrSession session;
    xr_check(xrCreateSession(instance, &info, &session), "Failed to create OpenXR session");
    return session;
  }

  vsg::ref_ptr<vsg::Instance> getVulkanInstance(XrInstance instance, XrSystemId system, uint32_t vulkanApiVersion, std::vector<const char*> vulkanRequiredInstanceExtensions,
    std::string applicationName, uint32_t applicationVersion, std::string engineName, uint32_t engineVersion)
  {
    // Check we have vulkan available in openxr
    {
      PFN_xrGetVulkanGraphicsRequirements2KHR GetVulkanGraphicsRequirements2 = NULL;
      XrResult result =
        xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirements2KHR",
          (PFN_xrVoidFunction*)&GetVulkanGraphicsRequirements2);
      if (!XR_SUCCEEDED(result))
      {
        throw vsg::Exception({"Failed to look up xrGetVulkanGraphicsRequirements2KHR"});
      }

      XrGraphicsRequirementsVulkan2KHR vk_reqs;
      vk_reqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
      vk_reqs.next = nullptr;
      result = GetVulkanGraphicsRequirements2(instance, system, &vk_reqs);
      if (!XR_SUCCEEDED(result))
      {
        throw vsg::Exception({"Failed to get Vulkan graphics requirements"});
      }

      // min/max vulkan api version supported by the XR runtime (XR_MAKE_VERSION)
      auto vkMinVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(vk_reqs.minApiVersionSupported), XR_VERSION_MINOR(vk_reqs.minApiVersionSupported), XR_VERSION_PATCH(vk_reqs.minApiVersionSupported));
      auto vkMaxVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(vk_reqs.maxApiVersionSupported), XR_VERSION_MINOR(vk_reqs.maxApiVersionSupported), XR_VERSION_PATCH(vk_reqs.maxApiVersionSupported));
      if (vulkanApiVersion > vkMaxVersion || vulkanApiVersion < vkMinVersion) {
        throw vsg::Exception({"OpenXR runtime doesn't support requested Vulkan version"});
      }
    }

    // Find out which instance extensions are needed by OpenXR
    auto xrRequiredVulkanInstanceExtensions = getVulkanInstanceExtensionsRequired(instance, system);

    // Initialise a vulkan instance, using the OpenXR device
    VkInstance vkInstance;
    {
      VkApplicationInfo applicationInfo;
      applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      applicationInfo.pNext = nullptr;
      applicationInfo.pApplicationName = applicationName.c_str();
      applicationInfo.applicationVersion = applicationVersion;
      applicationInfo.pEngineName = engineName.c_str();
      applicationInfo.engineVersion = engineVersion;
      applicationInfo.apiVersion = vulkanApiVersion;

      auto tmpInstanceExtensions = vulkanRequiredInstanceExtensions;
      for (auto x : xrRequiredVulkanInstanceExtensions) tmpInstanceExtensions.push_back(x.data());

      VkInstanceCreateInfo instanceInfo;
      instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instanceInfo.pNext = nullptr;
      instanceInfo.flags = 0;
      instanceInfo.pApplicationInfo = &applicationInfo;
      instanceInfo.enabledLayerCount = 0;
      instanceInfo.ppEnabledLayerNames = nullptr;
      instanceInfo.enabledExtensionCount = static_cast<uint32_t>(tmpInstanceExtensions.size());
      instanceInfo.ppEnabledExtensionNames = tmpInstanceExtensions.empty() ? nullptr : tmpInstanceExtensions.data();

      XrVulkanInstanceCreateInfoKHR xrVulkanCreateInfo;
      xrVulkanCreateInfo.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
      xrVulkanCreateInfo.next = NULL;
      xrVulkanCreateInfo.systemId = system;
      xrVulkanCreateInfo.createFlags = 0;
      xrVulkanCreateInfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
      xrVulkanCreateInfo.vulkanCreateInfo = &instanceInfo;
      xrVulkanCreateInfo.vulkanAllocator = NULL;

      PFN_xrCreateVulkanInstanceKHR CreateVulkanInstanceKHR = nullptr;
      if (!XR_SUCCEEDED(xrGetInstanceProcAddr(instance, "xrCreateVulkanInstanceKHR",
        (PFN_xrVoidFunction*)&CreateVulkanInstanceKHR)))
      {
        throw vsg::Exception({"Failed to find xrCreateVulkanInstanceKHR"});
      }

      VkResult vkResult;
      if (!XR_SUCCEEDED(CreateVulkanInstanceKHR(instance, &xrVulkanCreateInfo, &vkInstance, &vkResult)))
      {
        throw vsg::Exception({"Failed to create Vulkan instance"});
      }
      if (vkResult != VK_SUCCESS)
      {
        throw vsg::Exception({"OpenXR runtime failed to create Vulkan instance"});
      }
    }

    // Now fetch the Vulkan device - OpenXR will require the specific device,
    // which is attached to the display
    VkPhysicalDevice vkPhysicalDevice;
    {
      PFN_xrGetVulkanGraphicsDevice2KHR fun = nullptr;
      if (!XR_SUCCEEDED(xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDevice2KHR",
        (PFN_xrVoidFunction*)&fun)))
      {
        throw vsg::Exception({"Failed to find xrGetVulkanGraphicsDevice2KHR"});
      }

      XrVulkanGraphicsDeviceGetInfoKHR info;
      info.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
      info.next = nullptr;
      info.systemId = system;
      info.vulkanInstance = vkInstance;

      if (!XR_SUCCEEDED(fun(instance, &info, &vkPhysicalDevice)))
      {
        throw vsg::Exception({"Failed to get Vulkan graphics device"});
      }
    }

    auto vsgInstance = vsgvr::OpenXRVkInstance::create(vkInstance);
    auto vsgPhysicalDevice = vsgvr::OpenXRVkPhysicalDevice::create(vsgInstance, vkPhysicalDevice);
    vsgInstance->setPhysicalDevice(vsgPhysicalDevice);

    return vsgInstance;
  }

  vsg::ref_ptr<vsg::Device> getVulkanDevice(XrInstance instance, XrSystemId system, vsg::ref_ptr<vsg::Instance> vsgInstance, vsg::ref_ptr<vsg::PhysicalDevice> vsgPhysicalDevice) {

    auto qFamily = vsgPhysicalDevice->getQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
    if (qFamily < 0)
    {
      throw vsg::Exception({"Failed to locate graphics queue"});
    }

    PFN_xrCreateVulkanDeviceKHR fun = nullptr;
    if (!XR_SUCCEEDED(xrGetInstanceProcAddr(instance, "xrCreateVulkanDeviceKHR", (PFN_xrVoidFunction*)&fun)))
    {
      throw vsg::Exception({"Failed to find xrCreateVulkanDeviceKHR"});
    }

    // Create the Vulkan Device through OpenXR - It will add any extensions required
    float qPriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo qInfo;
    qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qInfo.pNext = nullptr;
    qInfo.flags = 0;
    qInfo.queueFamilyIndex = qFamily;
    qInfo.queueCount = 1;
    qInfo.pQueuePriorities = qPriorities;

    VkPhysicalDeviceFeatures enabledFeatures;
    std::memset(&enabledFeatures, 0x00, sizeof(enabledFeatures));
    enabledFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &qInfo;
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = nullptr;
    deviceInfo.enabledExtensionCount = 0;
    deviceInfo.ppEnabledExtensionNames = nullptr;
    deviceInfo.pEnabledFeatures = &enabledFeatures;

    XrVulkanDeviceCreateInfoKHR info;
    info.type = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR;
    info.next = nullptr;
    info.systemId = system;
    info.createFlags = 0;
    info.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
    info.vulkanPhysicalDevice = vsgPhysicalDevice->getPhysicalDevice();
    info.vulkanCreateInfo = &deviceInfo;
    info.vulkanAllocator = nullptr;

    VkDevice vkDevice;
    VkResult vkResult;
    if (!XR_SUCCEEDED(fun(instance, &info, &vkDevice, &vkResult)))
    {
      throw vsg::Exception({"Failed to create Vulkan device"});
    }
    if (vkResult != VK_SUCCESS)
    {
      throw vsg::Exception({"OpenXR failed to create Vulkan device"});
    }

    auto vsgDevice = vsgvr::OpenXRVkDevice::create(vsgInstance, vsgPhysicalDevice, vkDevice);
    vsgDevice->getQueue(qFamily, 0); // To populate Device::_queues
    return vsgDevice;
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
    XrSession session;

    // TODO: Expose application settings to app, correct engine version from some version header / vsg base
    const std::vector<const char*> sRequiredXrInstanceExtensions = { "XR_KHR_vulkan_enable", "XR_KHR_vulkan_enable2" };
    const std::vector<const char*> sRequiredXrLayers = {};
    const std::string sApplicationName = "VSGVR Application";
    const uint32_t sApplicationVersion = 0;
    const std::string sEngineName = "VSGVR";
    const uint32_t sEngineVersion = 0;

    void preVulkanInit() {
      instance = createInstance(
        sRequiredXrInstanceExtensions,
        sRequiredXrLayers,
        sApplicationName,
        sApplicationVersion,
        sEngineName,
        sEngineVersion,
        XR_MAKE_VERSION(1, 0, 0)
      );

      {
        // TODO: Expose display type to application
        auto sys = getSystem(instance, XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY);
        systemID = sys.first;
        systemProperties = sys.second;
      }
    }

    vsg::ref_ptr<vsg::Window> createVSGWindow(vsg::ref_ptr<vsg::WindowTraits> traits) {

      // TODO:
      // - Need preVulkanInit to have been called
      // - Need to create a vulkan instance through OpenXR
      // - Need to create a vsg::Window instance, with those bits

      auto vsgInstance = getVulkanInstance(
        instance, systemID, traits->vulkanVersion, traits->instanceExtensionNames,
        sApplicationName, sApplicationVersion, sEngineName, sEngineVersion
      );

      auto vsgPhysicalDevice = vsgInstance->getPhysicalDevices().front();

      auto vsgDevice = getVulkanDevice(instance, systemID, vsgInstance, vsgPhysicalDevice);

      auto window = vsg::Window::create(traits);
      window->setInstance(vsgInstance);
      window->setPhysicalDevice(vsgPhysicalDevice);
      window->setDevice(vsgDevice);
      return window;
    }
    /*
    void init(vsg::ref_ptr<vsg::Window> renderWindow) {
      if( intialised ) return;

      renderWindow->getOrCreateInstance();
      renderWindow->getOrCreatePhysicalDevice();
      renderWindow->getOrCreateDevice();

      // Window::_initDevice()

      auto qFamily = renderWindow->getPhysicalDevice()->getQueueFamily(renderWindow->traits()->queueFlags);
      // uint32_t qFamily = 0;

      //auto [graphicsFamily, presentFamily] = renderWindow->getOrCreatePhysicalDevice()->getQueueFamily(
      //  renderWindow->traits()->queueFlags, renderWindow->getOrCreateSurface()
      //);

      vkDeviceWaitIdle(*renderWindow->getDevice());
      // MAYBE MAYBE: vsg is going off and building swapchains, but those are for the desktop window - Possibly we need to avoid that before hitting here?
      session = createSession(instance, systemID,
        *renderWindow->getInstance(),
        *renderWindow->getPhysicalDevice(),
        *renderWindow->getDevice(),
        qFamily,
        0 // TODO: Always queue 0 in core vsg? Seems unreliable.
      );

      intialised = true;
    }*/

    void deinit() {
      destroyInstance(instance);
    }

    std::list<std::string> instanceExtensionsRequired(uint32_t vkVersion) {
      // Viewer will always need to check for extensions, so also perform the api version check here
      auto graphicsRequirements = getVulkanGraphicsRequirements(instance, systemID);
      if (vkVersion < graphicsRequirements.minApiVersionSupported || vkVersion > graphicsRequirements.maxApiVersionSupported)
      {
        vsg::Exception e;
        e.message = "Vulkan version not supported by OpenXR runtime";
      }

      return getVulkanInstanceExtensionsRequired(instance, systemID);
    }

    std::list<std::string> deviceExtensionsRequired(vsg::ref_ptr<vsg::Instance> vulkanInstance, vsg::ref_ptr<vsg::PhysicalDevice> vulkanPhysicalDevice) {
      // TODO: This also checks that the physical device OpenXR requires is the same as the one the window has selected.
      // Device selection needs refactoring for the OpenXR approach, context needs to be able to instruct
      // the window which device to use (likely core vsg change needed)
      getVulkanDeviceExtensionsRequired(instance, systemID, vulkanInstance, vulkanPhysicalDevice);
      return {};
    }
  };

  OpenXRContext::OpenXRContext(vsgvr::VRContext::TrackingOrigin origin)
    : m(new OpenXRContextImpl())
  {
    m->preVulkanInit();
  }

  OpenXRContext::~OpenXRContext()
  {
    m->deinit();
  }

  vsg::ref_ptr<vsg::Window> OpenXRContext::createVSGWindow(vsg::ref_ptr<vsg::WindowTraits> traits)
  {
    return m->createVSGWindow(traits);
  }

  //void OpenXRContext::init(vsg::ref_ptr<vsg::Window> renderWindow)
  //{
  //  m->init(renderWindow);
  //}

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
    return {};
  }

  std::vector<vsg::dmat4> OpenXRContext::getEyeToHeadTransforms()
  {
    // TODO
    return {};
  }

  uint32_t OpenXRContext::numberOfHmdImages() const
  {
    // TODO: Doesn't apply to XR?
    return 0;
  }

  bool OpenXRContext::getRecommendedTargetSize(uint32_t& width, uint32_t& height)
  {
    // TODO: Doesn't apply to XR?
    return false;
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
