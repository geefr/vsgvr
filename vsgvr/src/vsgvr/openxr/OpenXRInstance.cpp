
#include <vsgvr/openxr/OpenXRInstance.h>
#include <vsg/core/Exception.h>

#include <openxr/openxr_reflection.h>
#include "OpenXRMacros.cpp"

#include <iostream>

using namespace vsg;

namespace vsgvr
{
    OpenXRInstance::OpenXRInstance(OpenXrTraits xrTraits, OpenXrVulkanTraits vkTraits)
    {
        createInstance();
        getSystem();
        validateTraits();
        getViewConfiguration();
        createGraphicsBinding();
        createSession();
    }

    OpenXRInstance::~OpenXRInstance()
    {
        if( _session ) destroySession();

        destroyGraphicsBinding();
        _system = 0;
        _systemProperties = XrSystemProperties();
        destroyInstance();
    }

    auto OpenXRInstance::pollEvents() -> PollEventsResult
    {
        if( !_instance ) return PollEventsResult::NotReady;

        _eventHandler.pollEvents(this, _session);

        if( !_session ) return PollEventsResult::NotReady;

        switch(_session->getSessionState())
        {
            case XR_SESSION_STATE_IDLE:
                return PollEventsResult::RuntimeIdle;
            case XR_SESSION_STATE_READY:
                std::cerr << "Beginning Session" << std::endl;
                _session->beginSession();
                return PollEventsResult::NotReady;
            case XR_SESSION_STATE_SYNCHRONIZED:
                return PollEventsResult::ReadyDontRender;
            case XR_SESSION_STATE_VISIBLE:
            case XR_SESSION_STATE_FOCUSED:
                return PollEventsResult::ReadyRender;
            case XR_SESSION_STATE_STOPPING:
                std::cerr << "Ending Session" << std::endl;
                _session->endSession();
                return PollEventsResult::NotReady;
            case XR_SESSION_STATE_LOSS_PENDING:
                std::cerr << "State Loss" << std::endl;
                // TODO: Display connection lost. Re-init may be possible later
            case XR_SESSION_STATE_EXITING:
                std::cerr << "Exit" << std::endl;
                _session->endSession();
                return PollEventsResult::Exit;
            case XR_SESSION_STATE_UNKNOWN:
            default:
                break;
        }
    }

    void OpenXRInstance::acquireFrame()
    {

    }

    void OpenXRInstance::releaseFrame()
    {
        
    }

    void OpenXRInstance::onEventInstanceLossPending(const XrEventDataInstanceLossPending& event)
    {
      // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrEventDataInstanceLossPending
      // TODO: This indicates the overall runtime is about to become unavailable. Given that encountering
      //       the subsequent XR_ERROR_RUNTIME_UNAVAILABLE when re-creating the instance isn't handled, 
      //       just throw an exception.
      throw Exception({"OpenXR: Instance loss pending"});
    }

    void OpenXRInstance::createInstance()
    {
        if (_instance)
        {
            throw Exception({"OpenXRInstance: Instance already initialised"});
        }
        std::vector<const char *> extensions = {
            "XR_KHR_vulkan_enable",
            "XR_KHR_vulkan_enable2"};
        for (auto &e : _xrTraits.xrExtensions)
            extensions.push_back(e.c_str());

        std::vector<const char *> layers = {};
        for (auto &l : _xrTraits.xrLayers)
            layers.push_back(l.c_str());

        XrInstanceCreateInfo info;
        info.type = XR_TYPE_INSTANCE_CREATE_INFO;
        info.next = nullptr;
        info.createFlags = 0;
        info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
        info.enabledApiLayerNames = layers.empty() ? nullptr : layers.data();
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.enabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

        strncpy(info.applicationInfo.applicationName, _xrTraits.applicationName.c_str(), std::min(static_cast<int>(_xrTraits.applicationName.size() + 1), XR_MAX_APPLICATION_NAME_SIZE));
        info.applicationInfo.applicationVersion = _xrTraits.applicationVersion;
        strncpy(info.applicationInfo.engineName, _xrTraits.engineName.c_str(), std::min(static_cast<int>(_xrTraits.engineName.size() + 1), XR_MAX_ENGINE_NAME_SIZE));
        info.applicationInfo.apiVersion = _xrTraits.apiVersion;
        info.applicationInfo.engineVersion = _xrTraits.engineVersion;

        xr_check(xrCreateInstance(&info, &_instance), "Failed to create XR Instance");
        xr_check(xrGetInstanceProperties(_instance, &_instanceProperties), "Failed to get XR Instance properties");
    }
    void OpenXRInstance::destroyInstance()
    {
        if (!_instance)
        {
            throw Exception({"OpenXRInstance: Instance not initialised"});
        }
        xr_check(xrDestroyInstance(_instance), "Failed to destroy XR Instance");
        _instance = nullptr;
    }

    void OpenXRInstance::getSystem()
    {
        XrSystemGetInfo info;
        info.type = XR_TYPE_SYSTEM_GET_INFO;
        info.next = nullptr;
        info.formFactor = _xrTraits.formFactor;

        XrSystemId system;
        xr_check(xrGetSystem(_instance, &info, &_system), "Failed to get OpenXR system");

        XrSystemProperties props;
        props.type = XR_TYPE_SYSTEM_PROPERTIES;
        props.next = nullptr;
        props.graphicsProperties = {0};
        props.trackingProperties = {0};
        xr_check(xrGetSystemProperties(_instance, _system, &props), "Failed to get OpenXR system properties");
    }
    void OpenXRInstance::validateTraits()
    {
        // Form factor validated by getSystem

        // View configuration type
        {
            std::vector<XrViewConfigurationType> types;
            uint32_t count = 0;
            xr_check(xrEnumerateViewConfigurations(_instance, _system, 0, &count, nullptr));
            types.resize(count);
            xr_check(xrEnumerateViewConfigurations(_instance, _system, types.size(), &count, types.data()));
            if( std::find(types.begin(), types.end(), _xrTraits.viewConfigurationType) == types.end() )
            {
                throw Exception({"View configuration type not supported"});
            }
        }

        // Environment blend mode
        {
            std::vector<XrEnvironmentBlendMode> modes;
            uint32_t count = 0;
            xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, _xrTraits.viewConfigurationType, 0, &count, nullptr));
            modes.resize(count);
            xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, _xrTraits.viewConfigurationType, modes.size(), &count, modes.data()));
            if( std::find(modes.begin(), modes.end(), _xrTraits.environmentBlendMode) == modes.end() )
            {
                throw Exception({"Environment blend mode not supported"});
            }
        }
    }
    void OpenXRInstance::getViewConfiguration() {
        xr_check(xrGetViewConfigurationProperties(_instance, _system, _xrTraits.viewConfigurationType, &_viewConfigurationProperties));
        uint32_t count = 0;
        xr_check(xrEnumerateViewConfigurationViews(_instance, _system, _xrTraits.viewConfigurationType, 0, &count, nullptr));
        _viewConfigurationViews.resize(count);
        xr_check(xrEnumerateViewConfigurationViews(_instance, _system, _xrTraits.viewConfigurationType, _viewConfigurationViews.size(), &count, _viewConfigurationViews.data()));
    }

    void OpenXRInstance::createGraphicsBinding() {
        if( _graphicsBinding ) {
            throw Exception({"openXRInstance: Graphics binding already initialised"});
        }

        _graphicsBinding = OpenXRGraphicsBindingVulkan2::create(_instance, _system, _xrTraits, _vkTraits);
    }

    void OpenXRInstance::destroyGraphicsBinding() {
        if( !_graphicsBinding ) {
            throw Exception({"openXRInstance: Graphics binding not initialised"});
        }

        _graphicsBinding = 0;
    }

    void OpenXRInstance::createSession() {
        if( _session ) {
            throw Exception({"openXRInstance: Session already initialised"});
        }
        
        _session = OpenXRSession::create(_instance, _system, _graphicsBinding);
    }

    void OpenXRInstance::destroySession() {
        if( !_session ) {
            throw Exception({"openXRInstance: Session not initialised"});
        }

        _session = 0;
    }
}
