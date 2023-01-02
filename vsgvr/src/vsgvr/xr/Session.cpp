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

#include "Macros.cpp"

namespace vsgvr {

  Session::Session(vsg::ref_ptr<Instance> instance, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding)
    : _instance(instance)
    , _graphicsBinding(graphicsBinding)
  {
    createSession();
  }

  Session::~Session()
  {
    destroySession();
  }
  
  std::vector<XrReferenceSpaceType> Session::getSupportedReferenceSpaceTypes()
  {
    uint32_t count = 0;
    xr_check(xrEnumerateReferenceSpaces(_session, 0, &count, nullptr));
    std::vector<XrReferenceSpaceType> spaceTypes(count);
    xr_check(xrEnumerateReferenceSpaces(_session, static_cast<uint32_t>(spaceTypes.size()), &count, spaceTypes.data()));
    return spaceTypes;
  }

  std::vector<VkFormat> Session::getSupportedSwapchainFormats()
  {
    uint32_t count = 0;
    xr_check(xrEnumerateSwapchainFormats(_session, 0, &count, nullptr));
    std::vector<int64_t> formats(count);
    xr_check(xrEnumerateSwapchainFormats(_session, static_cast<uint32_t>(formats.size()), &count, formats.data()));

    std::vector<VkFormat> vkFormats;
    for( auto& format : formats ) vkFormats.emplace_back(static_cast<VkFormat>(format));
    return vkFormats;
  }

  XrViewConfigurationProperties Session::getViewConfigurationProperties()
  {
    auto viewConfigProperties = XrViewConfigurationProperties();
    viewConfigProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
    viewConfigProperties.next = nullptr;

    xr_check(xrGetViewConfigurationProperties(
      _instance->getInstance(),
      _instance->getSystem(),
      _instance->traits->viewConfigurationType,
      &viewConfigProperties)
    );
    return viewConfigProperties;
  }

  std::vector<XrViewConfigurationView> Session::getViewConfigurationViews()
  {
    uint32_t count = 0;
    std::vector<XrViewConfigurationView> viewConfigurationViews;
    xr_check(xrEnumerateViewConfigurationViews(
      _instance->getInstance(), 
      _instance->getSystem(), 
      _instance->traits->viewConfigurationType, 
      0, &count, nullptr
    ));
    viewConfigurationViews.resize(count, XrViewConfigurationView());
    for (auto& v : viewConfigurationViews) {
      v.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
      v.next = nullptr;
    }
    xr_check(xrEnumerateViewConfigurationViews(
      _instance->getInstance(),
      _instance->getSystem(),
      _instance->traits->viewConfigurationType,
      static_cast<uint32_t>(viewConfigurationViews.size()),
      &count,
      viewConfigurationViews.data())
    );
    return viewConfigurationViews;
  }

  bool Session::checkSwapchainFormatSupported(VkFormat format)
  {
    auto formats = getSupportedSwapchainFormats();
    if (std::find(formats.begin(), formats.end(), static_cast<int64_t>(format)) == formats.end())
    {
      return false;
    }
    return true;
  }

  bool Session::checkSwapchainSampleCountSupported(uint32_t numSamples)
  {
    if (numSamples == 0) return false;
    auto viewConfigurationViews = getViewConfigurationViews();
    for (auto i = 0; i < viewConfigurationViews.size(); ++i)
    {
      if (numSamples > viewConfigurationViews[i].maxSwapchainSampleCount)
      {
        return false;
      }
    }
    return true;
  }

  bool Session::checkReferenceSpaceTypeSupported(XrReferenceSpaceType referenceSpaceType)
  {
    auto supportedTypes = getSupportedReferenceSpaceTypes();
    if (std::find(supportedTypes.begin(), supportedTypes.end(), referenceSpaceType) == supportedTypes.end())
    {
      return false;
    }
    return true;
  }

  void Session::createSession()
  {
    auto info = XrSessionCreateInfo();
    info.type = XR_TYPE_SESSION_CREATE_INFO;
    info.next = &_graphicsBinding->getBinding();
    info.systemId = _instance->getSystem();

    xr_check(xrCreateSession(_instance->getInstance(), &info, &_session), "Failed to create OpenXR session");
    _sessionState = XR_SESSION_STATE_IDLE;
  }

  void Session::destroySession()
  {
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

