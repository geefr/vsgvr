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
#include <vsgvr/xr/Common.h>

#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>
#include <vsgvr/xr/Swapchain.h>

#include <vsg/vk/Framebuffer.h>

namespace vsgvr {
    class VSGVR_DECLSPEC Session : public vsg::Inherit<vsg::Object, Session>
    {
        public:
            Session() = delete;
            Session(vsg::ref_ptr<Instance> instance, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding);

            XrSession getSession() const { return _session; }
            XrSessionState getSessionState() const { return _sessionState; }
            bool getSessionRunning() const { return _sessionRunning; }

            void onEventStateChanged(const XrEventDataSessionStateChanged& event);

            void beginSession(XrViewConfigurationType viewConfigurationType);
            void endSession();

            /// The session's reference space
            XrSpace getSpace() const { return _space; }

            vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> getGraphicsBinding() { return _graphicsBinding; }

        protected:
            virtual ~Session();
        private:
            void createSession(vsg::ref_ptr<Instance> instance);
            void destroySession();

            vsg::ref_ptr<GraphicsBindingVulkan> _graphicsBinding;
            
            XrSession _session = XR_NULL_HANDLE;
            XrSessionState _sessionState = XrSessionState::XR_SESSION_STATE_UNKNOWN;
            bool _sessionRunning = false;
            XrSpace _space = XR_NULL_HANDLE;
    };
}


EVSG_type_name(vsgvr::Session);
