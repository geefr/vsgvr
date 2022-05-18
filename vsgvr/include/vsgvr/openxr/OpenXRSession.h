#pragma once

#include <vsg/core/Inherit.h>
#include <vsgvr/openxr/OpenXR.h>

#include <vsgvr/openxr/OpenXRGraphicsBindingVulkan2.h>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRSession : public vsg::Inherit<vsg::Object, OpenXRSession>
    {
        public:
            OpenXRSession() = delete;
            OpenXRSession(XrInstance instance, XrSystemId system, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding);
            ~OpenXRSession();

            // TODO: A regular session update is needed - should handle the session lifecycle and such
            // void update(XrInstance instance, XrSystemID system);

            XrSession getSession() const { return _session; }
            XrSessionState getSessionState() const { return _sessionState; }

            void onEventStateChanged(const XrEventDataSessionStateChanged& event);

            void beginSession();
            void endSession();

        private:
            void createSession(XrInstance instance, XrSystemId system);
            void destroySession();

            vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> _graphicsBinding;
            XrSession _session = nullptr;
            XrSessionState _sessionState = XR_SESSION_STATE_UNKNOWN;
    };
}
