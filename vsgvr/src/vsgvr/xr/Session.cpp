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

#include <vsgvr/xr/Session.h>

#include <vsg/core/Exception.h>

#include "Macros.cpp"

namespace vsgvr {

  Session::Session(vsg::ref_ptr<Instance> instance, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding)
    : _graphicsBinding(graphicsBinding)
  {
    createSession(instance);
  }

  Session::~Session()
  {
    destroySession();
  }

  void Session::createSession(vsg::ref_ptr<Instance> instance)
  {
    auto info = XrSessionCreateInfo();
    info.type = XR_TYPE_SESSION_CREATE_INFO;
    info.next = &_graphicsBinding->getBinding();
    info.systemId = instance->getSystem();

    xr_check(xrCreateSession(instance->getInstance(), &info, &_session), "Failed to create OpenXR session");
    _sessionState = XR_SESSION_STATE_IDLE;

    auto spaceCreateInfo = XrReferenceSpaceCreateInfo();
    spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    spaceCreateInfo.next = nullptr;
    // Session's pose within the natural reference space
    // In the case of STAGE, x-right, y-up, z-back
    spaceCreateInfo.poseInReferenceSpace.orientation = XrQuaternionf{ 0.0f, 0.0f, 0.0f, 1.0f };
    spaceCreateInfo.poseInReferenceSpace.position = XrVector3f{ 0.0f, 0.0f, 0.0f };
    // TODO: Should check what spaces are supported here
    //       STAGE is relative to the VR space bounds, but may not exist on standing-only or AR headsets
    spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;

    xr_check(xrCreateReferenceSpace(_session, &spaceCreateInfo, &_space), "Failed to create Session reference space");
  }

  void Session::destroySession()
  {
    xr_check(xrDestroySpace(_space));
    _space = XR_NULL_HANDLE;
    xr_check(xrDestroySession(_session));
    _session = XR_NULL_HANDLE;
  }

  void Session::beginSession(XrViewConfigurationType viewConfigurationType)
  {
    if (_sessionRunning) return;
    auto info = XrSessionBeginInfo();
    info.type = XR_TYPE_SESSION_BEGIN_INFO;
    info.next = nullptr;
    info.primaryViewConfigurationType = viewConfigurationType;
    xr_check(xrBeginSession(_session, &info), "Failed to begin session");
    _sessionRunning = true;
  }

  void Session::endSession()
  {
    if (!_sessionRunning) return;
    xr_check(xrEndSession(_session), "Failed to end session");
    _sessionRunning = false;
  }

  void Session::onEventStateChanged(const XrEventDataSessionStateChanged& event)
  {
    _sessionState = event.state;
  }
}

