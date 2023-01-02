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

#include <vsgvr/xr/EventHandler.h>

#include <vsgvr/app/Viewer.h>
#include <vsgvr/xr/Session.h>

#include "Macros.cpp"

using namespace vsg;

namespace vsgvr {

  EventHandler::EventHandler() {}

  EventHandler::~EventHandler() {}

  void EventHandler::pollEvents(Instance* instance, Session* session)
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

