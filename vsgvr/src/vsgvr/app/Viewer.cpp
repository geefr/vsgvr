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

#include <vsgvr/app/Viewer.h>
#include <vsgvr/xr/ViewMatrix.h>
#include <vsgvr/xr/ProjectionMatrix.h>

#include <vsgvr/actions/ActionPoseBinding.h>
#include <vsgvr/actions/SpaceBinding.h>

#include <openxr/openxr_reflection.h>
#include "../xr/Macros.cpp"

#include <iostream>

#include <vsg/utils/ComputeBounds.h>
#include <vsg/app/CompileTraversal.h>
#include <vsg/app/View.h>
#include <vsg/ui/UIEvent.h>

namespace vsgvr
{
  Viewer::Viewer(vsg::ref_ptr<Instance> xrInstance, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding)
    : _instance(xrInstance)
  {
    createSession(graphicsBinding);
  }

  Viewer::~Viewer()
  {
    shutdownAll();
  }

  void Viewer::shutdownAll()
  {
    if (_session) destroySession();
  }

  auto Viewer::pollEvents() -> PollEventsResult
  {
    if (!_instance) return PollEventsResult::NotRunning;

    _eventHandler.pollEvents(_instance, _session);

    if (!_session) return PollEventsResult::NotRunning;

    if( _firstUpdate )
    {
      createActionSpacesAndAttachActionSets();
      _firstUpdate = false;
    }

    switch (_session->getSessionState())
    {
    case XR_SESSION_STATE_IDLE:
      return PollEventsResult::RuntimeIdle;
    case XR_SESSION_STATE_READY:
      if (!_session->getSessionRunning())
      {
        // Begin session. Transition to synchronised after a few begin/end frames
        _session->beginSession(_instance->traits->viewConfigurationType);
      }
      return PollEventsResult::RunningDontRender;
    case XR_SESSION_STATE_SYNCHRONIZED:
      return PollEventsResult::RunningDontRender;
    case XR_SESSION_STATE_VISIBLE:
    case XR_SESSION_STATE_FOCUSED:
      syncSpaceBindings();
      syncActions();
      return PollEventsResult::RunningDoRender;
    case XR_SESSION_STATE_STOPPING:
      _session->endSession();
      return PollEventsResult::NotRunning;
    case XR_SESSION_STATE_LOSS_PENDING:
      std::cerr << "State Loss" << std::endl;
      // TODO: Display connection lost. Re-init may be possible later
      [[fallthrough]];
    case XR_SESSION_STATE_EXITING:
      if (_session->getSessionRunning())
      {
        _session->endSession();
      }
      shutdownAll();
      return PollEventsResult::Exit;
    case XR_SESSION_STATE_UNKNOWN:
    default:
      break;
    }
    return PollEventsResult::RunningDontRender;
  }

#if VSG_VERSION_MAJOR >= 1 && VSG_VERSION_MINOR >= 1
  bool Viewer::advanceToNextFrame(double simulationTime)
#else
  bool Viewer::advanceToNextFrame()
#endif
  {
    // Viewer::acquireNextFrame
    _frameState = XrFrameState();
    _frameState.type = XR_TYPE_FRAME_STATE;
    _frameState.next = nullptr;

    // TODO: Return statuses - session/instance loss fairly likely here
    xr_check(xrWaitFrame(_session->getSession(), nullptr, &_frameState));
    xr_check(xrBeginFrame(_session->getSession(), nullptr));

    // Viewer::advanceToNextFrame
    // create FrameStamp for frame
    // TODO: originally vsg::clock::now()
    // Should use XR extensions below to convert to wall-time
    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrTime
    // XR_KHR_win32_convert_performance_counter_time or XR_KHR_convert_timespec_time.
    vsg::clock::time_point t(std::chrono::nanoseconds(_frameState.predictedDisplayTime));
    if (!_frameStamp)
    {
      // first frame, initialize to frame count and indices to 0
#if VSG_VERSION_MAJOR >= 1 && VSG_VERSION_MINOR >= 1

      _start_time_point = t;

      if (simulationTime == UseTimeSinceStartPoint) simulationTime = 0.0;
      _frameStamp = vsg::FrameStamp::create(t, 0, simulationTime);
#else
      _frameStamp = vsg::FrameStamp::create(t, 0);
#endif
    }
    else
    {
      // after first frame so increment frame count and indices
#if VSG_VERSION_MAJOR >= 1 && VSG_VERSION_MINOR >= 1
      if (simulationTime == UseTimeSinceStartPoint)
      {
          simulationTime = std::chrono::duration<double, std::chrono::seconds::period>(t - _start_time_point).count();
      }
      _frameStamp = vsg::FrameStamp::create(t, _frameStamp->frameCount + 1, simulationTime);
#else
      _frameStamp = vsg::FrameStamp::create(t, _frameStamp->frameCount + 1);
#endif
    }

    for (auto& layer : compositionLayers)
    {
      layer->advanceToNextFrame();
    }

    // create an event for the new frame.
    // _events.emplace_back(new FrameEvent(_frameStamp));

    // Inform the application whether it should actually render
    // The frame must be completed regardless to remain synchronised with OpenXR
    return static_cast<bool>(_frameState.shouldRender);
  }

  void Viewer::recordAndSubmit()
  {
    for( auto& layer : compositionLayers )
    {
      layer->render(_instance, _session, _frameState, _frameStamp);
      // Add the composition layer for the frame end info
      _layers.push_back(layer->getCompositionLayerBaseHeaderPtr());
    }
  }

  void Viewer::releaseFrame()
  {
    auto info = XrFrameEndInfo();
    info.type = XR_TYPE_FRAME_END_INFO;
    info.next = nullptr;
    info.displayTime = _frameState.predictedDisplayTime;
    info.environmentBlendMode = _instance->traits->environmentBlendMode;
    info.layerCount = static_cast<uint32_t>(_layers.size());
    info.layers = _layers.data();

    xr_check(xrEndFrame(_session->getSession(), &info));

    _layers.clear();
  }

  void Viewer::syncSpaceBindings()
  {
    // Update each of the space bindings
    if( spaceBindings.empty() ) return;
    if( !referenceSpace ) return;

    for (auto& space : spaceBindings)
    {
      auto spaceLocation = space->getSpace()->locate(referenceSpace->getSpace(), _frameState.predictedDisplayTime);
      space->setTransform(spaceLocation);
    }
  }

  void Viewer::syncActions()
  {
    if( activeActionSets.empty() ) return;
    if( !referenceSpace) return;

    // Sync the active action sets
    auto info = XrActionsSyncInfo();
    info.type = XR_TYPE_ACTIONS_SYNC_INFO;
    info.next = nullptr;
    std::vector<XrActiveActionSet> d;
    for( auto& actionSet : activeActionSets ) d.push_back({actionSet->getActionSet(), XR_NULL_PATH});
    info.countActiveActionSets = static_cast<uint32_t>(d.size());
    info.activeActionSets = d.data();
    xr_check(xrSyncActions(_session->getSession(), &info));

    // Extract action and pose information
    for (auto& actionSet : activeActionSets)
    {
      for (auto& action : actionSet->actions)
      {
        if (auto a = action.cast<ActionPoseBinding>())
        {
          auto location = XrSpaceLocation();
          location.type = XR_TYPE_SPACE_LOCATION;
          location.next = nullptr;
          xr_check(xrLocateSpace(a->getActionSpace(), referenceSpace->getSpace(), _frameState.predictedDisplayTime, &location));
          a->setTransform(location);
        }

        auto subPaths = action->getSubPaths();
        if( subPaths.empty() ) {
          action->syncInputState(_instance, _session);
        } else {
          for( auto& p : subPaths ) action->syncInputState(_instance, _session, p);
        }
      }
    }
  }

  void Viewer::createActionSpacesAndAttachActionSets()
  {
    if( !_attachedActionSets.empty() ) throw Exception({"Action spaces have already been attached"});
    // Attach action sets to the session
    if( !actionSets.empty() )
    {
      for( auto& actionSet : actionSets ) _attachedActionSets.push_back(actionSet->getActionSet());

      auto info = XrSessionActionSetsAttachInfo();
      info.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
      info.next = nullptr;
      info.countActionSets = static_cast<uint32_t>(_attachedActionSets.size());
      info.actionSets = _attachedActionSets.data();
      xr_check(xrAttachSessionActionSets(_session->getSession(), &info), "Failed to attach action sets to session");

      for( auto& actionSet : actionSets )
      {
        for( auto& action : actionSet->actions )
        {
          if( auto a = action.cast<ActionPoseBinding>() )
          {
            a->createActionSpace(_session);
          }
        }
      }
    }
  }

  void Viewer::destroyActionSpaces()
  {
    for( auto& actionSet : actionSets )
    {
      for( auto& action : actionSet->actions )
      {
        if( auto a = action.cast<ActionPoseBinding>() )
        {
          if( a->getActionSpace() ) a->destroyActionSpace();
        }
      }
    }
  }

  void Viewer::createSession(vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> graphicsBinding) {
    if (_session) {
      throw Exception({ "Viewer: Session already initialised" });
    }

    _session = Session::create(_instance, graphicsBinding);
  }

  void Viewer::destroySession() {
    if (!_session) {
      throw Exception({ "Viewer: Session not initialised" });
    }
    destroyActionSpaces();
    _session = 0;
  }
}

