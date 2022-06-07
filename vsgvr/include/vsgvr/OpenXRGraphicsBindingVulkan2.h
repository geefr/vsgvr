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

#include <vsg/core/Inherit.h>

#include <vsgvr/OpenXRCommon.h>
#include <vsgvr/OpenXRTraits.h>

#include <vsgvr/OpenXRVkInstance.h>
#include <vsgvr/OpenXRVkPhysicalDevice.h>
#include <vsgvr/OpenXRVkDevice.h>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRGraphicsBindingVulkan2 : public vsg::Inherit<vsg::Object, OpenXRGraphicsBindingVulkan2>
    {
        public:
            OpenXRGraphicsBindingVulkan2() = delete;
            OpenXRGraphicsBindingVulkan2(XrInstance instance, XrSystemId system, OpenXrTraits traits, OpenXrVulkanTraits vkTraits);
            ~OpenXRGraphicsBindingVulkan2();

            const XrGraphicsBindingVulkan2KHR& getBinding() const { return _binding; }

            vsg::ref_ptr<vsg::Instance> getVkInstance() const { return _vkInstance; }
            vsg::ref_ptr<vsg::PhysicalDevice> getVkPhysicalDevice() const { return _vkPhysicalDevice; }
            vsg::ref_ptr<vsg::Device> getVkDevice() const { return _vkDevice; }

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
