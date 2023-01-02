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

#pragma once

#include <vsgvr/xr/Common.h>
#include <vsgvr/xr/Instance.h>

#include <vsg/core/Inherit.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/PhysicalDevice.h>
#include <vsg/vk/Device.h>
#include <vulkan/vulkan.h>

#include <set>
#include <string>

namespace vsgvr {
    struct VSGVR_DECLSPEC VulkanRequirements
    {
      uint32_t minVersion;
      uint32_t maxVersion;
      std::string minVersionStr;
      std::string maxVersionStr;
      std::set<std::string> instanceExtensions;
      std::set<std::string> deviceExtensions;
    };

    struct VSGVR_DECLSPEC VulkanGraphicsRequirements
    {
      VkPhysicalDevice* physicalDevice;
    };

    class VSGVR_DECLSPEC GraphicsBindingVulkan : public vsg::Inherit<vsg::Object, GraphicsBindingVulkan>
    {
        public:
            static VulkanRequirements getVulkanRequirements(vsg::ref_ptr<Instance> xrInstance);
            static VkPhysicalDevice getVulkanDeviceRequirements(vsg::ref_ptr<Instance> xrInstance, vsg::ref_ptr<vsg::Instance> vsgInstance, const VulkanRequirements& versionReqs);

            GraphicsBindingVulkan() = delete;
            GraphicsBindingVulkan(vsg::ref_ptr<vsg::Instance> vsgInstance, vsg::ref_ptr<vsg::PhysicalDevice> vsgPhysicalDevice, vsg::ref_ptr<vsg::Device> vsgDevice, uint32_t queueFamilyIndex, uint32_t queueIndex);
            ~GraphicsBindingVulkan();

            vsg::ref_ptr<vsg::Instance> getVsgInstance() const { return _vsgInstance; }
            vsg::ref_ptr<vsg::PhysicalDevice> getVsgPhysicalDevice() const { return _vsgPhysicalDevice; }
            vsg::ref_ptr<vsg::Device> getVsgDevice() const { return _vsgDevice; }

            const XrGraphicsBindingVulkanKHR& getBinding() const { return _binding; }

        private:
            vsg::ref_ptr<vsg::Instance> _vsgInstance;
            vsg::ref_ptr<vsg::PhysicalDevice> _vsgPhysicalDevice;
            vsg::ref_ptr<vsg::Device> _vsgDevice;

            XrGraphicsBindingVulkanKHR _binding;
    };
}

EVSG_type_name(vsgvr::GraphicsBindingVulkan);
