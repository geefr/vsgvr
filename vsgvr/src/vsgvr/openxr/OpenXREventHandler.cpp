#include <vsgvr/openxr/OpenXREventHandler.h>

#include <vsgvr/openxr/OpenXRInstance.h>
#include <vsgvr/openxr/OpenXRSession.h>

#include <vsg/core/Exception.h>

#include "OpenXRMacros.cpp"

using namespace vsg;

namespace vsgvr {

  OpenXREventHandler::OpenXREventHandler() {}

  OpenXREventHandler::~OpenXREventHandler() {}

  void OpenXREventHandler::pollEvents(OpenXRInstance* instance, OpenXRSession* session)
  {
    auto toProcess = 20;
    while (toProcess-- > 0)
    {
      auto event = XrEventDataBuffer();
      event.type = XR_TYPE_EVENT_DATA_BUFFER;
      event.next = nullptr;
      auto result = xrPollEvent(instance->getInstance(), &event);

      if (result == XR_EVENT_UNAVAILABLE)
      {
        // Done
        break;
      }
      // TODO: Gracefully handle XR_ERROR_INSTANCE_LOST?
      xr_check(result, "xrPollEvent");

      // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#events
      switch (event.type)
      {
      case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
        if (instance) instance->onEventInstanceLossPending(*reinterpret_cast<XrEventDataInstanceLossPending*>(&event));
        break;
      case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
        // TODO
        break;
      case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
        // TODO
        break;
      case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
        // TODO: event.session - Are multiple sessions possible? Certainly not yet in vsgvr
        if (session) session->onEventStateChanged(*reinterpret_cast<XrEventDataSessionStateChanged*>(&event));
        break;
      case XR_TYPE_EVENT_DATA_EVENTS_LOST:
        // TODO: Sensible logging? Is this possible if the queue is polled regularly?
        // throw Exception({"OpenXR event queue overflow, events lost"});
        break;
      default:
        break;
      }

    }
  }

}