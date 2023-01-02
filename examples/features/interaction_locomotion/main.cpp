#include <vsg/all.h>

#include <iostream>

#include "game.h"

int main(int argc, char** argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // related options to use when reading and writing files.
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    arguments.read(options);

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

    Game game(xrInstance, vr, desktopViewer, true);

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
