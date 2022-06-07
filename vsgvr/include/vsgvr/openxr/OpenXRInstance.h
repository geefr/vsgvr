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
#include <vsgvr/openxr/OpenXREventHandler.h>
#include <vsgvr/openxr/OpenXRGraphicsBindingVulkan2.h>
#include <vsgvr/openxr/OpenXRSession.h>

#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/ui/FrameStamp.h>

#include <string>
#include <vector>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRInstance : public vsg::Inherit<vsg::Object, OpenXRInstance>
    {
        public:
            OpenXRInstance() = delete;
            OpenXRInstance(OpenXrTraits xrTraits, OpenXrVulkanTraits vkTraits);
            ~OpenXRInstance();

            XrInstance getInstance() const { return _instance; }

            // TODO: Greater control over view configuration - At the moment traits
            // are specified, and if invalid init fails. Between system init and
            // session init there should be a point where view config can be chosen
            // such as choosing whether multisampling should be on, the size of
            // view images and such (based on _viewConfigurationViews)
            // TODO: Choices in traits should be a preference list?
            
            void onEventInstanceLossPending(const XrEventDataInstanceLossPending& event);

            // TODO: Update this - Summary level of what to do, based on the event updates.
            //       Simple things don't need any input from app, so it might just reduce
            //       to a boolean good/exit status.
            enum class PollEventsResult {
                Success,
                RuntimeIdle,
                NotRunning,
                RunningDontRender,
                RunningDoRender,
                Exit,
            };
            /// Must be called regularly, within the render loop
            /// The application must handle the returned values accordingly,
            /// to avoid exceptions being thrown on subsequence calls
            auto pollEvents() -> PollEventsResult;

            bool advanceToNextFrame();
            void releaseFrame();

            // TODO: The 'Viewer' implementation, avoid this duplication
            vsg::ref_ptr<vsg::Camera> createCameraForScene(vsg::ref_ptr<vsg::Node> scene, const VkExtent2D& extent);
            std::vector<vsg::ref_ptr<vsg::CommandGraph>> createCommandGraphsForView(vsg::ref_ptr<vsg::Node> vsg_scene, bool assignHeadlight = true);
            void assignRecordAndSubmitTaskAndPresentation(std::vector<vsg::ref_ptr<vsg::CommandGraph>> in_commandGraphs);
            // Manage the work to do each frame using RecordAndSubmitTasks.
            using RecordAndSubmitTasks = std::vector<vsg::ref_ptr<vsg::RecordAndSubmitTask>>;
            RecordAndSubmitTasks recordAndSubmitTasks;
            void compile(vsg::ref_ptr<vsg::ResourceHints> hints = {});
            void update();
            void recordAndSubmit();

        private:
            void shutdownAll();
            void createInstance();
            void destroyInstance();
            void getSystem();
            // TODO: Support/validation of xr layers - Debug/validation, view type extensions
            void validateTraits();
            void getViewConfiguration();

            OpenXrTraits _xrTraits;
            OpenXrVulkanTraits _vkTraits;

            OpenXREventHandler _eventHandler;

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

            // Per-frame
            XrFrameState _frameState;
            vsg::ref_ptr<vsg::FrameStamp> _frameStamp;

            std::vector<XrCompositionLayerBaseHeader*> _layers;
            XrCompositionLayerProjection _layerProjection;
            std::vector<XrCompositionLayerProjectionView> _layerProjectionViews;
    };
}
