#include <vsg/all.h>

#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>
#include <vsgvr/xr/ViewMatrix.h>

#include <vsgvr/app/Viewer.h>
#include <vsgvr/app/CompositionLayerProjection.h>
#include <vsgvr/actions/ActionSet.h>
#include <vsgvr/actions/ActionPoseBinding.h>

#include <iostream>
#include <algorithm>

#include "../../models/controller/controller.cpp"
#include "../../models/controller/controller2.cpp"
#include "../../models/world/world.cpp"

XrViewConfigurationType selectViewConfigurationType(vsg::ref_ptr<vsgvr::Instance> instance)
{
  // Most likely will be stereo for a head-mounted display.
  // Mono will be used if supported otherwise.
  // QUAD_VARJO and other extension-based configs could be selected here, but the application would
  // also need to enable these extensions during Instance::create.
  std::vector<XrViewConfigurationType> viewPrefs = {
    XrViewConfigurationType::XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
    XrViewConfigurationType::XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO,
  };
  for (auto& view : viewPrefs)
  {
    if (instance->checkViewConfigurationSupported(view))
    {
      return view;
    }
  }
  // Fall back to first-supported
  return instance->getSupportedViewConfigurationTypes().front();
}

XrEnvironmentBlendMode selectEnvironmentBlendMode(vsg::ref_ptr<vsgvr::Instance> instance, XrViewConfigurationType viewConfigurationType)
{
  // VR headsets should support OPAQUE
  // AR headsets should support ADDITIVE or ALPHA_BLEND
  std::vector<XrEnvironmentBlendMode> blendModes = {
    XrEnvironmentBlendMode::XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
    XrEnvironmentBlendMode::XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
    XrEnvironmentBlendMode::XR_ENVIRONMENT_BLEND_MODE_ADDITIVE,
  };
  for (auto& mode : blendModes)
  {
    if (instance->checkEnvironmentBlendModeSupported(viewConfigurationType, mode))
    {
      return mode;
    }
  }
  // Fall back to first-supported
  return instance->getSupportedEnvironmentBlendModes(viewConfigurationType).front();
}

void configureXrVulkanRequirements(vsg::ref_ptr<vsg::WindowTraits> windowTraits, vsgvr::VulkanRequirements xrVulkanReqs)
{
  if (windowTraits->vulkanVersion < xrVulkanReqs.minVersion)
  {
    throw std::runtime_error("Vulkan API too low for OpenXR");
  }
  if (windowTraits->vulkanVersion > xrVulkanReqs.maxVersion)
  {
    std::cout << "Warning: Vulkan API higher than OpenXR maximum. Maximum tested version is " << xrVulkanReqs.maxVersionStr << std::endl;
  }

  // Encountered with SteamVR: debug_marker is requested in device extensions but is not present, causing device creation to fail
  // This doesn't actually appear to be required by SteamVR to work, so drop it if present.
  xrVulkanReqs.deviceExtensions.erase("VK_EXT_debug_marker");
  xrVulkanReqs.instanceExtensions.erase("VK_EXT_debug_report");
  // Add any other requirements of OpenXR to the window traits, this mostly includes memory sharing and synchronisation extensions
  for (auto& ext : xrVulkanReqs.instanceExtensions)
  {
    if (std::find(windowTraits->instanceExtensionNames.begin(), windowTraits->instanceExtensionNames.end(), ext) == windowTraits->instanceExtensionNames.end()) {
      windowTraits->instanceExtensionNames.push_back(ext.c_str());
    }
  }
  for (auto& ext : xrVulkanReqs.deviceExtensions)
  {
    if (std::find(windowTraits->deviceExtensionNames.begin(), windowTraits->deviceExtensionNames.end(), ext) == windowTraits->deviceExtensionNames.end()) {
      windowTraits->deviceExtensionNames.push_back(ext.c_str());
    }
  }
}

VkFormat selectSwapchainFormat(vsg::ref_ptr<vsgvr::Session> vrSession)
{
  // The specific swapchain format shouldn't impact the application much, but a compatible
  // image format must be selected for OpenXR to operate
  // These two formats are listed in the OpenXR specification, and should be supported by
  // most runtimes.
  std::vector<VkFormat> formats = {
    VkFormat::VK_FORMAT_R8G8B8A8_UNORM,
    VkFormat::VK_FORMAT_R8G8B8A8_SRGB
  };
  for (auto& format : formats) {
    if (vrSession->checkSwapchainFormatSupported(format))
    {
      return format;
    }
  }
  // Fall back to first-supported
  return vrSession->getSupportedSwapchainFormats().front();
}

uint32_t selectSwapchainSampleCount(vsg::ref_ptr<vsgvr::Session> vrSession, VkSampleCountFlags samples)
{
  // Similar to swapchain image format, the number of samples shouldn't matter much, but if
  // possible should match the multisample settings of the vsg rendering.
  // After rendering, swapchain images are handed to the OpenXR compositor, which should
  // have access to the highest-quality image available.
  if (vrSession->checkSwapchainSampleCountSupported(samples))
  {
    return samples;
  }
  return 1;
}

XrReferenceSpaceType selectReferenceSpaceType(vsg::ref_ptr<vsgvr::Session> vrSession)
{
  // The spaces available will depend upon the OpenXR runtime
  // Typically STAGE is supported for room-scale systems,
  // with LOCAL supported for standing-only scale systems.
  // Most runtimes will support multiple however, or support
  // additional extension-based spaces.
  //
  // For common VR headsets, the difference between STAGE and LOCAL may be quite subtle,
  // and have little impact on this application.
  std::vector<XrReferenceSpaceType> spaces = {
    XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_STAGE,
    XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_LOCAL,
  };
  for (auto& space : spaces)
  {
    if (vrSession->checkReferenceSpaceTypeSupported(space))
    {
      return space;
    }
  }
  return vrSession->getSupportedReferenceSpaceTypes().front();
}

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

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "example_vr";
    arguments.read("--screen", windowTraits->screenNum);
    arguments.read("--display", windowTraits->display);

    // Initialise OpenXR through vsgvr
    // vsgvr has a similar Traits mechanism to a vsg::Window, however they affect multiple aspects of the runtime.
    // Some parts of these are configured up front, while others cannot be configured until later - Traits may
    // be accessed through vsgvr::Instance, but once configured should not be modified.
    //
    // Firstly an OpenXR Instance is required - In this case the application can only work for stereo head-mounted displays
    auto xrTraits = vsgvr::Traits::create();
    xrTraits->applicationName = "VSGVR Generic OpenXR Example";
    xrTraits->setApplicationVersion(0, 0, 0);
    // An application could check for an exception when creating the instance and retry with different form factor,
    // but OpenXR applications are typically built around a known platform / configuration.
    auto xrInstance = vsgvr::Instance::create(XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY, xrTraits);
    xrTraits->viewConfigurationType = selectViewConfigurationType(xrInstance);
    xrTraits->environmentBlendMode = selectEnvironmentBlendMode(xrInstance, xrTraits->viewConfigurationType);

    // Retrieve the vulkan requirements - The OpenXR runtime will require certain vulkan versions,
    // along with a specific physical device, and instance/device extensions
    auto xrVulkanReqs = vsgvr::GraphicsBindingVulkan::getVulkanRequirements(xrInstance);
    configureXrVulkanRequirements(windowTraits, xrVulkanReqs);

    // Configure a desktop Window and Viewer, for both Vulkan initialisation and mirror window
    // VSync must be disabled - The desktop window and HMD are being rendered from a single thread,
    // the vsgvr Viewer must never be waiting for the desktop window to sync, otherwise the VR framerate will be limites as well.
    windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    // And enable multisampling
    windowTraits->samples = VK_SAMPLE_COUNT_4_BIT;

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

    // Bind OpenXR to the desktop window's vulkan instance
    desktopWindow->getOrCreateSurface();
    auto vkDevice = desktopWindow->getOrCreateDevice();
    auto graphicsBinding = vsgvr::GraphicsBindingVulkan::create(vkInstance, physicalDevice, vkDevice, physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT), 0);

    // Configure the OpenXR session, managed by a vsgvr Viewer
    // As part of this, perform further trait validation, and selection of appropriate rendering parameters
    auto vrViewer = vsgvr::Viewer::create(xrInstance, graphicsBinding);
    auto vrSession = vrViewer->getSession();
    xrTraits->swapchainFormat = selectSwapchainFormat(vrSession);
    xrTraits->swapchainSampleCount = selectSwapchainSampleCount(vrSession, windowTraits->samples);

    // Lastly define the world space used for the application, from one of the OpenXR reference spaces
    // This is required both for rendering, and as a reference to locate tracked devices within.
    auto referenceSpaceType = selectReferenceSpaceType(vrSession);
    // Configure world space to be the origin of the selected reference space type
    // Our reference space may be rotated or translated if required
    auto referenceSpace = vsgvr::ReferenceSpace::create(vrSession, referenceSpaceType);

    // Configure rendering of the vsg scene into a composition layer
    // Multiple composition layers may be provided, but at minimum a single CompositionLayerProjection is needed
    // to display the scene within the headset / OpenXR displays.
    //
    // CompositionLayerProjection is somewhat special in that it doesn't represent an object within the world,
    // rather it is always bound to the headset, and cameras / views within the layer will be auto-assigned.
    //
    // Other object-based layers such as CompositionLayerQuad appear within the world as textured objects,
    // and may be moved / rotated by defining another ReferenceSpace, based upon our defined world space
    std::vector<vsg::ref_ptr<vsg::Camera>> xrCameras;
    auto headsetCompositionLayer = vsgvr::CompositionLayerProjection::create(referenceSpace);
    auto xrCommandGraphs = headsetCompositionLayer->createCommandGraphsForView(xrInstance, vrSession, vsg_scene, xrCameras, false);
    headsetCompositionLayer->assignRecordAndSubmitTask(xrCommandGraphs);
    headsetCompositionLayer->compile();
    vrViewer->compositionLayers.push_back(headsetCompositionLayer);

    // Create a CommandGraph to render the desktop window
    // Directly sharing one of the HMD's cameras here is tempting, but will cause a deadlock under SteamVR.
    // To avoid that, and present a nicer view create a regular camera for the desktop, and mirror the position / matrices across.
    // A vsgvr::SpaceBinding may also be used to track the VIEW space relative to referenceSpace - To obtain the pose of
    // the user's head within world space.
    auto lookAt = vsg::LookAt::create(vsg::dvec3(-4.0, -15.0, 25.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 0.0, 1.0));
    auto perspective = vsg::Perspective::create(30.0, static_cast<double>(desktopWindow->extent2D().width) / static_cast<double>(desktopWindow->extent2D().height), 0.1, 100.0);
    auto desktopCamera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(desktopWindow->extent2D()));
    auto desktopCommandGraph = vsg::createCommandGraphForView(desktopWindow, desktopCamera, vsg_scene);
    desktopViewer->assignRecordAndSubmitTaskAndPresentation({desktopCommandGraph});
    desktopViewer->compile();
    
    // This example shows 2 models at the location of the user's controllers, for this an ActionSet is required.
    // Also bind the controller triggers such that we can detect the trigger press while rendering (See further interaction examples)
    auto baseActionSet = vsgvr::ActionSet::create(xrInstance, "gameplay", "Gameplay");
    auto leftHandPoseBinding = vsgvr::ActionPoseBinding::create(xrInstance, baseActionSet, "left_hand", "Left Hand");
    auto rightHandPoseBinding = vsgvr::ActionPoseBinding::create(xrInstance, baseActionSet, "right_hand", "Right Hand");
    auto example_trigger_action = vsgvr::Action::create(xrInstance, baseActionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT,
      "hand_select", "Hand Select", std::vector<std::string>{ "/user/hand/left", "/user/hand/right" });
    baseActionSet->actions = {
      leftHandPoseBinding,
      rightHandPoseBinding,
      example_trigger_action,
    };
    if (vsgvr::ActionSet::suggestInteractionBindings(xrInstance, "/interaction_profiles/khr/simple_controller", {
          {leftHandPoseBinding, "/user/hand/left/input/aim/pose"},
          {rightHandPoseBinding, "/user/hand/right/input/aim/pose"},
          {example_trigger_action, "/user/hand/left/input/select/click"},
          {example_trigger_action, "/user/hand/right/input/select/click"},
       }))
    {
      vrViewer->actionSets.push_back(baseActionSet);
      vrViewer->activeActionSets.push_back(baseActionSet);
    }
    else
    {
      throw std::runtime_error("Failed to configure interaction bindings for controllers");
    }

    // add close handler to respond the close window button and pressing escape
    desktopViewer->addEventHandler(vsg::CloseHandler::create(desktopViewer));
    double leftRot = 0.0;
    double rightRot = 0.0;

    // vsgvr render loop
    // This render loop is more complex than a desktop equivalent, as the application must perform
    // certain actions, based on the OpenXR runtime's session lifecycle. See vsgvr::Viewer::PollEventsResult.
    for(;;)
    {
      // OpenXR events must be checked first
      auto pol = vrViewer->pollEvents();
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

      // Scene graph updates, to position controller models
      if (leftHandPoseBinding->getTransformValid())
      {
        controllerNodeLeft->matrix = leftHandPoseBinding->getTransform();
      }
      if (rightHandPoseBinding->getTransformValid())
      {
        controllerNodeRight->matrix = rightHandPoseBinding->getTransform();
      }

      // Simple input check - When triggers are pulled, rotate the controllers
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

      // Match the desktop camera to the HMD view - mirroring the projectionMatrix exactly
      // is non-optimal here, but works as a simple desktop mirror setup.
      desktopCamera->viewMatrix = xrCameras.front()->viewMatrix;
      desktopCamera->projectionMatrix = xrCameras.front()->projectionMatrix;

      // Desktop render
      // * The scene graph is updated by the desktop render
      // * if PollEventsResult::RunningDontRender the desktop render could be skipped
      auto shouldQuit = false;
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

      // The PollEventsResult signifies that the session is running in some form.
      // In this case a frame must be rendered by the application, even if it is empty,
      // this is important to maintain sync between the application's render loop
      // and OpenXR runtime.
      if (vrViewer->advanceToNextFrame())
      {
        if (pol == vsgvr::Viewer::PollEventsResult::RunningDontRender)
        {
          // XR Runtime requested that rendering is not performed (not visible to user)
          // While this happens frames must still be acquired and released
        }
        else
        {
          // Render each of the composition layers to their swapchains
          vrViewer->recordAndSubmit();
        }
      }

      // End the frame, and present composition layers to the OpenXR compositor
      // Frames must be explicitly released, even if the previous advanceToNextFrame returned false
      vrViewer->releaseFrame();

      if(shouldQuit)
      {
        break;
      }
    }

    return EXIT_SUCCESS;
  }
  catch (const vsg::Exception& e)
  {
    std::cerr << "VSG Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
  catch (const vsgvr::Exception& e)
  {
    std::cerr << "VSGVR Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
