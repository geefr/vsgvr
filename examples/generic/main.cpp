#include <vsg/all.h>

#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>
#include <vsgvr/xr/ViewMatrix.h>

#include <vsgvr/app/Viewer.h>
#include <vsgvr/app/CompositionLayerprojection.h>
#include <vsgvr/actions/ActionSet.h>
#include <vsgvr/actions/ActionPoseBinding.h>

#include <iostream>
#include <algorithm>

#include "../../models/controller/controller.cpp"
#include "../../models/controller/controller2.cpp"
#include "../../models/world/world.cpp"

int main(int argc, char **argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // related options to use when reading and writing files.
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    arguments.read(options);

    auto vsg_scene = vsg::Group::create();

    // load the scene graph
    // * Load the bulk of the scene from command line, or the built-in world model
    // * Always load controllers separately from built-in models
    // read any vsg files
    for (int i = 1; i < argc; ++i)
    {
      vsg::Path filename = arguments[i];
      auto path = vsg::filePath(filename);
      auto object = vsg::read(filename, options);
      if (auto node = object.cast<vsg::Node>(); node)
      {
        vsg_scene->addChild(node);
      }
      else if (object)
      {
        std::cerr << "Unable to view object of type " << object->className() << std::endl;
        return EXIT_FAILURE;
      }
      else
      {
        std::cerr << "Unable to load file " << filename << std::endl;
        return EXIT_FAILURE;
      }
    }

    if (vsg_scene->children.size() == 0)
    {
      std::cerr << "Loading built-in example scene" << std::endl;
      vsg_scene->addChild(world());
    }

    auto controllerNodeLeft = controller();
    vsg_scene->addChild(controllerNodeLeft);
    auto controllerNodeRight = controller2();
    vsg_scene->addChild(controllerNodeRight);

    // Initialise OpenXR
    // TODO: At the moment traits must be configured up front, exceptions will be thrown if these can't be satisfied
    //       This should be improved in the future, at least to query what form factors are available.
    // TODO: Some parameters on xrTraits are non-functional at the moment
    auto xrTraits = vsgvr::Traits::create();
    xrTraits->formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    xrTraits->viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "example_vr";
    arguments.read("--screen", windowTraits->screenNum);
    arguments.read("--display", windowTraits->display);

    // Retrieve vulkan requirements
    // OpenXR will require certain vulkan versions, along with a specific physical device, and instance/device extensions
    auto xrInstance = vsgvr::Instance::create(xrTraits);
    auto xrVulkanReqs = vsgvr::GraphicsBindingVulkan::getVulkanRequirements(xrInstance);

    if (windowTraits->vulkanVersion < xrVulkanReqs.minVersion)
    {
      std::cout << "Vulkan API too low for OpenXR. Minimum required is " << xrVulkanReqs.minVersionStr << std::endl;
      return EXIT_FAILURE;
    }
    if (windowTraits->vulkanVersion > xrVulkanReqs.maxVersion)
    {
      std::cout << "Warning: Vulkan API higher than OpenXR maximum. Maximum tested version is " << xrVulkanReqs.maxVersionStr << std::endl;
    }

    // Encountered with SteamVR: debug_marker is requested in device extensions, causing device creation to fail
    // This doesn't actually appear to be required by SteamVR to work, so drop it if present.
    // debug_report doesn't appear to be present, but drop it as well.
    // SteamVR doesn't appear to need these to run, or the replacement VK_EXT_debug_utils.
    xrVulkanReqs.deviceExtensions.erase("VK_EXT_debug_marker");
    xrVulkanReqs.instanceExtensions.erase("VK_EXT_debug_report");
    // Add any other requirements of OpenXR to the window traits, this mostly includes memory sharing and synchronisation extensions
    for (auto& ext : xrVulkanReqs.instanceExtensions)
    {
      if (std::find(windowTraits->instanceExtensionNames.begin(), windowTraits->instanceExtensionNames.end(), ext) == windowTraits->instanceExtensionNames.end() ) {
        windowTraits->instanceExtensionNames.push_back(ext.c_str());
      }
    }
    for (auto& ext : xrVulkanReqs.deviceExtensions)
    {
      if (std::find(windowTraits->deviceExtensionNames.begin(), windowTraits->deviceExtensionNames.end(), ext) == windowTraits->deviceExtensionNames.end()) {
        windowTraits->deviceExtensionNames.push_back(ext.c_str());
      }
    }
    
    // VSync must be disabled - In this example we're driving a desktop window and HMD from the same thread,
    // the OpenXRViewer must never be waiting for the desktop window to sync
    windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    auto desktopViewer = vsg::Viewer::create();
    auto desktopWindow = vsg::Window::create(windowTraits);
    if (!desktopWindow)
    {
        std::cout << "Could not create windows." << std::endl;
        return EXIT_FAILURE;
    }
    desktopViewer->addWindow(desktopWindow);

    // Ensure the correct physical device is selected
    // Technically the desktop window and HMD could use different devices, but for simplicity OpenXR is configured to use the same device as the desktop
    auto vkInstance = desktopWindow->getOrCreateInstance();
    VkPhysicalDevice xrRequiredDevice = vsgvr::GraphicsBindingVulkan::getVulkanDeviceRequirements(xrInstance, vkInstance, xrVulkanReqs);
    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice;
    for (auto& dev : vkInstance->getPhysicalDevices())
    {
      if (dev->vk() == xrRequiredDevice)
      {
        physicalDevice = dev;
      }
    }
    if (!physicalDevice)
    {
      std::cout << "Unable to select physical device, as required by OpenXR" << std::endl;
      return EXIT_FAILURE;
    }
    desktopWindow->setPhysicalDevice(physicalDevice);

    // Bind OpenXR to the Device
    desktopWindow->getOrCreateSurface();
    auto vkDevice = desktopWindow->getOrCreateDevice();
    auto graphicsBinding = vsgvr::GraphicsBindingVulkan::create(vkInstance, physicalDevice, vkDevice, physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT), 0);

    // Set up a renderer to OpenXR, similar to a vsg::Viewer
    auto vr = vsgvr::Viewer::create(xrInstance, xrTraits, graphicsBinding);

    // Create CommandGraphs to render the scene to the HMD
    std::vector<vsg::ref_ptr<vsg::Camera>> xrCameras;
    // TODO: This only really exists because vsg::createCommandGraphForView requires
    // a Window instance. Other than some possible improvements later, it could use the same code as vsg
    // OpenXR rendering may use one or more command graphs, as decided by the viewer
    // (TODO: At the moment only a single CommandGraph will be used, even if there's multiple XR views)
    auto headsetCompositionLayer = vsgvr::CompositionLayerProjection::create(vr->getInstance(), vr->getTraits(), vr->getSession()->getSpace());
    auto xrCommandGraphs = headsetCompositionLayer->createCommandGraphsForView(vr->getSession(), vsg_scene, xrCameras, false);
    // TODO: This is almost identical to Viewer::assignRecordAndSubmitTaskAndPresentation - The only difference is
    // that OpenXRViewer doesn't have presentation - If presentation was abstracted we could avoid awkward duplication here
    headsetCompositionLayer->assignRecordAndSubmitTask(xrCommandGraphs);
    // TODO: This is identical to Viewer::compile, except CompileManager requires a child class of Viewer
    // OpenXRViewer can't be a child class of Viewer yet (Think this was due to the assumption that a Window/Viewer has presentation / A Surface)
    headsetCompositionLayer->compile();
    vr->compositionLayers.push_back(headsetCompositionLayer);

    // Create a CommandGraph to render the desktop window
    // Directly sharing one of the HMD's cameras here is tempting, but causes a deadlock under SteamVR.
    // To avoid that, and present a nicer view create a regular camera for the desktop, and map it to the location of the user's head
    auto lookAt = vsg::LookAt::create(vsg::dvec3(-4.0, -15.0, 25.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 0.0, 1.0));
    auto perspective = vsg::Perspective::create(30.0, static_cast<double>(desktopWindow->extent2D().width) / static_cast<double>(desktopWindow->extent2D().height), 0.1, 100.0);
    auto desktopCamera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(desktopWindow->extent2D()));
    auto desktopCommandGraph = vsg::createCommandGraphForView(desktopWindow, desktopCamera, vsg_scene);

    desktopViewer->assignRecordAndSubmitTaskAndPresentation({desktopCommandGraph});
    desktopViewer->compile();
    
    // Configure OpenXR action sets and pose bindings - These allow elements of the OpenXR device tree to be located and tracked in space,
    // along with binding the OpenXR input subsystem through to usable actions.
    auto baseActionSet = vsgvr::ActionSet::create(xrInstance, "gameplay", "Gameplay");

    // Pose bindings - One for each hand
    auto leftHandPoseBinding = vsgvr::ActionPoseBinding::create(xrInstance, baseActionSet, "left_hand", "Left Hand");
    auto rightHandPoseBinding = vsgvr::ActionPoseBinding::create(xrInstance, baseActionSet, "right_hand", "Right Hand");

    // An action - In this case applied to both left and right hands
    // Two separate actions could also be used (Note: Though may not work on the Oculus Quest runtime)
    auto example_trigger_action = vsgvr::Action::create(xrInstance, baseActionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT,
      "hand_select", "Hand Select", std::vector<std::string>{ "/user/hand/left", "/user/hand/right" });

    baseActionSet->actions = {
      leftHandPoseBinding,
      rightHandPoseBinding,
      example_trigger_action,
    };

    // Ask OpenXR to suggest interaction bindings.
    // * If subpaths are used, list all paths that each action should be bound for
    // * Note that this may only be called once for each interaction profile (but may be across multiple overlapping action sets)
    if (vsgvr::ActionSet::suggestInteractionBindings(xrInstance, "/interaction_profiles/khr/simple_controller", {
          {leftHandPoseBinding, "/user/hand/left/input/aim/pose"},
          {rightHandPoseBinding, "/user/hand/right/input/aim/pose"},
          {example_trigger_action, "/user/hand/left/input/select/click"},
          {example_trigger_action, "/user/hand/right/input/select/click"},
       }))
    {
      // All action sets, which will be initialised in the viewer's session
      vr->actionSets.push_back(baseActionSet);

      // The active action set(s)
      // These will be updated during the render loop, and update the scene graph accordingly
      // Active action sets may be changed before calling pollEvents
      vr->activeActionSets.push_back(baseActionSet);
    }
    else
    {
      std::cout << "Failed to configure interaction bindings for controllers" << std::endl;
      return EXIT_FAILURE;
    }

    // add close handler to respond the close window button and pressing escape
    desktopViewer->addEventHandler(vsg::CloseHandler::create(desktopViewer));

    double leftRot = 0.0;
    double rightRot = 0.0;

    // Render loop
    for(;;)
    {
      // OpenXR events must be checked first
      auto pol = vr->pollEvents();
      if( pol == vsgvr::Viewer::PollEventsResult::Exit )
      {
        // User exited through VR overlay / XR runtime
        break;
      }

      if( pol == vsgvr::Viewer::PollEventsResult::NotRunning)
      {
        continue;
      }
      else if (pol == vsgvr::Viewer::PollEventsResult::RuntimeIdle)
      {
        // Reduce power usage, wait for XR to wake
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }

      // Scene graph updates
      // TODO: This should be automatic, or handled by a graph traversal / node tags?
      // TODO: The transforms / spaces on these need to be validated. Visually they're correct,
      //       but there's probably bugs in here.
      if (leftHandPoseBinding->getTransformValid())
      {
        controllerNodeLeft->matrix = leftHandPoseBinding->getTransform();
      }
      if (rightHandPoseBinding->getTransformValid())
      {
        controllerNodeRight->matrix = rightHandPoseBinding->getTransform();
      }

      // Quick input test - When triggers pressed (select action binding) make controllers spin
      // Note coordinate space of controllers - Z is forward
      if(example_trigger_action->getStateValid("/user/hand/left"))
      {
        auto lState = example_trigger_action->getStateBool("/user/hand/left");
        if (lState.isActive && lState.currentState)
        {
          leftRot -= 0.1;
        }
      }
      if(example_trigger_action->getStateValid("/user/hand/right"))
      {
        auto rState = example_trigger_action->getStateBool("/user/hand/right");
        if (rState.isActive && rState.currentState)
        {
          rightRot += 0.1;
        }
      }
      controllerNodeLeft->matrix = controllerNodeLeft->matrix * vsg::rotate(leftRot, { 0.0, 0.0, 1.0 });
      controllerNodeRight->matrix = controllerNodeRight->matrix * vsg::rotate(rightRot, { 0.0, 0.0, 1.0 });

      // Match the desktop camera to the HMD view
      // Ideally OpenXR would provide /user/head as a pose, but at least in the simple_controller profile it won't be available
      // Instead place the camera on one of the user's eyes, it'll be close enough
      desktopCamera->viewMatrix = xrCameras.front()->viewMatrix;
      // TODO: Just mirroring the projection matrix here isn't quite right but works as a desktop mirror window
      //       What may be better is placing the camera at the average of the xr cameras (between the eyes)
      desktopCamera->projectionMatrix = xrCameras.front()->projectionMatrix;

      // The session is running in some form, and a frame must be processed
      // The OpenXR frame loop takes priority - Acquire a frame to render into
      auto shouldQuit = false;

      // Desktop render
      // * The scene graph is updated by the desktop render
      // * if PollEventsResult::RunningDontRender the desktop render could be skipped
      if (desktopViewer->advanceToNextFrame())
      {
        desktopViewer->handleEvents();
        desktopViewer->update();
        desktopViewer->recordAndSubmit();
        desktopViewer->present();
      }
      else
      {
        // Desktop window was closed
        shouldQuit = true;
      }

      if (vr->advanceToNextFrame())
      {
        if (pol == vsgvr::Viewer::PollEventsResult::RunningDontRender)
        {
          // XR Runtime requested that rendering is not performed (not visible to user)
          // While this happens frames must still be acquired and released however, in
          // order to synchronise with the OpenXR runtime
        }
        else
        {
          // Render to the HMD
          vr->recordAndSubmit(); // Render XR frame
        }
      }

      // End the frame, and present to user
      // Frames must be explicitly released, even if the previous advanceToNextFrame returned false (PollEventsResult::RunningDontRender)
      vr->releaseFrame();

      if(shouldQuit)
      {
        break;
      }
    }

    return EXIT_SUCCESS;
  }
  catch( const vsg::Exception& e )
  {
    std::cerr << "VSG Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
}
