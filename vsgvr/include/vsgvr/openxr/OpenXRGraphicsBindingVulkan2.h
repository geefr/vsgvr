#pragma once

#include <vsg/core/Inherit.h>

#include <vsgvr/openxr/OpenXR.h>
#include <vsgvr/openxr/OpenXRTraits.h>

#include <vsgvr/openxr/OpenXRVkInstance.h>
#include <vsgvr/openxr/OpenXRVkPhysicalDevice.h>
#include <vsgvr/openxr/OpenXRVkDevice.h>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRGraphicsBindingVulkan2 : public vsg::Inherit<vsg::Object, OpenXRGraphicsBindingVulkan2>
    {
        public:
            OpenXRGraphicsBindingVulkan2() = delete;
            OpenXRGraphicsBindingVulkan2(XrInstance instance, XrSystemId system, OpenXrTraits traits, OpenXrVulkanTraits vkTraits);
            ~OpenXRGraphicsBindingVulkan2();
        private:
            void createVulkanInstance(XrInstance instance, XrSystemId system, OpenXrTraits traits, OpenXrVulkanTraits vkTraits);
            void createVulkanPhysicalDevice(XrInstance instance, XrSystemId system, OpenXrTraits traits, OpenXrVulkanTraits vkTraits);
            void createVulkanDevice(XrInstance instance, XrSystemId system, OpenXrTraits traits, OpenXrVulkanTraits vkTraits);
            void destroyVulkanDevice();
            void destroyVulkanPhysicalDevice();
            void destroyVulkanInstance();
            

            vsg::ref_ptr<vsgvr::OpenXRVkInstance> _vkInstance;
            vsg::ref_ptr<vsgvr::OpenXRVkPhysicalDevice> _vkPhysicalDevice;
            vsg::ref_ptr<vsgvr::OpenXRVkDevice> _vkDevice;

            XrGraphicsRequirementsVulkan2KHR _graphicsRequirements;
            XrGraphicsBindingVulkan2KHR _binding;
    };
}
