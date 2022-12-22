
#include "game.h"

#include "../../models/controller/controller.cpp"
#include "../../models/controller/controller2.cpp"
#include "../../models/scenes/world_1/world_1.cpp"
#include "../../models/assets/teleport_marker/teleport_marker.cpp"

// #include "interactions/interaction_example.h"
#include "interactions/interaction_teleport.h"

Game::Game(vsg::ref_ptr<vsgvr::OpenXRInstance> xrInstance, vsg::ref_ptr<vsgvr::OpenXRViewer> vr, vsg::ref_ptr<vsg::Viewer> desktopViewer)
  : _xrInstance(xrInstance)
  , _vr(vr)
  , _desktopViewer(desktopViewer)
{
  loadScene();
  initVR();
  initActions();
}

Game::~Game()
{

}

void Game::loadScene()
{
  _sceneRoot = vsg::Group::create();

  _controllerLeft = controller();
  _sceneRoot->addChild(_controllerLeft);
  _controllerRight = controller2();
  _sceneRoot->addChild(_controllerRight);

  _teleportMarker = vsg::Switch::create();
  _teleportMarker->addChild(false, teleport_marker());
  _sceneRoot->addChild(_teleportMarker);

  _scene = vsg::MatrixTransform::create();
  _ground = world_1(); // TODO: Not actually correct - buildings etc in the world should not be 'ground'
  _scene->addChild(_ground);
  _sceneRoot->addChild(_scene);
}

void Game::initVR()
{
  // Create CommandGraphs to render the scene to the HMD
  // TODO: This only really exists because vsg::createCommandGraphForView requires
  // a Window instance. Other than some possible improvements later, it could use the same code as vsg
  // OpenXR rendering may use one or more command graphs, as decided by the viewer
  // (TODO: At the moment only a single CommandGraph will be used, even if there's multiple XR views)
  auto xrCommandGraphs = _vr->createCommandGraphsForView(_sceneRoot, _xrCameras, false);
  // TODO: This is almost identical to Viewer::assignRecordAndSubmitTaskAndPresentation - The only difference is
  // that OpenXRViewer doesn't have presentation - If presentation was abstracted we could avoid awkward duplication here
  _vr->assignRecordAndSubmitTask(xrCommandGraphs);
  // TODO: This is identical to Viewer::compile, except CompileManager requires a child class of Viewer
  // OpenXRViewer can't be a child class of Viewer yet (Think this was due to the assumption that a Window/Viewer has presentation / A Surface)
  _vr->compile();

  // Create a CommandGraph to render the desktop window
  // TODO: I tried to share one of the HMD's cameras here, but under steamvr that caused a deadlock when rendering
  //       Instead, create a standard perspective camera and bind it to the hmd's position - This looks better on the desktop window anyway

  // set up the camera
  auto lookAt = vsg::LookAt::create(vsg::dvec3(-4.0, -15.0, 25.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 0.0, 1.0));
  auto desktopWindow = _desktopViewer->windows().front();
  auto perspective = vsg::Perspective::create(30.0,
    static_cast<double>(desktopWindow->extent2D().width) / static_cast<double>(desktopWindow->extent2D().height)
    , 0.1, 100.0
  );
  _desktopCamera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(desktopWindow->extent2D()));
  auto desktopCommandGraph = vsg::createCommandGraphForView(desktopWindow, _desktopCamera, _sceneRoot);
  _desktopViewer->assignRecordAndSubmitTaskAndPresentation({ desktopCommandGraph });
  _desktopViewer->compile();
}

void Game::initActions()
{
  // Configure OpenXR action sets and pose bindings - These allow elements of the OpenXR device tree to be located and tracked in space,
  // along with binding the OpenXR input subsystem through to usable actions.
  _baseActionSet = vsgvr::OpenXRActionSet::create(_xrInstance, "controller_positions", "Controller Positions");
  // Pose bindings - One for each hand
  _leftHandPose = vsgvr::OpenXRActionPoseBinding::create(_xrInstance, _baseActionSet, "left_hand", "Left Hand");
  _rightHandPose = vsgvr::OpenXRActionPoseBinding::create(_xrInstance, _baseActionSet, "right_hand", "Right Hand");
  _baseActionSet->actions = {
    _leftHandPose,
    _rightHandPose,
  };

  // _interactions.emplace("example", new Interaction_example(_xrInstance, _controllerLeft, _controllerRight));
  _interactions.emplace("teleport", new Interaction_teleport(_xrInstance, _leftHandPose, _teleportMarker, _ground));

  // Ask OpenXR to suggest interaction bindings.
  // * If subpaths are used, list all paths that each action should be bound for
  // * Note that this may only be called once for each interaction profile (but may be across multiple overlapping action sets)
  // * If a particular profile is used, all interactions should be bound to this i.e. If grabbing items only specifies bindings for
  //   an oculus controller, it will not be bound if the simple_controller is chosen by the runtime
  std::map<std::string, std::list<vsgvr::OpenXRActionSet::SuggestedInteractionBinding>> actionsToSuggest;
  actionsToSuggest["/interaction_profiles/khr/simple_controller"] = {
        {_leftHandPose, "/user/hand/left/input/aim/pose"},
        {_rightHandPose, "/user/hand/right/input/aim/pose"},
  };
  actionsToSuggest["/interaction_profiles/oculus/touch_controller"] = {
      {_leftHandPose, "/user/hand/left/input/aim/pose"},
      {_rightHandPose, "/user/hand/right/input/aim/pose"},
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
    if (! vsgvr::OpenXRActionSet::suggestInteractionBindings(_xrInstance, p.first, p.second))
    {
      throw vsg::Exception({ "Failed to configure interaction bindings for controllers" });
    }
  }

  // All action sets the viewer should attach to the session
  _vr->actionSets.push_back(_baseActionSet);
  for( auto& interaction : _interactions )
  {
    _vr->actionSets.push_back(interaction.second->actionSet());
  }

  // The action sets which are currently active (will be synced each frame)
  _vr->activeActionSets.push_back(_baseActionSet);
  // _vr->activeActionSets.push_back(_interactions["example"]->actionSet());
  _vr->activeActionSets.push_back(_interactions["teleport"]->actionSet());

  // add close handler to respond the close window button and pressing escape
  _desktopViewer->addEventHandler(vsg::CloseHandler::create(_desktopViewer));
}

void Game::frame()
{
  if (_spaceChangePending)
  {
    _spaceChangePending = false;
    _scene-> matrix = vsg::inverse(getPlayerTransform());
  }

  // OpenXR events must be checked first
  auto pol = _vr->pollEvents();
  if (pol == vsgvr::OpenXRViewer::PollEventsResult::Exit)
  {
    // User exited through VR overlay / XR runtime
    shouldExit = true;
    return;
  }

  if (pol == vsgvr::OpenXRViewer::PollEventsResult::NotRunning)
  {
    return;
  }
  else if (pol == vsgvr::OpenXRViewer::PollEventsResult::RuntimeIdle)
  {
    // Reduce power usage, wait for XR to wake
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }

  // Scene graph updates
  // TODO: This should be automatic, or handled by a graph traversal / node tags?
  // TODO: The transforms / spaces on these need to be validated. Visually they're correct,
  //       but there's probably bugs in here.
  if (_leftHandPose->getTransformValid())
  {
    _controllerLeft->matrix = _leftHandPose->getTransform();
  }
  if (_rightHandPose->getTransformValid())
  {
    _controllerRight->matrix = _rightHandPose->getTransform();
  }

  for (auto& interaction : _interactions)
  {
    if (std::find(_vr->activeActionSets.begin(), _vr->activeActionSets.end(), 
        interaction.second->actionSet()) != _vr->activeActionSets.end())
    {
      interaction.second->frame(_sceneRoot, *this);
    }
  }

  // Match the desktop camera to the HMD view
  // Ideally OpenXR would provide /user/head as a pose, but at least in the simple_controller profile it won't be available
  // Instead place the camera on one of the user's eyes, it'll be close enough
  _desktopCamera->viewMatrix = _xrCameras.front()->viewMatrix;
  // TODO: Just mirroring the projection matrix here isn't quite right but works as a desktop mirror window
  //       What may be better is placing the camera at the average of the xr cameras (between the eyes)
  _desktopCamera->projectionMatrix = _xrCameras.front()->projectionMatrix;

  // The session is running in some form, and a frame must be processed
  // The OpenXR frame loop takes priority - Acquire a frame to render into
  auto shouldQuit = false;

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

  if (_vr->advanceToNextFrame())
  {
    if (pol == vsgvr::OpenXRViewer::PollEventsResult::RunningDontRender)
    {
      // XR Runtime requested that rendering is not performed (not visible to user)
      // While this happens frames must still be acquired and released however, in
      // order to synchronise with the OpenXR runtime
    }
    else
    {
      // Render to the HMD
      _vr->recordAndSubmit(); // Render XR frame
    }
  }

  // End the frame, and present to user
  // Frames must be explicitly released, even if the previous advanceToNextFrame returned false (PollEventsResult::RunningDontRender)
  _vr->releaseFrame();
}

void Game::setPlayerOriginInWorld(vsg::dvec3 position)
{
  _playerOriginInWorld = position;
  _spaceChangePending = true;
}

void Game::setPlayerRotationInWorld(vsg::dquat rotation)
{
  _playerRotationInWorld = rotation;
  _spaceChangePending = true;
}

vsg::dmat4 Game::getPlayerTransform() {
  return vsg::translate(_playerOriginInWorld) * vsg::rotate(_playerRotationInWorld);
}
