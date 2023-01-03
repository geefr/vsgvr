
#include "game.h"

#include <vsgvr/app/CompositionLayerQuad.h>
#include <vsgvr/app/CompositionLayerProjection.h>
#include <vsgvr/extensions/KHRCompositionLayerEquirect.h>
#include <vsgvr/extensions/KHRCompositionLayerEquirect2.h>


#include "../../models/controller/controller.cpp"
#include "../../models/controller/controller2.cpp"
#include "../../models/world/world.cpp"
#include "../../models/assets/teleport_marker/teleport_marker.cpp"

#include "interactions/interaction_teleport.h"

std::vector<std::string> Game::requiredInstanceExtensions()
{
  // See Game::initVR
  std::vector<std::string> extensions;

  // One of the Equirect layers for a skybox (These extensions are equivalent, with a marginally different API)
  if (vsgvr::Instance::checkInstanceExtensionSupported(vsgvr::KHRCompositionLayerEquirect::instanceExtension()))
  {
    extensions.emplace_back(vsgvr::KHRCompositionLayerEquirect::instanceExtension());
  }
  else if (vsgvr::Instance::checkInstanceExtensionSupported(vsgvr::KHRCompositionLayerEquirect2::instanceExtension()))
  {
    extensions.emplace_back(vsgvr::KHRCompositionLayerEquirect2::instanceExtension());
  }
  return extensions;
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

void Game::createCompositionLayerEquirect()
{
  // Create an equirect layer, covering the full surface of a sphere (default values)
  // The area covered on the sphere may be modified, though note that the 2 extensions
  // use different parameters to achieve this.
  if (vsgvr::Instance::checkInstanceExtensionSupported(vsgvr::KHRCompositionLayerEquirect::instanceExtension()))
  {
    auto layer = vsgvr::KHRCompositionLayerEquirect::create(_vr->referenceSpace);
    layer->clearColor = {0.7f, 0.2f, 0.2f};
    _skyboxLayer = layer;
  }
  else if (vsgvr::Instance::checkInstanceExtensionSupported(vsgvr::KHRCompositionLayerEquirect2::instanceExtension()))
  {
    auto layer = vsgvr::KHRCompositionLayerEquirect2::create(_vr->referenceSpace);
    layer->clearColor = {0.2f, 0.7f, 0.2f};
    _skyboxLayer = layer;
  }
  else
  {
    return;
  }

  auto renderGraph = vsg::RenderGraph::create();
  auto commandGraphs = _skyboxLayer->createCommandGraphsForRenderGraph(_vr->getInstance(), _vr->getSession(), renderGraph);
  _skyboxLayer->assignRecordAndSubmitTask(commandGraphs);
  _skyboxLayer->compile();
  _vr->compositionLayers.push_back(_skyboxLayer);
}

void Game::createCompositionLayerProjection()
{
  // Create CommandGraphs to render the scene to a projection layer
  // Projection layers do not represent geometry within the world - Rather they
  // represent the view to a VR headset or other display, and will auto-assign
  // views / cameras as required during createCommandGraphsForView
  auto layer = vsgvr::CompositionLayerProjection::create(_vr->referenceSpace);

  if( _skyboxLayer )
  {
    // By default the projection layer will not alpha-blend, and clear to the default vsg clearColour
    // With the skybox layer present however, allow it to show through
    layer->clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    layer->flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT;
  }

  // Note: assignHeadlight = false -> Scene lighting is required
  auto commandGraphs = layer->createCommandGraphsForView(_vr->getInstance(), _vr->getSession(), _sceneRoot, _xrCameras, false);
  layer->assignRecordAndSubmitTask(commandGraphs);
  layer->compile();
  _vr->compositionLayers.push_back(layer);
}

void Game::createCompositionLayerQuadInWorld()
{
  /*
  auto lookAt = vsg::LookAt::create(vsg::dvec3(0.0, 0.0, 10.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, -1.0, 0.0));
  // Camera parameters as if it's rendering to a desktop display, appropriate size for the displayed quad
  auto perspective = vsg::Perspective::create(30.0, 1920.0 / 1080.0, 0.1, 100.0);

  // Configure rendering from an overhead camera, displaying the scene
  // - A camera is not require here, but is required if the application later wants a reference to it
  // - The composition layer's basic parameters (pose, scale) may be modified later at any time
  // - There is a runtime-specific limit to the number of composition layers. Only a few should be used, if more than one.
  auto overheadCamera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(0, 0, 1920, 1080));

 
   // A quad positioned in the world (scene reference space)
   auto quadLayer = vsgvr::CompositionLayerQuad::create(_vr->getInstance(), _vr->getSession(), _vr->getTraits(), _vr->getSession()->getSpace(), 1920, 1080);
   quadLayer->setPose(
     {0.0, 4.0, 1.0},
     vsg::dquat(vsg::radians(25.0), {1.0, 0.0, 0.0})
   );
   quadLayer->sizeMeters.width = 1.920;
   quadLayer->sizeMeters.height = 1.080;

   // Pass a RenderGraph to the composition layer
   // This must _not_ have a window or view assigned - Windows are not relevant for OpenXR
   // rendering, rather OpenXR swapchains are acquired/released for each frame
   auto quadRenderGraph = vsg::RenderGraph::create();
   auto renderImgui = vsgvr::RenderImGui::create(_vr->getGraphicsBinding(), VkExtent2D{1920, 1080}, true);
   renderImgui->add(_ui);
   quadRenderGraph->addChild(renderImgui);
   auto quadCommandGraphs = quadLayer->createCommandGraphsForRenderGraph(_vr->getSession(), quadRenderGraph);
   quadLayer->assignRecordAndSubmitTask({quadCommandGraphs});
   quadLayer->compile();
   _vr->compositionLayers.emplace_back(quadLayer);
   */
}

void Game::createCompositionLayerQuadInView()
{
  /*
   // A quad positioned in the world (scene reference space)
   auto quadLayer = vsgvr::CompositionLayerQuad::create(_vr->getInstance(), _vr->getSession(), _vr->getTraits(), _vr->getSession()->getSpace(), 1920, 1080);
   quadLayer->setPose(
     {0.0, 4.0, 1.0},
     vsg::dquat(vsg::radians(25.0), {1.0, 0.0, 0.0})
   );
   quadLayer->sizeMeters.width = 1.920;
   quadLayer->sizeMeters.height = 1.080;

   // Pass a RenderGraph to the composition layer
   // This must _not_ have a window or view assigned - Windows are not relevant for OpenXR
   // rendering, rather OpenXR swapchains are acquired/released for each frame
   auto quadRenderGraph = vsg::RenderGraph::create();
   auto renderImgui = vsgvr::RenderImGui::create(_vr->getGraphicsBinding(), VkExtent2D{1920, 1080}, true);
   renderImgui->add(_ui);
   quadRenderGraph->addChild(renderImgui);
   auto quadCommandGraphs = quadLayer->createCommandGraphsForRenderGraph(_vr->getSession(), quadRenderGraph);
   quadLayer->assignRecordAndSubmitTask({quadCommandGraphs});
   quadLayer->compile();
   _vr->compositionLayers.emplace_back(quadLayer);
   */
}

void Game::initVR()
{
  // The base OpenXR specification includes CompositionLayerProjection and CompositionLayerQuad
  // Any further layers require extensions, which must be requested during instance creation (main.cpp)
  // 
  // Note that OpenXR layers are fairly vendor-specific, and a smaller subset are available across runtimes.
  // Notably SteamVR does not support either KHRCompositionLayerEquirect layer referenced below, and both
  // the Oculus desktop and mobile runtimes use a different versions of the equirect layer.
  // 
  // For elements such as a skybox within a real application - A decision is needed
  // between using an OpenXR composition layer, or rendering the skybox within the scene, as a desktop application would
  // See https://github.khronos.org/OpenXR-Inventory/extension_support.html#matrix
  //
  // Composition layers are rendered in the order provided to vsgvr::Viewer::compositionLayers,
  // and are combined using vsgvr::Traits::environmentBlendMode.

  // Equirect image -> inside of a sphere (skybox)
  // Extension layer - May not be present on all runtimes
  createCompositionLayerEquirect();

  // Scene -> Headset views
  createCompositionLayerProjection();

  // Image -> A quad in world space (pause menu or similar)
  createCompositionLayerQuadInWorld();

  // Image -> A quad in view space (A hud)
  createCompositionLayerQuadInView();

  // Render a desktop window, mirroring the contents of the headset
  // As this is a separate window rendered by vsg it only renders the scene - OpenXR composition layers are not present
  // (But are present when viewing a desktop mirror window provided by the OpenXR runtime)
  if(_desktopWindowEnabled)
  {
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
  // This may be done by the application, but vsgvr::Viewer can also track a set of SpaceBindings per-frame (within the session's reference space)
  _headPose = vsgvr::SpaceBinding::create(vsgvr::ReferenceSpace::create(_vr->getSession(), XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_VIEW));
  _vr->spaceBindings.push_back(_headPose);

  // Input devices are tracked via ActionPoseBindings - Tracking elements from the OpenXR device tree in the session space,
  // along with binding the OpenXR input subsystem through to usable actions.
  _baseActionSet = vsgvr::ActionSet::create(_xrInstance, "controller_positions", "Controller Positions");
  // Pose bindings
  _leftHandPose = vsgvr::ActionPoseBinding::create(_xrInstance, _baseActionSet, "left_hand", "Left Hand");
  _rightHandPose = vsgvr::ActionPoseBinding::create(_xrInstance, _baseActionSet, "right_hand", "Right Hand");

  _baseActionSet->actions = {
    _leftHandPose,
    _rightHandPose,
  };

  _interactions.emplace("teleport", new Interaction_teleport(_xrInstance, _headPose, _leftHandPose, _teleportMarker, _ground));

  // Ask OpenXR to suggest interaction bindings.
  // * If subpaths are used, list all paths that each action should be bound for
  // * Note that this may only be called once for each interaction profile (but may be across multiple overlapping action sets)
  // * If a particular profile is used, all interactions should be bound to this i.e. If grabbing items only specifies bindings for
  //   an oculus controller, it will not be bound if the simple_controller is chosen by the runtime
  std::map<std::string, std::list<vsgvr::ActionSet::SuggestedInteractionBinding>> actionsToSuggest;
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
    if (! vsgvr::ActionSet::suggestInteractionBindings(_xrInstance, p.first, p.second))
    {
      throw std::runtime_error({ "Failed to configure interaction bindings for controllers" });
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
  
  // TODO: Set up input action to switch between modes
  // TODO: Display active mode somewhere in the world, maybe as text panel when looking at controllers
  _vr->activeActionSets.push_back(_interactions["teleport"]->actionSet());

  // add close handler to respond the close window button and pressing escape
  _desktopViewer->addEventHandler(vsg::CloseHandler::create(_desktopViewer));
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
