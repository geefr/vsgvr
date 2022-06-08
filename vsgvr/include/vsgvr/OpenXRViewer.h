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
#include <vsg/core/ref_ptr.h>

#include <vsgvr/OpenXRCommon.h>
#include <vsgvr/OpenXRInstance.h>
#include <vsgvr/OpenXRTraits.h>
#include <vsgvr/OpenXREventHandler.h>
#include <vsgvr/OpenXRGraphicsBindingVulkan.h>
#include <vsgvr/OpenXRSession.h>

#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/ui/FrameStamp.h>

#include <string>
#include <vector>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRViewer : public vsg::Inherit<vsg::Object, OpenXRViewer>
    {
        public:
            OpenXRViewer() = delete;
            OpenXRViewer(vsg::ref_ptr<OpenXRInstance> xrInstance, OpenXrTraits xrTraits, vsg::ref_ptr<OpenXRGraphicsBindingVulkan> graphicsBinding);
            ~OpenXRViewer();

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

            // OpenXR action sets, which will be managed by the viewer / session
            std::vector<vsg::ref_ptr<OpenXRActionSet>> actionSets;
            // Active actions sets, which will be synchronised each update
            // One or more may be active at a time, depending on user interaction mode
            std::vector<vsg::ref_ptr<OpenXRActionSet>> activeActionSets;

        private:
            void shutdownAll();
            void getViewConfiguration();

            void syncActions();
            void createActionSpaces();
            void destroyActionSpaces();

            OpenXrTraits _xrTraits;

            OpenXREventHandler _eventHandler;

            vsg::ref_ptr<OpenXRInstance> _instance;

            // Details of chosen _xrTraits.viewConfigurationType
            // Details of individual views - recommended size / sampling
            XrViewConfigurationProperties _viewConfigurationProperties;
            std::vector<XrViewConfigurationView> _viewConfigurationViews;

            // Graphics binding
            vsg::ref_ptr<OpenXRGraphicsBindingVulkan> _graphicsBinding;

            // Session
            void createSession();
            void destroySession();
            vsg::ref_ptr<OpenXRSession> _session;

            bool _firstUpdate = true;
            std::vector<XrActionSet> _attachedActionSets;

            // Per-frame
            XrFrameState _frameState;
            vsg::ref_ptr<vsg::FrameStamp> _frameStamp;

            std::vector<XrCompositionLayerBaseHeader*> _layers;
            XrCompositionLayerProjection _layerProjection;
            std::vector<XrCompositionLayerProjectionView> _layerProjectionViews;
    };
}
