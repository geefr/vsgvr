
#include "game.h"

#include <vsgvr/app/CompositionLayerProjection.h>

#include "../../models/controller/controller.cpp"
#include "../../models/controller/controller2.cpp"
#include "../../models/world/world.cpp"
#include "../../models/assets/teleport_marker/teleport_marker.cpp"

#include "interactions/interaction_teleport.h"
#include "interactions/interaction_slide.h"

std::vector<std::string> Game::requiredInstanceExtensions()
{
  return {};
}

Game::Game(vsg::ref_ptr<vsgvr::Instance> xrInstance, vsg::ref_ptr<vsgvr::Viewer> vr, vsg::ref_ptr<vsg::Viewer> desktopViewer, bool displayDesktopWindow)
  : _xrInstance(xrInstance)
  , _vr(vr)
  , _desktopViewer(desktopViewer)
  , _desktopWindowEnabled(displayDesktopWindow)
{
  loadScene();
  initVR();
  initActions();
  _lastFrameTime = vsg::clock::now();
}

Game::~Game() {}

void Game::loadScene()
{
  // User / OpenXR root space
  _sceneRoot = vsg::Group::create();
  _controllerLeft = controller();
  _sceneRoot->addChild(_controllerLeft);
  _controllerRight = controller2();
  _sceneRoot->addChild(_controllerRight);

  _teleportMarker = vsg::Switch::create();
  auto teleportMatrix = vsg::MatrixTransform::create();
  _teleportMarker->addChild(false, teleportMatrix);
  teleportMatrix->addChild(teleport_marker());
  _sceneRoot->addChild(_teleportMarker);

  // User origin - The regular vsg scene / world space,
  // which the user origin may move around in
  _userOrigin = vsgvr::UserOrigin::create();
  _sceneRoot->addChild(_userOrigin);

  _ground = world(); // TODO: Not actually correct - buildings etc in the world should not be 'ground'
  _userOrigin->addChild(_ground);
}

void Game::initVR()
{
  // Create CommandGraphs to render the scene to the HMD
  auto headsetCompositionLayer = vsgvr::CompositionLayerProjection::create(_vr->referenceSpace);
  auto xrCommandGraphs = headsetCompositionLayer->createCommandGraphsForView(_vr->getInstance(), _vr->getSession(), _sceneRoot, _xrCameras, false);
  headsetCompositionLayer->assignRecordAndSubmitTask(xrCommandGraphs);
  headsetCompositionLayer->compile();
  _vr->compositionLayers.push_back(headsetCompositionLayer);

  if(_desktopWindowEnabled)
  {
    // Create a CommandGraph to render the desktop window
    auto lookAt = vsg::LookAt::create(vsg::dvec3(-4.0, -15.0, 25.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 0.0, 1.0));
    auto desktopWindow = _desktopViewer->windows().front();
    auto perspective = vsg::Perspective::create(30.0,
      static_cast<double>(desktopWindow->extent2D().width) / static_cast<double>(desktopWindow->extent2D().height)
      , 0.1, 100.0
    );
    _desktopCamera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(desktopWindow->extent2D()));
    auto desktopCommandGraph = vsg::createCommandGraphForView(desktopWindow, _desktopCamera, _sceneRoot, VK_SUBPASS_CONTENTS_INLINE, false);
    _desktopViewer->assignRecordAndSubmitTaskAndPresentation({ desktopCommandGraph });
    _desktopViewer->compile();
  }
}

void Game::initActions()
{
  // Tracking the location of the user's headset is achieved by tracking the VIEW reference space
  // vsgvr provides a SpaceBinding class for this - Similar to the ActionPoseBindings the head's pose
  // will be tracked during rendering, and available when performing interactions
  _headPose = vsgvr::SpaceBinding::create(vsgvr::ReferenceSpace::create(_vr->getSession(), XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_VIEW));
  _vr->spaceBindings.push_back(_headPose);

  // Input devices are tracked via ActionPoseBindings - Tracking elements from the OpenXR device tree in the session space,
  // along with binding the OpenXR input subsystem through to usable actions.
  _baseActionSet = vsgvr::ActionSet::create(_xrInstance, "controller_positions", "Controller Positions");
  // Pose bindings
  _leftHandPose = vsgvr::ActionPoseBinding::create(_xrInstance, _baseActionSet, "left_hand", "Left Hand");
  _rightHandPose = vsgvr::ActionPoseBinding::create(_xrInstance, _baseActionSet, "right_hand", "Right Hand");

  // Action to bind right trigger to switching between locomotion modes
  _switchInteractionAction = vsgvr::Action::create(_xrInstance, _baseActionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT, "switch_locomotion_mode", "Switch Locomotion Mode");

  _baseActionSet->actions = {
    _leftHandPose,
    _rightHandPose,
    _switchInteractionAction,
  };

  _interactions.emplace(InteractionMethod::Teleport, new Interaction_teleport(_xrInstance, _headPose, _leftHandPose, _teleportMarker, _ground));
  _interactions.emplace(InteractionMethod::Slide, new Interaction_slide(_xrInstance, _headPose, _leftHandPose, _ground));

  // Ask OpenXR to suggest interaction bindings.
  // * If subpaths are used, list all paths that each action should be bound for
  // * Note that this may only be called once for each interaction profile (but may be across multiple overlapping action sets)
  // * If a particular profile is used, all interactions should be bound to this i.e. If grabbing items only specifies bindings for
  //   an oculus controller, it will not be bound if the simple_controller is chosen by the runtime
  std::map<std::string, std::list<vsgvr::ActionSet::SuggestedInteractionBinding>> actionsToSuggest;
  actionsToSuggest["/interaction_profiles/khr/simple_controller"] = {
        {_leftHandPose, "/user/hand/left/input/aim/pose"},
        {_rightHandPose, "/user/hand/right/input/aim/pose"},
        {_switchInteractionAction, "/user/hand/right/input/select/click"},
  };
  actionsToSuggest["/interaction_profiles/oculus/touch_controller"] = {
      {_leftHandPose, "/user/hand/left/input/aim/pose"},
      {_rightHandPose, "/user/hand/right/input/aim/pose"},
      // A boolean action on a float input will be converted by the OpenXR runtime
      {_switchInteractionAction, "/user/hand/right/input/trigger/value"},
  };
  for(auto& interaction : _interactions )
  {
    for (auto& p : interaction.second->actionsToSuggest())
    {
      for( auto& x : p.second )
      {
        actionsToSuggest[p.first].push_back(x);
      }
    }
  }
  for (auto& p : actionsToSuggest)
  {
    if (! vsgvr::ActionSet::suggestInteractionBindings(_xrInstance, p.first, p.second))
    {
      throw std::runtime_error("Failed to configure interaction bindings for controllers");
    }
  }

  // All action sets the viewer should attach to the session
  _vr->actionSets.push_back(_baseActionSet);
  for( auto& interaction : _interactions )
  {
    _vr->actionSets.push_back(interaction.second->actionSet());
  }

  updateActiveInteraction(_currentInteractionMethod);

  // add close handler to respond the close window button and pressing escape
  _desktopViewer->addEventHandler(vsg::CloseHandler::create(_desktopViewer));
}

void Game::updateActiveInteraction(InteractionMethod method) {
  // The action sets which are currently active (will be synced each frame)
  _vr->activeActionSets.clear();
  _vr->activeActionSets.push_back(_baseActionSet);
  _vr->activeActionSets.push_back(_interactions[method]->actionSet());
}

void Game::frame()
{
  // OpenXR events must be checked first
  auto pol = _vr->pollEvents();
  if (pol == vsgvr::Viewer::PollEventsResult::Exit)
  {
    // User exited through VR overlay / XR runtime
    shouldExit = true;
    return;
  }

  if (pol == vsgvr::Viewer::PollEventsResult::NotRunning)
  {
    return;
  }
  else if (pol == vsgvr::Viewer::PollEventsResult::RuntimeIdle)
  {
    // Reduce power usage, wait for XR to wake
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }

  // Scene graph updates
  if (_leftHandPose->getTransformValid())
  {
    _controllerLeft->matrix = _leftHandPose->getTransform();
  }
  if (_rightHandPose->getTransformValid())
  {
    _controllerRight->matrix = _rightHandPose->getTransform();
  }

  // The session is running in some form, and a frame must be processed
  // The OpenXR frame loop takes priority - Acquire a frame to render into
  auto shouldQuit = false;

  if(_desktopWindowEnabled)
  {
    // Match the desktop camera to the HMD view
    _desktopCamera->viewMatrix = _xrCameras.front()->viewMatrix;
    _desktopCamera->projectionMatrix = _xrCameras.front()->projectionMatrix;

    // Desktop render
    // * The scene graph is updated by the desktop render
    // * if PollEventsResult::RunningDontRender the desktop render could be skipped
    if (_desktopViewer->advanceToNextFrame())
    {
      _desktopViewer->handleEvents();
      _desktopViewer->update();
      _desktopViewer->recordAndSubmit();
      _desktopViewer->present();
    }
    else
    {
      // Desktop window was closed
      shouldQuit = true;
      return;
    }
  }

  if (_vr->advanceToNextFrame())
  {
    if (pol == vsgvr::Viewer::PollEventsResult::RunningDontRender)
    {
      // XR Runtime requested that rendering is not performed (not visible to user)
      // While this happens frames must still be acquired and released however, in
      // order to synchronise with the OpenXR runtime
    }
    else
    {
      if (_switchInteractionAction->getStateValid())
      {
        auto state = _switchInteractionAction->getStateBool();
        if (state.isActive && state.changedSinceLastSync && state.currentState)
        {
          _currentInteractionMethod = static_cast<InteractionMethod>(_currentInteractionMethod + 1);
          if( _currentInteractionMethod == InteractionMethod::InteractionMethod_Max ) _currentInteractionMethod = static_cast<InteractionMethod>(InteractionMethod::InteractionMethod_Min + 1);
          updateActiveInteraction(_currentInteractionMethod);
        }
      }

      for (auto& interaction : _interactions)
      {
        if (std::find(_vr->activeActionSets.begin(), _vr->activeActionSets.end(),
          interaction.second->actionSet()) != _vr->activeActionSets.end())
        {
          auto deltaT = static_cast<double>(
            std::chrono::duration_cast<std::chrono::microseconds>(_vr->getFrameStamp()->time - _lastFrameTime).count()
          ) / 1e6;
          _lastFrameTime = _vr->getFrameStamp()->time;
          interaction.second->frame(_userOrigin, *this, deltaT);
        }
      }

      // Render to the HMD
      _vr->recordAndSubmit(); // Render XR frame
    }
  }

  // End the frame, and present to user
  // Frames must be explicitly released, even if the previous advanceToNextFrame returned false (PollEventsResult::RunningDontRender)
  _vr->releaseFrame();
}
