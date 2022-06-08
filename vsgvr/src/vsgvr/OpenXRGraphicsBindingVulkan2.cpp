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

#include <vsgvr/OpenXRGraphicsBindingVulkan2.h>

#include <vsg/core/Exception.h>

#include "OpenXRMacros.cpp"

#include <string>
#include <sstream>
#include <vector>

using namespace vsg;

namespace vsgvr {
  OpenXRGraphicsBindingVulkan2::OpenXRGraphicsBindingVulkan2(vsg::ref_ptr<OpenXRInstance> instance, OpenXrTraits traits, OpenXrVulkanTraits vkTraits)
  {
    createVulkanInstance(instance, traits, vkTraits);
    createVulkanPhysicalDevice(instance, traits, vkTraits);
    createVulkanDevice(instance, traits, vkTraits);

    _binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
    _binding.next = nullptr;
    _binding.instance = _vkInstance->getInstance();
    _binding.physicalDevice = _vkPhysicalDevice->getPhysicalDevice();
    _binding.device = _vkDevice->getDevice();
    _binding.queueFamilyIndex = _vkPhysicalDevice->getQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
    _binding.queueIndex = 0;
  }

  OpenXRGraphicsBindingVulkan2::~OpenXRGraphicsBindingVulkan2()
  {
    destroyVulkanDevice();
    destroyVulkanPhysicalDevice();
    destroyVulkanInstance();
  }

  void OpenXRGraphicsBindingVulkan2::createVulkanInstance(vsg::ref_ptr<OpenXRInstance> instance, OpenXrTraits traits, OpenXrVulkanTraits vkTraits)
  {
    {
      _graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
      _graphicsRequirements.next = nullptr;
      auto fn = (PFN_xrGetVulkanGraphicsRequirements2KHR)xr_pfn(instance->getInstance(), "xrGetVulkanGraphicsRequirements2KHR");
      xr_check(fn(instance->getInstance(), instance->getSystem(), &_graphicsRequirements), "Failed to get Vulkan requirements");
    }
    // min/max vulkan api version supported by the XR runtime (XR_MAKE_VERSION)
    auto vkMinVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(_graphicsRequirements.minApiVersionSupported), XR_VERSION_MINOR(_graphicsRequirements.minApiVersionSupported), XR_VERSION_PATCH(_graphicsRequirements.minApiVersionSupported));
    auto vkMaxVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(_graphicsRequirements.maxApiVersionSupported), XR_VERSION_MINOR(_graphicsRequirements.maxApiVersionSupported), XR_VERSION_PATCH(_graphicsRequirements.maxApiVersionSupported));
    if (vkTraits.vulkanVersion > vkMaxVersion || vkTraits.vulkanVersion < vkMinVersion) {
      throw Exception({ "OpenXR runtime doesn't support requested Vulkan version" });
    }

    std::vector<std::string> vkInstanceExtensions;
    {
      auto fn = (PFN_xrGetVulkanInstanceExtensionsKHR)xr_pfn(instance->getInstance(), "xrGetVulkanInstanceExtensionsKHR");
      uint32_t size = 0;
      xr_check(fn(instance->getInstance(), instance->getSystem(), 0, &size, nullptr), "Failed to get instance extensions (num)");
      std::string names;
      names.reserve(size);
      xr_check(fn(instance->getInstance(), instance->getSystem(), size, &size, names.data()), "Failed to get instance extensions");
      // Single-space delimited
      std::stringstream s(names);
      std::string name;
      while (std::getline(s, name)) vkInstanceExtensions.push_back(name);
    }
    for (auto& e : vkTraits.vulkanInstanceExtensions) vkInstanceExtensions.push_back(e);

    std::vector<std::string> vkDeviceExtensions;
    {
      auto GetDeviceExtensions = (PFN_xrGetVulkanDeviceExtensionsKHR)xr_pfn(instance->getInstance(), "xrGetVulkanDeviceExtensionsKHR");
      uint32_t size = 0;
      xr_check(GetDeviceExtensions(instance->getInstance(), instance->getSystem(), 0, &size, nullptr), "Failed to get device extensions (num)");
      std::string names;
      names.reserve(size);
      xr_check(GetDeviceExtensions(instance->getInstance(), instance->getSystem(), size, &size, names.data()), "Failed to get device extensions");
      // Single-space delimited
      std::stringstream s(names);
      std::string name;
      while (std::getline(s, name)) vkDeviceExtensions.push_back(name);
    }

    // Initialise a vulkan instance, using the OpenXR device
    {
      auto applicationInfo = VkApplicationInfo();
      applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      applicationInfo.pNext = nullptr;
      applicationInfo.pApplicationName = traits.applicationName.c_str();
      applicationInfo.applicationVersion = traits.applicationVersion;
      applicationInfo.pEngineName = traits.engineName.c_str();
      applicationInfo.engineVersion = traits.engineVersion;
      applicationInfo.apiVersion = vkTraits.vulkanVersion;

      std::vector<const char*> tmpInstanceExtensions;
      for (auto& x : vkInstanceExtensions) tmpInstanceExtensions.push_back(x.data());

      auto instanceInfo = VkInstanceCreateInfo();
      instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instanceInfo.pNext = nullptr;
      instanceInfo.flags = 0;
      instanceInfo.pApplicationInfo = &applicationInfo;
      instanceInfo.enabledLayerCount = 0;
      instanceInfo.ppEnabledLayerNames = nullptr;
      instanceInfo.enabledExtensionCount = static_cast<uint32_t>(tmpInstanceExtensions.size());
      instanceInfo.ppEnabledExtensionNames = tmpInstanceExtensions.empty() ? nullptr : tmpInstanceExtensions.data();

      auto xrVulkanCreateInfo = XrVulkanInstanceCreateInfoKHR();
      xrVulkanCreateInfo.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
      xrVulkanCreateInfo.next = NULL;
      xrVulkanCreateInfo.systemId = instance->getSystem();
      xrVulkanCreateInfo.createFlags = 0;
      xrVulkanCreateInfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
      xrVulkanCreateInfo.vulkanCreateInfo = &instanceInfo;
      // TODO: Support for custom allocators - vsg has those
      xrVulkanCreateInfo.vulkanAllocator = NULL;

      auto CreateVulkanInstanceKHR = (PFN_xrCreateVulkanInstanceKHR)xr_pfn(instance->getInstance(), "xrCreateVulkanInstanceKHR");
      VkResult vkResult;
      VkInstance vkInstance;
      // TODO: Log vulkan error as well
      xr_check(CreateVulkanInstanceKHR(instance->getInstance(), &xrVulkanCreateInfo, &vkInstance, &vkResult), "Failed to create Vulkan Instance");

      _vkInstance = vsgvr::OpenXRVkInstance::create(vkInstance);
    }
  }
  void OpenXRGraphicsBindingVulkan2::createVulkanPhysicalDevice(vsg::ref_ptr<OpenXRInstance> instance, OpenXrTraits traits, OpenXrVulkanTraits vkTraits)
  {
    // Now fetch the Vulkan device - OpenXR will require the specific device,
    // which is attached to the display
    {
      auto fn = (PFN_xrGetVulkanGraphicsDevice2KHR)xr_pfn(instance->getInstance(), "xrGetVulkanGraphicsDevice2KHR");

      auto info = XrVulkanGraphicsDeviceGetInfoKHR();
      info.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
      info.next = nullptr;
      info.systemId = instance->getSystem();
      info.vulkanInstance = _vkInstance->getInstance();

      VkPhysicalDevice vkPhysicalDevice;
      xr_check(fn(instance->getInstance(), &info, &vkPhysicalDevice), "Failed to get Vulkan physical device from OpenXR");
      _vkPhysicalDevice = vsgvr::OpenXRVkPhysicalDevice::create(_vkInstance, vkPhysicalDevice);
    }
  }

  void OpenXRGraphicsBindingVulkan2::createVulkanDevice(vsg::ref_ptr<OpenXRInstance> instance, OpenXrTraits traits, OpenXrVulkanTraits vkTraits)
  {
    auto qFamily = _vkPhysicalDevice->getQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
    if (qFamily < 0)
    {
      throw Exception({ "Failed to locate graphics queue" });
    }

    auto CreateVulkanDeviceKHR = (PFN_xrCreateVulkanDeviceKHR)xr_pfn(instance->getInstance(), "xrCreateVulkanDeviceKHR");

    // Create the Vulkan Device through OpenXR - It will add any extensions required
    float qPriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo qInfo;
    qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qInfo.pNext = nullptr;
    qInfo.flags = 0;
    qInfo.queueFamilyIndex = qFamily;
    qInfo.queueCount = 1;
    qInfo.pQueuePriorities = qPriorities;

    auto enabledFeatures = VkPhysicalDeviceFeatures();
    enabledFeatures.samplerAnisotropy = VK_TRUE;

    auto deviceInfo = VkDeviceCreateInfo();
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

    auto info = XrVulkanDeviceCreateInfoKHR();
    info.type = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR;
    info.next = nullptr;
    info.systemId = instance->getSystem();
    info.createFlags = 0;
    info.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
    info.vulkanPhysicalDevice = _vkPhysicalDevice->getPhysicalDevice();
    info.vulkanCreateInfo = &deviceInfo;
    info.vulkanAllocator = nullptr;

    VkDevice vkDevice;
    VkResult vkResult;
    xr_check(CreateVulkanDeviceKHR(instance->getInstance(), &info, &vkDevice, &vkResult), "Failed to create Vulkan Device");

    _vkDevice = vsgvr::OpenXRVkDevice::create(_vkInstance, _vkPhysicalDevice, vkDevice);
    _vkDevice->getQueue(qFamily, 0); // To populate Device::_queues
  }

  void OpenXRGraphicsBindingVulkan2::destroyVulkanDevice() {
    _vkDevice = 0;
  }

  void OpenXRGraphicsBindingVulkan2::destroyVulkanPhysicalDevice() {
    _vkPhysicalDevice = 0;
  }

  void OpenXRGraphicsBindingVulkan2::destroyVulkanInstance()
  {
    _vkDevice = 0;
    _vkPhysicalDevice = 0;
    _vkInstance = 0;
  }
}
