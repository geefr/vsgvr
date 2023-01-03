namespace {
int exampleMain(int argc, char** argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // related options to use when reading and writing files.
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    arguments.read(options);

    // Initialise OpenXR through vsgvr
    // vsgvr has a similar Traits mechanism to a vsg::Window, however they affect multiple aspects of the runtime.
    // Some parts of these are configured up front, while others cannot be configured until later - Traits may
    // be accessed through vsgvr::Instance, but once configured should not be modified.
    //
    // Firstly an OpenXR Instance is required - In this case the application can only work for stereo head-mounted displays
    auto xrTraits = vsgvr::Traits::create();
    xrTraits->applicationName = "VSGVR Generic OpenXR Example";
    xrTraits->setApplicationVersion(0, 0, 0);

    for (auto& extension : Game::requiredInstanceExtensions())
    {
      xrTraits->xrExtensions.emplace_back(extension);
    }

    // An application could check for an exception when creating the instance and retry with different form factor,
    // but OpenXR applications are typically built around a known platform / configuration.
    auto xrInstance = vsgvr::Instance::create(XrFormFactor::XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY, xrTraits);
    xrTraits->viewConfigurationType = selectViewConfigurationType(xrInstance);
    xrTraits->environmentBlendMode = selectEnvironmentBlendMode(xrInstance, xrTraits->viewConfigurationType);

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "example_vr";
    arguments.read("--screen", windowTraits->screenNum);
    arguments.read("--display", windowTraits->display);
    windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    windowTraits->samples = VK_SAMPLE_COUNT_4_BIT;

    // Retrieve vulkan requirements
    // OpenXR will require certain vulkan versions, along with a specific physical device, and instance/device extensions
    auto xrVulkanReqs = vsgvr::GraphicsBindingVulkan::getVulkanRequirements(xrInstance);
    configureXrVulkanRequirements(windowTraits, xrVulkanReqs);

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
    vrViewer->referenceSpace = referenceSpace;

    // Run the application loop
    Game game(xrInstance, vrViewer, desktopViewer, true);
    while (!game.shouldExit)
    {
      game.frame();
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
}