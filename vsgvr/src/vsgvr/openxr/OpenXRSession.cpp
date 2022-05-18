#include <vsgvr/openxr/OpenXRSession.h>

#include <iostream>

#include <vsg/core/Exception.h>
#include "OpenXRMacros.cpp"

namespace vsgvr {

  OpenXRSession::OpenXRSession(XrInstance instance, XrSystemId system, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding)
    : _graphicsBinding(graphicsBinding)
  {
    createSession(instance, system);
  }

  OpenXRSession::~OpenXRSession()
  {
    destroySession();
  }

  // TODO: A regular session update is needed - should handle the session lifecycle and such
  // void update(XrInstance instance, XrSystemId system);

  void OpenXRSession::createSession(XrInstance instance, XrSystemId system)
  {
    auto info = XrSessionCreateInfo();
    info.type = XR_TYPE_SESSION_CREATE_INFO;
    info.next = &_graphicsBinding->getBinding();
    info.systemId = system;

    xr_check(xrCreateSession(instance, &info, &_session), "Failed to create OpenXR session");
    _sessionState = XR_SESSION_STATE_IDLE;
  }
  void OpenXRSession::destroySession()
  {
    xr_check(xrDestroySession(_session));
    _session = nullptr;
  }

  void OpenXRSession::beginSession(XrViewConfigurationType viewConfigurationType)
  {
    if (_sessionRunning) return;
    auto info = XrSessionBeginInfo();
    info.type = XR_TYPE_SESSION_BEGIN_INFO;
    info.next = nullptr;
    info.primaryViewConfigurationType = viewConfigurationType;
    xr_check(xrBeginSession(_session, &info));
    _sessionRunning = true;
  }

  void OpenXRSession::endSession()
  {
    if (!_sessionRunning) return;
    xr_check(xrEndSession(_session));
    _sessionRunning = false;
  }

  void OpenXRSession::onEventStateChanged(const XrEventDataSessionStateChanged& event)
  {
    // TODO: Update _sessionState, handle the state change properly (Will be polled for in update?)
    //       Should the session trasition logic be here, or out in Instance? (Should Instance be renamed? It's more like a Viewer)
    _sessionState = event.state;
    std::cerr << "Session state changed: " << to_string(_sessionState) << std::endl;
  }

}