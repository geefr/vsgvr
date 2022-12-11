
#include <initializer_list>
#include <cstdlib>
#include <cstring>
#include <jni.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <vsg/all.h>
#include <vsg/platform/android/Android_Window.h>

#include <vsgvr/xr/OpenXRInstance.h>
#include <vsgvr/xr/OpenXRGraphicsBindingVulkan.h>
#include <vsgvr/xr/OpenXRViewMatrix.h>
#include <vsgvr/xr/OpenXRAndroidTraits.h>
#include <vsgvr/app/OpenXRViewer.h>
#include <vsgvr/actions/OpenXRActionSet.h>
#include <vsgvr/actions/OpenXRActionPoseBinding.h>
#include <openxr/openxr_platform.h>

#include "../../../../models/controller/controller.cpp"
#include "../../../../models/controller/controller2.cpp"
#include "../../../../models/world/world.cpp"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vsgnative", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "vsgnative", __VA_ARGS__))

//
// App state
//
struct AppData
{
    struct android_app* app;

    vsg::ref_ptr<vsg::WindowTraits> traits;
    vsg::ref_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsgvr::OpenXRViewer> vr;
    vsg::ref_ptr<vsgvr::OpenXRActionPoseBinding> leftHandPoseBinding;
    vsg::ref_ptr<vsgvr::OpenXRAction> leftHandButtonPress;
    vsg::ref_ptr<vsgvr::OpenXRActionPoseBinding> rightHandPoseBinding;
    vsg::ref_ptr<vsgvr::OpenXRAction> rightHandButtonPress;
    vsg::ref_ptr<vsg::MatrixTransform> controllerNodeLeft;
    vsg::ref_ptr<vsg::MatrixTransform> controllerNodeRight;
    vsg::ref_ptr<vsgAndroid::Android_Window> window;

    double leftRot = 0.0f;
    double rightRot = 0.0f;
};

//
// Init the vsg viewer and load assets
//
static int vsg_init(struct AppData* appData)
{
    auto vsg_scene = vsg::Group::create();
    vsg_scene->addChild(world());
    appData->controllerNodeLeft = controller();
    vsg_scene->addChild(appData->controllerNodeLeft);
    appData->controllerNodeRight = controller2();
    vsg_scene->addChild(appData->controllerNodeRight);

    // Initialise OpenXR
    // TODO: At the moment traits must be configured up front, exceptions will be thrown if these can't be satisfied
    //       This should be improved in the future, at least to query what form factors are available.
    // TODO: Some parameters on xrTraits are non-functional at the moment
    auto xrTraits = vsgvr::OpenXRAndroidTraits::create();
    xrTraits->formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    xrTraits->viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    xrTraits->vm = appData->app->activity->vm;
    xrTraits->activity = appData->app->activity->clazz;

    // Initialise OpenXR, and retrieve vulkan requirements
    // OpenXR will require certain vulkan versions, along with a specific physical device
    // Use a desktop window to create the instance, and select the correct device
    auto xrInstance = vsgvr::OpenXRInstance::create(xrTraits);
    auto xrVulkanReqs = vsgvr::OpenXRGraphicsBindingVulkan::getVulkanRequirements(xrInstance);


    appData->viewer = vsg::Viewer::create();
    // setup VSG traits, to provide a compatible Vulkan instance
    appData->traits = vsg::WindowTraits::create();
    appData->traits->setValue("nativeWindow", appData->app->window);
    appData->traits->width = ANativeWindow_getWidth(appData->app->window);
    appData->traits->height = ANativeWindow_getHeight(appData->app->window);
    // Validate the Vulkan instance, and re-create the instance with the required extensions
    if (appData->traits->vulkanVersion < xrVulkanReqs.minVersion)
    {
        LOGW("Vulkan API too low for OpenXR.");
        return 1;
    }
    // Add any other requirements of OpenXR to the window traits, this mostly includes memory sharing and synchronisation extensions
    for (auto& ext : xrVulkanReqs.instanceExtensions)
    {
        if (std::find(appData->traits->instanceExtensionNames.begin(), appData->traits->instanceExtensionNames.end(), ext) == appData->traits->instanceExtensionNames.end() ) {
            appData->traits->instanceExtensionNames.push_back(ext.c_str());
        }
    }
    for (auto& ext : xrVulkanReqs.deviceExtensions)
    {
        if (std::find(appData->traits->deviceExtensionNames.begin(), appData->traits->deviceExtensionNames.end(), ext) == appData->traits->deviceExtensionNames.end()) {
            appData->traits->deviceExtensionNames.push_back(ext.c_str());
        }
    }

    appData->traits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    // create a window using the ANativeWindow passed via traits
    vsg::ref_ptr<vsg::Window> window;
    try {
        window = vsg::Window::create(appData->traits);
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

    // attach the window to the viewer
    appData->viewer->addWindow(window);

    // Ensure the correct physical device is selected
    // Technically the desktop window and HMD could use different devices, but for simplicity OpenXR is configured to use the same device as the desktop
    auto vkInstance = appData->window ->getOrCreateInstance();
    VkPhysicalDevice xrRequiredDevice = vsgvr::OpenXRGraphicsBindingVulkan::getVulkanDeviceRequirements(xrInstance, vkInstance, xrVulkanReqs);
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
    auto graphicsBinding = vsgvr::OpenXRGraphicsBindingVulkan::create(vkInstance, physicalDevice, vkDevice, physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT), 0);

    // Set up a renderer to OpenXR, similar to a vsg::Viewer
    auto vr = vsgvr::OpenXRViewer::create(xrInstance, xrTraits, graphicsBinding);

    // Create CommandGraphs to render the scene to the HMD
    std::vector<vsg::ref_ptr<vsg::Camera>> xrCameras;
    // TODO: This only really exists because vsg::createCommandGraphForView requires
    // a Window instance. Other than some possible improvements later, it could use the same code as vsg
    // OpenXR rendering may use one or more command graphs, as decided by the viewer
    // (TODO: At the moment only a single CommandGraph will be used, even if there's multiple XR views)
    auto xrCommandGraphs = vr->createCommandGraphsForView(vsg_scene, xrCameras, false);
    // TODO: This is almost identical to Viewer::assignRecordAndSubmitTaskAndPresentation - The only difference is
    // that OpenXRViewer doesn't have presentation - If presentation was abstracted we could avoid awkward duplication here
    vr->assignRecordAndSubmitTask(xrCommandGraphs);
    // TODO: This is identical to Viewer::compile, except CompileManager requires a child class of Viewer
    // OpenXRViewer can't be a child class of Viewer yet (Think this was due to the assumption that a Window/Viewer has presentation / A Surface)
    vr->compile();
    // Configure OpenXR action sets and pose bindings - These allow elements of the OpenXR device tree to be located and tracked in space,
    // along with binding the OpenXR input subsystem through to usable actions.

    auto baseActionSet = vsgvr::OpenXRActionSet::create(xrInstance, "gameplay", "Gameplay");
    appData->leftHandPoseBinding = vsgvr::OpenXRActionPoseBinding::create(baseActionSet, "left_hand", "Left Hand");
    appData->leftHandButtonPress = vsgvr::OpenXRAction::create(baseActionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT, "left_hand_select", "Left Hand Select");

    appData->rightHandPoseBinding = vsgvr::OpenXRActionPoseBinding::create(baseActionSet, "right_hand", "Right Hand");
    appData->rightHandButtonPress = vsgvr::OpenXRAction::create(baseActionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT, "right_hand_select", "Right Hand Select");
    baseActionSet->actions = {
            appData->leftHandPoseBinding,
            appData->leftHandButtonPress,
            appData->rightHandPoseBinding,
            appData->rightHandButtonPress,
    };

    // Ask OpenXR to bind the actions in the set
    // Note: While the Khronos simple controller profile works, it's generally
    //       best to use the platform-specific bindings if you know what you're running on (i.e. quest)
    //       For full portability suggestInteractionBindings should be called for all configurations you've tested on.
    if (baseActionSet->suggestInteractionBindings(xrInstance, "/interaction_profiles/oculus/touch_controller", {
            {appData->rightHandPoseBinding, "/user/hand/right/input/aim/pose"},
            {appData->rightHandButtonPress, "/user/hand/right/input/a/touch"},
            {appData->leftHandPoseBinding, "/user/hand/left/input/aim/pose"},
            {appData->leftHandButtonPress, "/user/hand/left/input/x/touch"},
        })
    )
//    if (baseActionSet->suggestInteractionBindings(xrInstance, "/interaction_profiles/khr/simple_controller", {
//            {appData->leftHandPoseBinding, "/user/hand/left/input/aim/pose"},
//            {appData->leftHandButtonPress, "/user/hand/left/input/select/click"},
//            {appData->rightHandPoseBinding, "/user/hand/right/input/aim/pose"},
//            {appData->rightHandButtonPress, "/user/hand/right/input/select/click"},
//    }))
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
        return 1;
    }

    appData->vr = vr;

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

    // OpenXR events must be checked first
    auto pol = appData->vr->pollEvents();
    if( pol == vsgvr::OpenXRViewer::PollEventsResult::Exit )
    {
        // User exited through VR overlay / XR runtime
        return;
    }

    if( pol == vsgvr::OpenXRViewer::PollEventsResult::NotRunning)
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
    if (appData->leftHandPoseBinding->getTransformValid())
    {
        appData->controllerNodeLeft->matrix = appData->leftHandPoseBinding->getTransform();
    }
    if (appData->rightHandPoseBinding->getTransformValid())
    {
        appData->controllerNodeRight->matrix = appData->rightHandPoseBinding->getTransform();
    }
    // Quick input test - When triggers pressed (select action binding) make controllers spin
    // Note coordinate space of controllers - Z is forward
    if(appData->leftHandButtonPress->getStateValid())
    {
        auto lState = appData->leftHandButtonPress->getStateBool();
        if (lState.isActive && lState.currentState)
        {
            appData->leftRot += 0.1;
        }
    }
    if(appData->rightHandButtonPress->getStateValid())
    {
        auto rState = appData->rightHandPoseBinding->getStateBool();
        if (rState.isActive && rState.currentState) {
            appData->rightRot += 0.1;
        }
    }
    appData->controllerNodeLeft->matrix = appData->controllerNodeLeft->matrix * vsg::rotate(appData->leftRot, { 0.0, 0.0, 1.0 });
    appData->controllerNodeRight->matrix = appData->controllerNodeRight->matrix * vsg::rotate(appData->rightRot, { 0.0, 0.0, 1.0 });

    if (appData->vr->advanceToNextFrame())
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
            appData->vr->recordAndSubmit(); // Render XR frame
        }
    }

    // End the frame, and present to user
    // Frames must be explicitly released, even if the previous advanceToNextFrame returned false (PollEventsResult::RunningDontRender)
    appData->vr->releaseFrame();
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
