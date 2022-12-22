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

#include <vsgvr/xr/Instance.h>

#include <vsg/core/Exception.h>
#include "Macros.cpp"

#ifdef ANDROID
# include <vsgvr/xr/AndroidTraits.h>
#endif

using namespace vsg;

namespace vsgvr
{
    Instance::Instance(vsg::ref_ptr<Traits> xrTraits)
        : _xrTraits(xrTraits)
    {
        createInstance();
        createSystem();
    }

    Instance::~Instance()
    {
        destroyInstance();
    }

    void Instance::onEventInstanceLossPending([[maybe_unused]] const XrEventDataInstanceLossPending &event)
    {
        // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrEventDataInstanceLossPending
        // TODO: This indicates the overall runtime is about to become unavailable. Given that encountering
        //       the subsequent XR_ERROR_RUNTIME_UNAVAILABLE when re-creating the instance isn't handled,
        //       just throw an exception.
        throw Exception({"OpenXR: Instance loss pending"});
    }

    void Instance::createInstance()
    {
        if (_instance)
        {
            throw Exception({"Viewer: Instance already initialised"});
        }
        std::vector<const char *> extensions = {
            "XR_KHR_vulkan_enable"
        };

#ifdef ANDROID
        // Before any XR functions may be called we need to check if additional values are needed
        // for runtime intiailisation. See XR_KHR_loader_init.
        // Presence of this extension / requirement is denoted by a getProcAddress lookup.
        auto fn = (PFN_xrInitializeLoaderKHR)xr_pfn_noexcept(XR_NULL_HANDLE, "xrInitializeLoaderKHR");
        if( fn )
        {
            auto androidTraits = _xrTraits.cast<vsgvr::AndroidTraits>();
            if( androidTraits && androidTraits->vm && androidTraits->activity )
            {
                XrLoaderInitInfoAndroidKHR loaderInitInfo;
                loaderInitInfo.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
                loaderInitInfo.next = nullptr;
                loaderInitInfo.applicationVM = static_cast<void*>(androidTraits->vm);
                loaderInitInfo.applicationContext = static_cast<void*>(androidTraits->activity);
                fn(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR*>(&loaderInitInfo));
            }
            else
            {
                throw Exception({"On Android an AndroidTraits structure must be provided, with valid JNI/Activity pointers."});
            }
        }

        extensions.push_back("XR_KHR_android_create_instance");
#endif

        for (auto &e : _xrTraits->xrExtensions)
            extensions.push_back(e.c_str());

        std::vector<const char *> layers = {};
        for (auto &l : _xrTraits->xrLayers)
            layers.push_back(l.c_str());

        XrInstanceCreateInfo info;
        info.type = XR_TYPE_INSTANCE_CREATE_INFO;
        info.next = nullptr;
        info.createFlags = 0;
        info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
        info.enabledApiLayerNames = layers.empty() ? nullptr : layers.data();
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.enabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

        strncpy(info.applicationInfo.applicationName, _xrTraits->applicationName.c_str(), std::min(static_cast<int>(_xrTraits->applicationName.size() + 1), XR_MAX_APPLICATION_NAME_SIZE));
        info.applicationInfo.applicationVersion = _xrTraits->applicationVersion;
        strncpy(info.applicationInfo.engineName, _xrTraits->engineName.c_str(), std::min(static_cast<int>(_xrTraits->engineName.size() + 1), XR_MAX_ENGINE_NAME_SIZE));
        info.applicationInfo.apiVersion = _xrTraits->apiVersion;
        info.applicationInfo.engineVersion = _xrTraits->engineVersion;

#ifdef ANDROID
        auto androidTraits = _xrTraits.cast<vsgvr::AndroidTraits>();
        if( androidTraits && androidTraits->vm && androidTraits->activity )
        {
            XrInstanceCreateInfoAndroidKHR androidCreateInfo;
            androidCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR;
            androidCreateInfo.next = nullptr;
            androidCreateInfo.applicationVM = static_cast<void*>(androidTraits->vm);
            androidCreateInfo.applicationActivity = static_cast<void*>(androidTraits->activity);
            info.next = &androidCreateInfo;
        }
        else
        {
            throw Exception({"On Android an AndroidTraits structure must be provided, with valid JNI/Activity pointers."});
        }
#endif

        _instanceProperties = XrInstanceProperties();
        _instanceProperties.type = XR_TYPE_INSTANCE_PROPERTIES;
        _instanceProperties.next = nullptr;

        xr_check(xrCreateInstance(&info, &_instance), "Failed to create XR Instance");
        xr_check(xrGetInstanceProperties(_instance, &_instanceProperties), "Failed to get XR Instance properties");
    }

    void Instance::destroyInstance()
    {
        if (!_instance)
        {
            throw Exception({"Viewer: Instance not initialised"});
        }
        _system = 0;
        _systemProperties = XrSystemProperties();
        xr_check(xrDestroyInstance(_instance), "Failed to destroy XR Instance");
        _instance = 0;
    }

    void Instance::createSystem()
    {
        XrSystemGetInfo info;
        info.type = XR_TYPE_SYSTEM_GET_INFO;
        info.next = nullptr;
        info.formFactor = _xrTraits->formFactor;

        xr_check(xrGetSystem(_instance, &info, &_system), "Failed to get OpenXR system");

        _systemProperties = XrSystemProperties();
        _systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
        _systemProperties.next = nullptr;
        _systemProperties.graphicsProperties = {0, 0, 0};
        _systemProperties.trackingProperties = {XR_FALSE, XR_FALSE};
        xr_check(xrGetSystemProperties(_instance, _system, &_systemProperties), "Failed to get OpenXR system properties");
    }

    void Instance::validateTraits()
    {
        // View configuration type
        {
            std::vector<XrViewConfigurationType> types;
            uint32_t count = 0;
            xr_check(xrEnumerateViewConfigurations(_instance, _system, 0, &count, nullptr));
            types.resize(count);
            xr_check(xrEnumerateViewConfigurations(_instance, _system, static_cast<uint32_t>(types.size()), &count, types.data()));
            if (std::find(types.begin(), types.end(), _xrTraits->viewConfigurationType) == types.end())
            {
                throw Exception({"View configuration type not supported"});
            }
        }

        // Environment blend mode
        {
            std::vector<XrEnvironmentBlendMode> modes;
            uint32_t count = 0;
            xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, _xrTraits->viewConfigurationType, 0, &count, nullptr));
            modes.resize(count);
            xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, _xrTraits->viewConfigurationType, static_cast<uint32_t>(modes.size()), &count, modes.data()));
            if (std::find(modes.begin(), modes.end(), _xrTraits->environmentBlendMode) == modes.end())
            {
                throw Exception({"Environment blend mode not supported"});
            }
        }
    }
}

