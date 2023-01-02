
#include <initializer_list>
#include <cstdlib>
#include <cstring>
#include <jni.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <vsg/all.h>
#include <vsg/platform/android/Android_Window.h>

#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>
#include <vsgvr/xr/ViewMatrix.h>
#include <vsgvr/xr/AndroidTraits.h>
#include <vsgvr/app/Viewer.h>
#include <vsgvr/actions/ActionSet.h>
#include <vsgvr/actions/ActionPoseBinding.h>
#include <openxr/openxr_platform.h>

#include "../../../interaction_locomotion/game.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vsgnative", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "vsgnative", __VA_ARGS__))

//
// App state
//
struct AppData
{
    struct android_app* app;
    vsg::ref_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsgAndroid::Android_Window> window;
    vsg::ref_ptr<vsgvr::Viewer> vr;
    std::unique_ptr<Game> game;
};

//
// Init the vsg viewer and load assets
//
static int vsg_init(struct AppData* appData)
{
    // Initialise OpenXR
    // TODO: At the moment traits must be configured up front, exceptions will be thrown if these can't be satisfied
    //       This should be improved in the future, at least to query what form factors are available.
    // TODO: Some parameters on xrTraits are non-functional at the moment
    auto xrTraits = vsgvr::AndroidTraits::create();
    xrTraits->formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    xrTraits->viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    xrTraits->vm = appData->app->activity->vm;
    xrTraits->activity = appData->app->activity->clazz;

    // Initialise OpenXR, and retrieve vulkan requirements
    // OpenXR will require certain vulkan versions, along with a specific physical device
    // Use a desktop window to create the instance, and select the correct device
    auto xrInstance = vsgvr::Instance::create(xrTraits);
    auto xrVulkanReqs = vsgvr::GraphicsBindingVulkan::getVulkanRequirements(xrInstance);


    appData->viewer = vsg::Viewer::create();
    // setup VSG traits, to provide a compatible Vulkan instance
    auto desktopTraits = vsg::WindowTraits::create();
    desktopTraits->setValue("nativeWindow", appData->app->window);
    desktopTraits->width = ANativeWindow_getWidth(appData->app->window);
    desktopTraits->height = ANativeWindow_getHeight(appData->app->window);
    // Validate the Vulkan instance, and re-create the instance with the required extensions
    if (desktopTraits->vulkanVersion < xrVulkanReqs.minVersion)
    {
        LOGW("Vulkan API too low for OpenXR.");
        return 1;
    }
    // Add any other requirements of OpenXR to the window traits, this mostly includes memory sharing and synchronisation extensions
    for (auto& ext : xrVulkanReqs.instanceExtensions)
    {
        if (std::find(desktopTraits->instanceExtensionNames.begin(), desktopTraits->instanceExtensionNames.end(), ext) == desktopTraits->instanceExtensionNames.end() ) {
            desktopTraits->instanceExtensionNames.push_back(ext.c_str());
        }
    }
    for (auto& ext : xrVulkanReqs.deviceExtensions)
    {
        if (std::find(desktopTraits->deviceExtensionNames.begin(), desktopTraits->deviceExtensionNames.end(), ext) == desktopTraits->deviceExtensionNames.end()) {
            desktopTraits->deviceExtensionNames.push_back(ext.c_str());
        }
    }

    desktopTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    // create a window using the ANativeWindow passed via traits
    vsg::ref_ptr<vsg::Window> window;
    try {
        window = vsg::Window::create(desktopTraits);
    } catch( const std::bad_any_cast& e ) {
        LOGW("Error: Failed to create a VSG Window due to std::bad_any_cast - The application was linked to the C++ STL incorrectly");
        throw;
    }
    if (!window)
    {
        LOGW("Error: Could not create window a VSG window.");
        return 1;
    }
    // cast the window to an android window so we can pass it events
    appData->window = window.cast<vsgAndroid::Android_Window>();

    // Attach the window to the viewer
    // Android: The window is present, but will not be rendered by the Game class
    appData->viewer->addWindow(window);

    // Ensure the correct physical device is selected
    // Technically the desktop window and HMD could use different devices, but for simplicity OpenXR is configured to use the same device as the desktop
    auto vkInstance = appData->window ->getOrCreateInstance();
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
        // std::cout << "Unable to select physical device, as required by OpenXR" << std::endl;
        return EXIT_FAILURE;
    }
    appData->window ->setPhysicalDevice(physicalDevice);

    // Bind OpenXR to the Device
    appData->window ->getOrCreateSurface();
    auto vkDevice = appData->window ->getOrCreateDevice();
    auto graphicsBinding = vsgvr::GraphicsBindingVulkan::create(vkInstance, physicalDevice, vkDevice, physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT), 0);

    // Set up a renderer to OpenXR, similar to a vsg::Viewer
    appData->vr = vsgvr::Viewer::create(xrInstance, xrTraits, graphicsBinding);

    appData->game.reset(new Game(xrInstance, appData->vr, appData->viewer, false));

    return 0;
}

static void vsg_term(struct AppData* appData)
{
    appData->viewer = nullptr;
    appData->vr = nullptr;
    appData->window = nullptr;
}

//
// Render a frame
//
static void vsg_frame(struct AppData* appData)
{
    if (!appData->vr.valid()) {
        // no viewer.
        return;
    }

    if(appData->game->shouldExit)
    {
        vsg_term(appData);
        return;
    }
    appData->game->frame();
}

//
// Handle input event
//
static int32_t android_handleinput(struct android_app* app, AInputEvent* event)
{
    struct AppData* appData = (struct AppData*)app->userData;
    if(appData->window.valid()) return 0;
    // pass the event to the vsg android window
    return appData->window->handleAndroidInputEvent(event) ? 1 : 0;
}

//
// Handle main commands
//
static void android_handlecmd(struct android_app* app, int32_t cmd)
{
    struct AppData* appData = (struct AppData*)app->userData;
    if(appData == nullptr) return;

    switch (cmd)
    {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL)
            {
                vsg_init(appData);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            vsg_term(appData);
            break;
    }
}

//
// Main entry point for the application
//
void android_main(struct android_app* app)
{
    XrInstanceCreateInfoAndroidKHR createInfo;
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR;
    createInfo.next = nullptr;
    // applicationVM is a pointer to the JNI’s opaque JavaVM structure, cast to a void pointer
    createInfo.applicationVM = static_cast<void*>(app->activity->vm);
    // applicationActivity is a JNI reference to an android.app.Activity that will drive
    // the session lifecycle of this instance, cast to a void pointer.
    createInfo.applicationActivity = static_cast<void*>(app->activity->clazz);


    struct AppData appData;

    memset(&appData, 0, sizeof(AppData));
    app->userData = &appData;
    app->onAppCmd = android_handlecmd;
    app->onInputEvent = android_handleinput;
    appData.app = app;

    // Used to poll the events in the main loop
    int events;
    android_poll_source* source;

    // main loop
    do
    {
        // poll events
        if (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0)
        {
            if (source != NULL) source->process(app, source);
        }

        // render if vulkan is ready and we are animating
        if (appData.viewer.valid()) {
            vsg_frame(&appData);
        }
    } while (app->destroyRequested == 0);
}
