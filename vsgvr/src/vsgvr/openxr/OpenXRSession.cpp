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
    // TODO, of course :)
    _session = nullptr;
}

void OpenXRSession::beginSession()
{

  // blabla this bit is tricky
  // You need to go read up on the session stuff, as the session includes the view and coordinate spaces etc.
  // Functions for begin and end are needed
  // The update function is after that
  // Ideally, calls through xr_check need to convert an impending session loss to an exception, which
  // then gets forwarded to a graceful Session::end call

  // TODO: Enumerate view configs, do this properly

  /*XrSessionBeginInfo beginInfo;
  beginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
  beginInfo.next = nullptr;
  beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
  xr_check(xrBeginSession(session, &beginInfo), "beginSession");*/
}

void OpenXRSession::endSession()
{

}

void OpenXRSession::onEventStateChanged(const XrEventDataSessionStateChanged& event)
{
    // TODO: Update _sessionState, handle the state change properly (Will be polled for in update?)
    //       Should the session trasition logic be here, or out in Instance? (Should Instance be renamed? It's more like a Viewer)
    _sessionState = event.state;
    std::cerr << "Session state changed: " << to_string(_sessionState) << std::endl;
}

}