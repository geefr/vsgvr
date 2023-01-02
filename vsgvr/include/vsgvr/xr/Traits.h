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

#include <vsg/core/Inherit.h>

#include <string>
#include <vector>

namespace vsgvr
{
  // Application and runtime settings for vsgvr
  class VSGVR_DECLSPEC Traits : public vsg::Inherit<vsg::Object, Traits>
  {
  public:
    Traits();

    // OpenXR extensions required by the application
    std::vector<std::string> xrExtensions = {
        // "XR_EXT_debug_utils"
    };
    std::vector<std::string> xrLayers = {
        // "XR_APILAYER_LUNARG_core_validation"
    };

    // Application metadata provided to the OpenXR runtime
    XrVersion apiVersion = XR_MAKE_VERSION(1, 0, 0);
    std::string applicationName = "VSGVR Application";
    std::string engineName = "VSGVR";
    uint32_t engineVersion = 0;
    uint32_t applicationVersion = 0;

    void setApplicationVersion(uint32_t maj, uint32_t min, uint32_t patch);
    void setEngineVersion(uint32_t maj, uint32_t min, uint32_t patch);

    // The configuration of displays used for the form factor
    // May be validated via Instance::checkViewConfigurationSupported
    XrViewConfigurationType viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    // The mode used to blend rendered data with the environment. This may vary based on the
    // capabilities of a headset, such as whether visual or camera-based environment is supported
    // May be validated via Instance::checkEnvironmentBlendModeSupported
    XrEnvironmentBlendMode environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;

    // The Vulkan image format used for OpenXR swapchain images (i.e. the render target for vsg content)
    // May be validated via Session::checkSwapchainFormatSupported
    // Runtimes should support VK_FORMAT_R8G8B8A8_UNORM and VK_FORMAT_R8G8B8A8_SRGB as preferred formats
    VkFormat swapchainFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // The number of samples used for swapchain images
    // May be validated via Session::checkSwapchainSampleCountSupported
    uint32_t swapchainSampleCount = 1;

  protected:
    virtual ~Traits();
  };
}

EVSG_type_name(vsgvr::Traits);
