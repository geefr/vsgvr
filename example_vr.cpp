#include <vsg/all.h>

#include <vsgvr/OpenXRInstance.h>
#include <vsgvr/OpenXRGraphicsBindingVulkan.h>
#include <vsgvr/OpenXRViewer.h>
#include <vsgvr/actions/OpenXRActionSet.h>
#include <vsgvr/actions/OpenXRActionPoseBinding.h>

#include <iostream>
#include <algorithm>

int main(int argc, char **argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // related options to use when reading and writing files.
    auto options = vsg::Options::create();
    arguments.read(options);

    vsg::Path filename = "world.vsgt";
    if (argc > 1)
      filename = arguments[1];
    if (arguments.errors())
      return arguments.writeErrorMessages(std::cerr);

    // load the scene graph
    vsg::ref_ptr<vsg::Group> vsg_scene =
        vsg::read_cast<vsg::Group>(filename, options);
    if (!vsg_scene)
      return 0;

    auto controllerNodeLeft = vsg::MatrixTransform::create();
    controllerNodeLeft->addChild(vsg::read_cast<vsg::Node>("controller.vsgt"));
    vsg_scene->addChild(controllerNodeLeft);

    auto controllerNodeRight = vsg::MatrixTransform::create();
    controllerNodeRight->addChild(vsg::read_cast<vsg::Node>("controller2.vsgt"));
    vsg_scene->addChild(controllerNodeRight);


    // Initialise vr, and add nodes to the scene graph for each tracked device
    // TODO: At the moment traits must be configured up front, exceptions will be thrown if these can't be satisfied
    // TODO: Some of these parameters are ignored at the moment
    auto xrTraits = vsgvr::OpenXrTraits();
    xrTraits.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    xrTraits.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "example_vr";
    arguments.read("--screen", windowTraits->screenNum);
    arguments.read("--display", windowTraits->display);
    
    // TODO: Instantiate devices and nodes in scene
    // TODO: If controllers are off when program starts they won't be added later
    // - This needs a better interface for controller poses - Ideally common between vr/xr apis, but xr does actions/spaces differently
    // vsgvr::createDeviceNodes(vr, vsg_scene, controllerNodeLeft, controllerNodeRight);

    // Initialise OpenXR, and retrieve vulkan requirements
    // OpenXR will require certain vulkan versions, along with a specific physical device
    // Use a desktop window to create the instance, and select the correct device
    auto xrInstance = vsgvr::OpenXRInstance::create(xrTraits);
    auto xrVulkanReqs = vsgvr::OpenXRGraphicsBindingVulkan::getVulkanRequirements(xrInstance);
    
    // Validate the Vulkan instance, and re-create the instance with the required extensions
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
    
    // TODO: For now, the window is just for instance creation, later it could also be used for mirror-window rendering
    auto desktopWindow = vsg::Window::create(windowTraits);
    auto vkInstance = desktopWindow->getOrCreateInstance();
    
    // Ensure the correct physical device is used
    VkPhysicalDevice xrRequiredDevice = vsgvr::OpenXRGraphicsBindingVulkan::getVulkanDeviceRequirements(xrInstance, vkInstance, xrVulkanReqs);
    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice;
    for (auto& dev : vkInstance->getPhysicalDevices())
    {
      if (dev->getPhysicalDevice() == xrRequiredDevice)
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

    // Ensure the VkDevice has been initialised
    // TODO: For now explicitly creating a device with just a graphics queue. OpenXR should be able to share the device directly from the window however,
    //       which probably ties into rendering the mirror window as part of the XR render graph (If that doesn't affect framerates)
    /*desktopWindow->getOrCreateSurface();
    desktopWindow->getOrCreateDevice();*/
    vsg::ref_ptr<vsg::Device> vkDevice;
    vsg::ref_ptr<vsgvr::OpenXRGraphicsBindingVulkan> graphicsBinding;
    {
      vsg::Names requestedLayers;
      if (windowTraits->debugLayer)
      {
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        if (windowTraits->apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
      }

      vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);
      vsg::Names deviceExtensions;
      deviceExtensions.insert(deviceExtensions.end(), windowTraits->deviceExtensionNames.begin(), windowTraits->deviceExtensionNames.end());

      auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
      if (queueFamily < 0)
      {
        std::cout << "Failed to find graphics queue" << std::endl;
      }

      vsg::QueueSettings queueSettings{ vsg::QueueSetting{queueFamily, {1.0}} };
      vkDevice = vsg::Device::create(physicalDevice, queueSettings, validatedNames, deviceExtensions, windowTraits->deviceFeatures, vkInstance->getAllocationCallbacks());
      std::cout << "Created our own vsg::Device " << vkDevice << std::endl;

      graphicsBinding = vsgvr::OpenXRGraphicsBindingVulkan::create(vkInstance, physicalDevice, vkDevice, queueFamily, 0);
    }
    
    // TODO: Hide the desktop window, it doesn't render anything for now
    desktopWindow = nullptr;

    // Set up a renderer to OpenXR, similar to a vsg::Viewer
    auto vr = vsgvr::OpenXRViewer::create(xrInstance, xrTraits, graphicsBinding);

    // add the CommandGraph to render the scene
    auto commandGraphs = vr->createCommandGraphsForView(vsg_scene, true);
    vr->assignRecordAndSubmitTaskAndPresentation(commandGraphs);

    // compile all Vulkan objects and transfer image, vertex and primitive data to GPU
    vr->compile();
    
    // TODO: Quick test of action sets / poses - Should be able to locate this space each frame, then turn it into a transform node update in the scene graph right?
    //       Probably easiest not to worry about pose-in-action-space - vsg can handle this
    auto baseActionSet = vsgvr::OpenXRActionSet::create(xrInstance, "gameplay", "Gameplay");
    auto leftHandPoseBinding = vsgvr::OpenXRActionPoseBinding::create(baseActionSet, "left_hand", "Left Hand");
    baseActionSet->actions.push_back(leftHandPoseBinding);
    auto rightHandPoseBinding = vsgvr::OpenXRActionPoseBinding::create(baseActionSet, "right_hand", "Right Hand");
    baseActionSet->actions.push_back(rightHandPoseBinding);
    // Ask OpenXR to bind the actions in the set
    if (baseActionSet->suggestInteractionBindings(xrInstance, "/interaction_profiles/khr/simple_controller", {
          {leftHandPoseBinding, "/user/hand/left/input/aim/pose"},
          {rightHandPoseBinding, "/user/hand/right/input/aim/pose"}
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

    // Render loop
    for(;;)
    {
      auto pol = vr->pollEvents();
      if( pol == vsgvr::OpenXRViewer::PollEventsResult::Exit)
      {
        // User exited through VR overlay / XR runtime
        break;
      }

      if( pol == vsgvr::OpenXRViewer::PollEventsResult::NotRunning)
      {
        continue;
      }
      else if (pol == vsgvr::OpenXRViewer::PollEventsResult::RuntimeIdle)
      {
        // Reduce power usage, wait for XR to wake
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }

      // Scene graph updates
      // TODO: This should be automatic, or handled by a graph traversal / node tags?
      if (leftHandPoseBinding->getTransformValid())
      {
        controllerNodeLeft->matrix = leftHandPoseBinding->getTransform();
      }
      if (rightHandPoseBinding->getTransformValid())
      {
        controllerNodeRight->matrix = rightHandPoseBinding->getTransform();
      }

      // The session is running, and a frame must be processed
      if (vr->advanceToNextFrame())
      {
        if (pol == vsgvr::OpenXRViewer::PollEventsResult::RunningDontRender)
        {
          // XR Runtime requested that rendering is not performed (not visible to user)
        }
        else
        {
          // Render a frame
          vr->update();
          vr->recordAndSubmit();
        }

        // End the frame, and present to user
        vr->releaseFrame();
      }
    }

    return EXIT_SUCCESS;
  }
  catch( const vsg::Exception& e )
  {
    std::cout << "VSG Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
}
