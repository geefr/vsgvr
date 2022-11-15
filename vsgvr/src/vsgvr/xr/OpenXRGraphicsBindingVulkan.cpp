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

#include <vsgvr/xr/OpenXRGraphicsBindingVulkan.h>

#include <vsg/core/Exception.h>

#include "OpenXRMacros.cpp"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

using namespace vsg;

namespace vsgvr {
  OpenXRGraphicsBindingVulkan::OpenXRGraphicsBindingVulkan(vsg::ref_ptr<vsg::Instance> vkInstance, vsg::ref_ptr<vsg::PhysicalDevice> vkPhysicalDevice, vsg::ref_ptr<vsg::Device> vkDevice, uint32_t queueFamilyIndex, uint32_t queueIndex)
    : _vkInstance(vkInstance)
    , _vkPhysicalDevice(vkPhysicalDevice)
    , _vkDevice(vkDevice)
  {
    _binding = XrGraphicsBindingVulkanKHR();
    _binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    _binding.next = nullptr;
    _binding.instance = _vkInstance->vk();
    _binding.physicalDevice = _vkPhysicalDevice->vk();
    _binding.device = _vkDevice->vk();
    _binding.queueFamilyIndex = queueFamilyIndex;
    _binding.queueIndex = queueIndex;
  }

  OpenXRGraphicsBindingVulkan::~OpenXRGraphicsBindingVulkan() {}

  VulkanRequirements OpenXRGraphicsBindingVulkan::getVulkanRequirements(vsg::ref_ptr<OpenXRInstance> xrInstance)
  {
    VulkanRequirements reqs;
    {
      auto graphicsRequirements = XrGraphicsRequirementsVulkanKHR();
      graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
      graphicsRequirements.next = nullptr;
      auto fn = (PFN_xrGetVulkanGraphicsRequirementsKHR)xr_pfn(xrInstance->getInstance(), "xrGetVulkanGraphicsRequirementsKHR");
      xr_check(fn(xrInstance->getInstance(), xrInstance->getSystem(), &graphicsRequirements), "Failed to get Vulkan requirements");

      // min/max vulkan api version supported by the XR runtime (XR_MAKE_VERSION)
      reqs.minVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported), XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported), XR_VERSION_PATCH(graphicsRequirements.minApiVersionSupported));
      reqs.maxVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported), XR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported), XR_VERSION_PATCH(graphicsRequirements.maxApiVersionSupported));
      std::stringstream ss;
      ss << XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported) << "." << XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported) << "." << XR_VERSION_PATCH(graphicsRequirements.minApiVersionSupported);
      reqs.minVersionStr = ss.str();
      ss = std::stringstream();
      ss << XR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported) << "." << XR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported) << "." << XR_VERSION_PATCH(graphicsRequirements.maxApiVersionSupported);
      reqs.maxVersionStr = ss.str();
    }

    {
      auto fn = (PFN_xrGetVulkanInstanceExtensionsKHR)xr_pfn(xrInstance->getInstance(), "xrGetVulkanInstanceExtensionsKHR");
      uint32_t size = 0;
      xr_check(fn(xrInstance->getInstance(), xrInstance->getSystem(), 0, &size, nullptr), "Failed to get instance extensions (num)");

      std::string names;
      names.resize(size);
      xr_check(fn(xrInstance->getInstance(), xrInstance->getSystem(), size, &size, names.data()), "Failed to get instance extensions");

      // Single-space delimited
      std::stringstream s(names);
      std::string name;
      while (std::getline(s, name, static_cast<char>(0x20)))
      {
        name.erase(name.find_last_not_of('\0') + 1, std::string::npos);
        reqs.instanceExtensions.insert(name);
      }
    }

    {
      auto fn = (PFN_xrGetVulkanDeviceExtensionsKHR)xr_pfn(xrInstance->getInstance(), "xrGetVulkanDeviceExtensionsKHR");
      uint32_t size = 0;
      xr_check(fn(xrInstance->getInstance(), xrInstance->getSystem(), 0, &size, nullptr), "Failed to get device extensions (num)");
      std::string names;
      names.resize(size);
      xr_check(fn(xrInstance->getInstance(), xrInstance->getSystem(), size, &size, names.data()), "Failed to get device extensions");
      // Single-space delimited
      std::stringstream s(names);
      std::string name;
      while (std::getline(s, name, static_cast<char>(0x20)))
      {
        name.erase(name.find_last_not_of('\0') + 1, std::string::npos);
        reqs.deviceExtensions.insert(name);
      }
    }

    return reqs;
  }

  VkPhysicalDevice OpenXRGraphicsBindingVulkan::getVulkanDeviceRequirements(vsg::ref_ptr<OpenXRInstance> xrInstance, vsg::ref_ptr<vsg::Instance> vkInstance, const VulkanRequirements& versionReqs)
  {
    if (vkInstance->apiVersion < versionReqs.minVersion) {
      throw Exception({ "OpenXR runtime doesn't support requested Vulkan version" });
    }

    VkPhysicalDevice vkPhysicalDevice;
    {
      auto fn = (PFN_xrGetVulkanGraphicsDeviceKHR)xr_pfn(xrInstance->getInstance(), "xrGetVulkanGraphicsDeviceKHR");
      xr_check(fn(xrInstance->getInstance(), xrInstance->getSystem(), vkInstance->vk(), &vkPhysicalDevice), "Failed to get Vulkan physical device from OpenXR");
    }
    return vkPhysicalDevice;
  }
}
