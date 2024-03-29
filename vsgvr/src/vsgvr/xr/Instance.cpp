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

#include "Macros.cpp"

#ifdef ANDROID
# include <vsgvr/xr/AndroidTraits.h>
#endif

namespace vsgvr
{
  std::vector<XrApiLayerProperties> Instance::getSupportedApiLayers()
  {
    uint32_t count = 0;
    xr_check(xrEnumerateApiLayerProperties(count, &count, nullptr));
    std::vector<XrApiLayerProperties> layers(count);
    for (auto& layer : layers)
    {
      layer.type = XR_TYPE_API_LAYER_PROPERTIES;
      layer.next = nullptr;
    }
    xr_check(xrEnumerateApiLayerProperties(static_cast<uint32_t>(layers.size()), &count, layers.data()));
    return layers;
  }

  std::vector<XrExtensionProperties> Instance::getSupportedInstanceExtensions(const char* apiLayerName)
  {
    uint32_t count = 0;
    xr_check(xrEnumerateInstanceExtensionProperties(apiLayerName, count, &count, nullptr));
    std::vector<XrExtensionProperties> extensions(count);
    for(auto& extension : extensions)
    {
      extension.type = XR_TYPE_EXTENSION_PROPERTIES;
      extension.next = nullptr;
    }
    xr_check(xrEnumerateInstanceExtensionProperties(apiLayerName, static_cast<uint32_t>(extensions.size()), &count, extensions.data()));
    return extensions;
  }

  bool Instance::checkApiLayerSupported(const char* apiLayerName)
  {
    if( apiLayerName == nullptr ) return false;
    auto layers = Instance::getSupportedApiLayers();
    std::string name(apiLayerName);
    auto it = std::find_if(layers.begin(), layers.end(), [&name](const XrApiLayerProperties& x) {
      return name == std::string(x.layerName);
    });
    if( it == layers.end() ) return false;
    return true;
  }

  bool Instance::checkInstanceExtensionSupported(const char* extensionName, const char* apiLayerName)
  {
    if( extensionName == nullptr ) return false;
    auto exts = Instance::getSupportedInstanceExtensions(apiLayerName);
    std::string name(extensionName);
    auto it = std::find_if(exts.begin(), exts.end(), [&name](const XrExtensionProperties& x) {
      return name == x.extensionName;
    });
    if( it == exts.end()) return false;
    return true;
  }

  Instance::Instance(XrFormFactor formFactor, vsg::ref_ptr<vsgvr::Traits> xrTraits)
    : traits(xrTraits)
    , _formFactor(formFactor)
  {
    createInstance();
    createSystem();
  }

  Instance::~Instance()
  {
    destroyInstance();
  }

  std::vector<XrViewConfigurationType> Instance::getSupportedViewConfigurationTypes()
  {
    uint32_t count = 0;
    xr_check(xrEnumerateViewConfigurations(_instance, _system, 0, &count, nullptr));
    std::vector<XrViewConfigurationType> types(count);
    xr_check(xrEnumerateViewConfigurations(_instance, _system, static_cast<uint32_t>(types.size()), &count, types.data()));
    return types;
  }

  std::vector<XrEnvironmentBlendMode> Instance::getSupportedEnvironmentBlendModes(XrViewConfigurationType viewConfiguration)
  {
    uint32_t count = 0;
    xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, viewConfiguration, 0, &count, nullptr));
    std::vector<XrEnvironmentBlendMode> modes(count);
    xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, viewConfiguration, static_cast<uint32_t>(modes.size()), &count, modes.data()));
    return modes;
  }

  bool Instance::checkViewConfigurationSupported(XrViewConfigurationType viewConfiguration)
  {
    auto types = getSupportedViewConfigurationTypes();
    if (std::find(types.begin(), types.end(), viewConfiguration) == types.end())
    {
      return false;
    }
    return true;
  }

  bool Instance::checkEnvironmentBlendModeSupported(XrViewConfigurationType viewConfiguration, XrEnvironmentBlendMode environmentBlendMode)
  {
    auto modes = getSupportedEnvironmentBlendModes(viewConfiguration);
    if (std::find(modes.begin(), modes.end(), environmentBlendMode) == modes.end())
    {
      return false;
    }
    return true;
  }

  void Instance::onEventInstanceLossPending([[maybe_unused]] const XrEventDataInstanceLossPending& event)
  {
    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrEventDataInstanceLossPending
    // TODO: This indicates the overall runtime is about to become unavailable. Given that encountering
    //       the subsequent XR_ERROR_RUNTIME_UNAVAILABLE when re-creating the instance isn't handled,
    //       just throw an exception.
    throw Exception({ "OpenXR: Instance loss pending" });
  }

  void Instance::createInstance()
  {
    if (_instance)
    {
      throw Exception({ "Instance already initialised" });
    }
    std::vector<const char*> extensions = {
        "XR_KHR_vulkan_enable"
    };

#ifdef ANDROID
    // Before any XR functions may be called we need to check if additional values are needed
    // for runtime intiailisation. See XR_KHR_loader_init.
    // Presence of this extension / requirement is denoted by a getProcAddress lookup.
    auto fn = (PFN_xrInitializeLoaderKHR)xr_pfn_noexcept(XR_NULL_HANDLE, "xrInitializeLoaderKHR");
    if (fn)
    {
      auto androidTraits = traits.cast<vsgvr::AndroidTraits>();
      if (androidTraits && androidTraits->vm && androidTraits->activity)
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
        throw Exception({ "On Android an AndroidTraits structure must be provided, with valid JNI/Activity pointers." });
      }
    }

    extensions.push_back("XR_KHR_android_create_instance");
#endif

    for (auto& e : traits->xrExtensions)
      extensions.push_back(e.c_str());

    std::vector<const char*> layers = {};
    for (auto& l : traits->xrLayers)
      layers.push_back(l.c_str());

    XrInstanceCreateInfo info;
    info.type = XR_TYPE_INSTANCE_CREATE_INFO;
    info.next = nullptr;
    info.createFlags = 0;
    info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
    info.enabledApiLayerNames = layers.empty() ? nullptr : layers.data();
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.enabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

    strncpy(info.applicationInfo.applicationName, traits->applicationName.c_str(), std::min(static_cast<int>(traits->applicationName.size() + 1), XR_MAX_APPLICATION_NAME_SIZE));
    info.applicationInfo.applicationVersion = traits->applicationVersion;
    strncpy(info.applicationInfo.engineName, traits->engineName.c_str(), std::min(static_cast<int>(traits->engineName.size() + 1), XR_MAX_ENGINE_NAME_SIZE));
    info.applicationInfo.apiVersion = traits->apiVersion;
    info.applicationInfo.engineVersion = traits->engineVersion;

#ifdef ANDROID
    auto androidTraits = traits.cast<vsgvr::AndroidTraits>();
    if (androidTraits && androidTraits->vm && androidTraits->activity)
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
      throw Exception({ "On Android an AndroidTraits structure must be provided, with valid JNI/Activity pointers." });
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
      throw Exception({ "Viewer: Instance not initialised" });
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
    info.formFactor = _formFactor;

    xr_check(xrGetSystem(_instance, &info, &_system), "Failed to get OpenXR system");

    _systemProperties = XrSystemProperties();
    _systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
    _systemProperties.next = nullptr;
    _systemProperties.graphicsProperties = { 0, 0, 0 };
    _systemProperties.trackingProperties = { XR_FALSE, XR_FALSE };
    xr_check(xrGetSystemProperties(_instance, _system, &_systemProperties), "Failed to get OpenXR system properties");
  }
}
