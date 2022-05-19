#include <vsgvr/openxr/OpenXRSession.h>

#include <iostream>

#include <vsg/core/Exception.h>
#include "OpenXRMacros.cpp"

using namespace vsg;

namespace vsgvr {

  OpenXRSession::OpenXRSession(XrInstance instance, XrSystemId system, vsg::ref_ptr<OpenXRGraphicsBindingVulkan2> graphicsBinding,
                               VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs)
    : _graphicsBinding(graphicsBinding)
  {
    createSession(instance, system);
    createSwapchain(swapchainFormat, viewConfigs);
  }

  OpenXRSession::~OpenXRSession()
  {
    destroySwapchain();
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

  void OpenXRSession::createSwapchain(VkFormat swapchainFormat, std::vector<XrViewConfigurationView> viewConfigs)
  {
    if( _swapchain ) throw Exception({"Swapchain already initialised"});
    if( !_session ) throw Exception({"Unable to create swapchain without session"});
    _swapchain = OpenXRSwapchain::create(_session, swapchainFormat, viewConfigs);
  }

  void OpenXRSession::destroySwapchain()
  {
    _swapchain = nullptr;
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
    xr_check(xrBeginSession(_session, &info), "Failed to begin session");
    _sessionRunning = true;
  }

  void OpenXRSession::endSession()
  {
    if (!_sessionRunning) return;
    xr_check(xrEndSession(_session), "Failed to end session");
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