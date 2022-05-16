#pragma once

/**
 * The global OpenXR context - The root wrapper object around OpenXR API.
 * 
 * Directly manages / owns:
 * - Instance - Exists until exit (context)
 * - System - Exists until exit (Selected headset and view configs)
 * - Session - Exists as long as it is valid. Recreated if not valid.
 *   -> getOrCreateSession()
 * - Graphics binding (vulkan_enable2 for now)
 *   - Initialised as part of System - graphics binding to the display
 *   - Partially accessed later by active session, or passed the views/
 *     spaces necessary for rendering
 * 
 * Public functions:
 * - Allocate - Likely with some preferences passed in
 * - Update - Root level call to perform an update pass
 *   - Any init required (instance, system, view config)
 *   - Return root-level actions to application - request for exit etc
 */

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

#include <vsgvr/openxr/OpenXR.h>
#include <vsgvr/openxr/OpenXRTraits.h>
#include <vsgvr/openxr/OpenXRGraphicsBindingVulkan2.h>
#include <vsgvr/openxr/OpenXRSession.h>

#include <string>
#include <vector>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRInstance : public vsg::Inherit<vsg::Object, OpenXRInstance>
    {
        public:
            OpenXRInstance() = delete;
            OpenXRInstance(OpenXrTraits xrTraits, OpenXrVulkanTraits vkTraits);
            ~OpenXRInstance();

            // TODO: Greater control over view configuration - At the moment traits
            // are specified, and if invalid init fails. Between system init and
            // session init there should be a point where view config can be chosen
            // such as choosing whether multisampling should be on, the size of
            // view images and such (based on _viewConfigurationViews)
            // TODO: Choices in traits should be a preference list?

        private:
            void createInstance();
            void destroyInstance();
            void getSystem();
            // TODO: Support/validation of xr layers - Debug/validation, view type extensions
            void validateTraits();
            void getViewConfiguration();

            OpenXrTraits _xrTraits;
            OpenXrVulkanTraits _vkTraits;

            XrInstance _instance = nullptr;
            XrInstanceProperties _instanceProperties;

            XrSystemId _system = 0;
            XrSystemProperties _systemProperties;

            // Details of chosen _xrTraits.viewConfigurationType
            // Details of individual views - recommended size / sampling
            XrViewConfigurationProperties _viewConfigurationProperties;
            std::vector<XrViewConfigurationView> _viewConfigurationViews;

            // TODO: Action sets

            // Graphics binding
            void createGraphicsBinding();
            void destroyGraphicsBinding();
            vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> _graphicsBinding;

            // Session
            void createSession();
            void destroySession();
            vsg::ref_ptr<OpenXRSession> _session;            
    };
}
