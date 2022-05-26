
#include <vsgvr/openxr/OpenXRInstance.h>
#include <vsg/core/Exception.h>

#include <openxr/openxr_reflection.h>
#include "OpenXRMacros.cpp"

#include <iostream>

#include <vsg/traversals/ComputeBounds.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/viewer/View.h>
#include <vsg/ui/UIEvent.h>

using namespace vsg;

namespace vsgvr
{
  OpenXRInstance::OpenXRInstance(OpenXrTraits xrTraits, OpenXrVulkanTraits vkTraits)
  {
    createInstance();
    getSystem();
    validateTraits();
    getViewConfiguration();
    createGraphicsBinding();
    createSession();
  }

  OpenXRInstance::~OpenXRInstance()
  {
    shutdownAll();
  }

  void OpenXRInstance::shutdownAll()
  {
    // TODO: May need to wait - Can't destroy swapchain in session until idle
    // vkDeviceWaitIdle(_graphicsBinding->getVkDevice()->getDevice());

    if (_session) destroySession();
    if (_graphicsBinding) destroyGraphicsBinding();
    _system = 0;
    _systemProperties = XrSystemProperties();
    if (_instance) destroyInstance();
  }

  auto OpenXRInstance::pollEvents() -> PollEventsResult
  {
    if (!_instance) return PollEventsResult::NotRunning;

    _eventHandler.pollEvents(this, _session);

    if (!_session) return PollEventsResult::NotRunning;

    switch (_session->getSessionState())
    {
    case XR_SESSION_STATE_IDLE:
      return PollEventsResult::RuntimeIdle;
    case XR_SESSION_STATE_READY:
      if (!_session->getSessionRunning())
      {
        // Begin session. Transition to synchronised after a few begin/end frames
        _session->beginSession(_xrTraits.viewConfigurationType);
      }
      return PollEventsResult::RunningDontRender;
    case XR_SESSION_STATE_SYNCHRONIZED:
      return PollEventsResult::RunningDontRender;
    case XR_SESSION_STATE_VISIBLE:
    case XR_SESSION_STATE_FOCUSED:
      return PollEventsResult::RunningDoRender;
    case XR_SESSION_STATE_STOPPING:
      std::cerr << "Ending Session" << std::endl;
      _session->endSession();
      return PollEventsResult::NotRunning;
    case XR_SESSION_STATE_LOSS_PENDING:
      std::cerr << "State Loss" << std::endl;
      // TODO: Display connection lost. Re-init may be possible later
      [[fallthrough]];
    case XR_SESSION_STATE_EXITING:
      if (_session->getSessionRunning())
      {
        _session->endSession();
      }
      shutdownAll();
      return PollEventsResult::Exit;
    case XR_SESSION_STATE_UNKNOWN:
    default:
      break;
    }
  }

  auto OpenXRInstance::advanceToNextFrame() -> RenderStatus
  {
    // Viewer::acquireNextFrame
    _frameState = XrFrameState();
    _frameState.type = XR_TYPE_FRAME_STATE;
    _frameState.next = nullptr;

    // TODO: Return statuses - session/instance loss fairly likely here
    xr_check(xrWaitFrame(_session->getSession(), nullptr, &_frameState));
    xr_check(xrBeginFrame(_session->getSession(), nullptr));
    
    auto image = _session->getSwapchain()->acquireImage(_frameImageIndexHACK);

    XrDuration timeoutNs = 20000000;
    auto waitSuccess = _session->getSwapchain()->waitImage(timeoutNs);

    // Viewer::advanceToNextFrame
    // create FrameStamp for frame
    // TODO: originally vsg::clock::now()
    // Should use XR extensions below to convert to wall-time
    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrTime
    // XR_KHR_win32_convert_performance_counter_time or XR_KHR_convert_timespec_time.
    vsg::clock::time_point t(std::chrono::nanoseconds(_frameState.predictedDisplayTime));
    if (!_frameStamp)
    {
      // first frame, initialize to frame count and indices to 0
      _frameStamp = FrameStamp::create(t, 0);
    }
    else
    {
      // after first frame so increment frame count and indices
      _frameStamp = FrameStamp::create(t, _frameStamp->frameCount + 1);
    }
    for (auto& task : recordAndSubmitTasks)
    {
      task->advance();
    }

    // create an event for the new frame.
    // _events.emplace_back(new FrameEvent(_frameStamp));

    // Inform the application whether it should actually render
    // The frame must be completed regardless to remain synchronised with OpenXR
    return RenderStatus{ waitSuccess ? static_cast<bool>(_frameState.shouldRender) : false, waitSuccess };
  }

  void OpenXRInstance::update()
  {
    for (auto& task : recordAndSubmitTasks)
    {
      if (task->databasePager)
      {
        task->databasePager->updateSceneGraph(_frameStamp);
      }
    }
  }

  void OpenXRInstance::recordAndSubmit()
  {
    // reset connected ExecuteCommands
    for (auto& recordAndSubmitTask : recordAndSubmitTasks)
    {
      for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
      {
        commandGraph->reset();
      }
    }

    for (auto& recordAndSubmitTask : recordAndSubmitTasks)
    {
      recordAndSubmitTask->submit(_frameStamp);
    }
  }

  void OpenXRInstance::releaseFrame(RenderStatus s)
  {
    if (s.waitSuccess)
    {
      _session->getSwapchain()->releaseImage();
    }

    auto info = XrFrameEndInfo();
    info.type = XR_TYPE_FRAME_END_INFO;
    info.next = nullptr;
    info.displayTime = _frameState.predictedDisplayTime; // TODO: Predicted, or 'now'?
    info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    info.layerCount = 0;
    info.layers = nullptr; // TODO

    xr_check(xrEndFrame(_session->getSession(), &info));
  }

  vsg::ref_ptr<vsg::Camera>
    OpenXRInstance::createCameraForScene(vsg::ref_ptr<vsg::Node> scene,
      const VkExtent2D& extent) {
    // Create an initial camera - Both the desktop and hmd cameras are intialised
    // like this but their parameters will be updated each frame based on the
    // hmd's pose/matrices
    vsg::ComputeBounds computeBounds;
    scene->accept(computeBounds);
    vsg::dvec3 centre =
      (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    double radius =
      vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
    double nearFarRatio = 0.001;

    // set up the camera
    auto lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, 0.0, radius * 3.5),
      centre, vsg::dvec3(0.0, 1.0, 0.0));

    auto perspective = vsg::Perspective::create(
      30.0,
      static_cast<double>(extent.width) / static_cast<double>(extent.height),
      nearFarRatio * radius, radius * 4.5);

    return vsg::Camera::create(perspective, lookAt,
      vsg::ViewportState::create(extent));
  }

  std::vector<vsg::ref_ptr<vsg::CommandGraph>>
    OpenXRInstance::createCommandGraphsForView(vsg::ref_ptr<vsg::Node> vsg_scene) {
    
    auto numImages = _viewConfigurationViews.size();

    vsg::ref_ptr<vsg::CompileTraversal> compile = vsg::CompileTraversal::create(_graphicsBinding->getVkDevice());

    // TODO: Multi-view, better size selection - To match size used in swapchain
    VkExtent2D hmdExtent{
      _viewConfigurationViews[0].recommendedImageRectWidth,
      _viewConfigurationViews[0].recommendedImageRectHeight,
    };

    // vsg::createCommandGraphForView
    auto hmdCommandGraph = CommandGraph::create(_graphicsBinding->getVkDevice(), 
                            _graphicsBinding->getVkPhysicalDevice()->getQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT));

    // for (auto imgI = 0u; imgI < numImages; ++imgI) {
    auto imgI = 0;
    {
      auto camera = createCameraForScene(vsg_scene, hmdExtent);
      // auto renderGraph = vsg::createRenderGraphForView({}, camera, vsg_scene);
      // set up the view
      auto view = View::create(camera);
      // if (assignHeadlight) view->addChild(createHeadlight());
      if (vsg_scene) view->addChild(vsg_scene);

      // set up the render graph
      // TODO: RenderGraph can either render to a single framebuffer, or will query the correct
      // framebuffer from the Window, based on Window::ImageIndex.
      // This is very wrong, but just set framebuffer 0 for now to get something to compile.
      // Expect RenderGraph will need updating to support multi-image framebuffers, or need
      // to rework the Window class to allow an OpenXR version to exist
      // (Split the VkSurface specific parts out to a graphics attachment setup, similar to OpenXR's graphicsBinding)
      auto renderGraph = RenderGraph::create();

      // HACK HACK HACK
      // Example app recreates command graph each frame, to do this..
      if (_session)
      {
        renderGraph->framebuffer = _session->frames()[_frameImageIndexHACK].framebuffer;
        renderGraph->previous_extent = _session->getSwapchain()->getExtent();
        renderGraph->renderArea.offset = {0, 0};
        renderGraph->renderArea.extent = _session->getSwapchain()->getExtent();

        // TODO: Will need to pass correct values, assume it comes from the window traits / equivalent?
        renderGraph->setClearValues();
      }
      // HACK HACK HACK
      
      // renderGraph->contents = contents;
      

      hmdCommandGraph->addChild(renderGraph);
    }

    return { hmdCommandGraph };
  }

  void OpenXRInstance::assignRecordAndSubmitTaskAndPresentation(std::vector<vsg::ref_ptr<vsg::CommandGraph>> in_commandGraphs)
  {
    struct DeviceQueueFamily
    {
      Device* device = nullptr;
      int queueFamily = -1;
      int presentFamily = -1;

      bool operator<(const DeviceQueueFamily& rhs) const
      {
        if (device < rhs.device) return true;
        if (device > rhs.device) return false;
        if (queueFamily < rhs.queueFamily) return true;
        if (queueFamily > rhs.queueFamily) return false;
        return presentFamily < rhs.presentFamily;
      }
    };

    // place the input CommandGraphs into separate groups associated with each device and queue family combination
    std::map<DeviceQueueFamily, CommandGraphs> deviceCommandGraphsMap;
    for (auto& commandGraph : in_commandGraphs)
    {
      deviceCommandGraphsMap[DeviceQueueFamily{ commandGraph->device.get(), commandGraph->queueFamily, commandGraph->presentFamily }].emplace_back(commandGraph);
    }

    // create the required RecordAndSubmitTask and any Presentation objects that are required for each set of CommandGraphs
    for (auto& [deviceQueueFamily, commandGraphs] : deviceCommandGraphsMap)
    {
      // make sure the secondary CommandGraphs appear first in the commandGraphs list so they are filled in first
      CommandGraphs primary_commandGraphs;
      CommandGraphs secondary_commandGraphs;
      for (auto& commandGraph : commandGraphs)
      {
        if (commandGraph->level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
          primary_commandGraphs.emplace_back(commandGraph);
        else
          secondary_commandGraphs.emplace_back(commandGraph);
      }
      if (!secondary_commandGraphs.empty())
      {
        commandGraphs = secondary_commandGraphs;
        commandGraphs.insert(commandGraphs.end(), primary_commandGraphs.begin(), primary_commandGraphs.end());
      }

      uint32_t numBuffers = 3; // TODO: Needs to relate to OpenXR swapchain surely

      auto device = deviceQueueFamily.device;
      // with don't have a presentFamily so this set of commandGraphs aren't associated with a window
      // set up Submission with CommandBuffer and signals
      auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers); 
      recordAndSubmitTask->commandGraphs = commandGraphs;
      recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
      recordAndSubmitTasks.emplace_back(recordAndSubmitTask);
    }
  }

  void OpenXRInstance::compile(ref_ptr<ResourceHints> hints)
  {
    if (recordAndSubmitTasks.empty())
    {
      return;
    }

    bool containsPagedLOD = false;
    ref_ptr<DatabasePager> databasePager;

    struct DeviceResources
    {
      CollectResourceRequirements collectResources;
      vsg::ref_ptr<vsg::CompileTraversal> compile;
    };

    // find which devices are available and the resources required for then,
    using DeviceResourceMap = std::map<ref_ptr<vsg::Device>, DeviceResources>;
    DeviceResourceMap deviceResourceMap;
    for (auto& task : recordAndSubmitTasks)
    {
      for (auto& commandGraph : task->commandGraphs)
      {
        auto& deviceResources = deviceResourceMap[commandGraph->device];
        commandGraph->accept(deviceResources.collectResources);
      }

      if (task->databasePager && !databasePager) databasePager = task->databasePager;
    }

    // allocate DescriptorPool for each Device
    ResourceRequirements::Views views;
    for (auto& [device, deviceResource] : deviceResourceMap)
    {
      auto& collectResources = deviceResource.collectResources;
      auto& resourceRequirements = collectResources.requirements;

      if (hints) hints->accept(collectResources);

      views.insert(resourceRequirements.views.begin(), resourceRequirements.views.end());

      if (resourceRequirements.containsPagedLOD) containsPagedLOD = true;

      auto physicalDevice = device->getPhysicalDevice();

      auto maxSets = resourceRequirements.computeNumDescriptorSets();
      auto descriptorPoolSizes = resourceRequirements.computeDescriptorPoolSizes();

      auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT); // TODO : could we just use transfer bit?

      deviceResource.compile = CompileTraversal::create(device, resourceRequirements);
      deviceResource.compile->overrideMask = 0xffffffff;

      // CT TODO need to reorganize this whole section
      for (auto& context : deviceResource.compile->contexts)
      {
        context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        context->graphicsQueue = device->getQueue(queueFamily);
        if (descriptorPoolSizes.size() > 0) context->descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);
      }
    }

    // assign the viewID's to each View
    for (auto& [const_view, binDetails] : views)
    {
      auto view = const_cast<View*>(const_view);
      for (auto& binNumber : binDetails.indices)
      {
        bool binNumberMatched = false;
        for (auto& bin : view->bins)
        {
          if (bin->binNumber == binNumber)
          {
            binNumberMatched = true;
          }
        }
        if (!binNumberMatched)
        {
          Bin::SortOrder sortOrder = (binNumber < 0) ? Bin::ASCENDING : ((binNumber == 0) ? Bin::NO_SORT : Bin::DESCENDING);
          view->bins.push_back(Bin::create(binNumber, sortOrder));
        }
      }
    }

    if (containsPagedLOD && !databasePager) databasePager = DatabasePager::create();

    // create the Vulkan objects
    for (auto& task : recordAndSubmitTasks)
    {
      std::set<Device*> devices;

      bool task_containsPagedLOD = false;

      for (auto& commandGraph : task->commandGraphs)
      {
        if (commandGraph->device) devices.insert(commandGraph->device);

        auto& deviceResource = deviceResourceMap[commandGraph->device];
        auto& resourceRequirements = deviceResource.collectResources.requirements;
        commandGraph->maxSlot = resourceRequirements.maxSlot;
        commandGraph->accept(*deviceResource.compile);

        if (resourceRequirements.containsPagedLOD) task_containsPagedLOD = true;
      }

      if (task_containsPagedLOD)
      {
        if (!task->databasePager) task->databasePager = databasePager;
      }

      if (task->databasePager)
      {
        // crude hack for taking first device as the one for the DatabasePager to compile resources for.
        for (auto& commandGraph : task->commandGraphs)
        {
          auto& deviceResource = deviceResourceMap[commandGraph->device];
          task->databasePager->compileTraversal = deviceResource.compile;
          break;
        }
      }
    }

    // record any transfer commands
    for (auto& dp : deviceResourceMap)
    {
      dp.second.compile->record();
    }

    // wait for the transfers to complete
    for (auto& dp : deviceResourceMap)
    {
      dp.second.compile->waitForCompletion();
    }

    // start any DatabasePagers
    for (auto& task : recordAndSubmitTasks)
    {
      if (task->databasePager)
      {
        task->databasePager->start();
      }
    }
  }

  void OpenXRInstance::onEventInstanceLossPending(const XrEventDataInstanceLossPending& event)
  {
    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrEventDataInstanceLossPending
    // TODO: This indicates the overall runtime is about to become unavailable. Given that encountering
    //       the subsequent XR_ERROR_RUNTIME_UNAVAILABLE when re-creating the instance isn't handled, 
    //       just throw an exception.
    throw Exception({ "OpenXR: Instance loss pending" });
  }

  void OpenXRInstance::createInstance()
  {
    if (_instance)
    {
      throw Exception({ "OpenXRInstance: Instance already initialised" });
    }
    std::vector<const char*> extensions = {
        "XR_KHR_vulkan_enable",
        "XR_KHR_vulkan_enable2" };
    for (auto& e : _xrTraits.xrExtensions)
      extensions.push_back(e.c_str());

    std::vector<const char*> layers = {};
    for (auto& l : _xrTraits.xrLayers)
      layers.push_back(l.c_str());

    XrInstanceCreateInfo info;
    info.type = XR_TYPE_INSTANCE_CREATE_INFO;
    info.next = nullptr;
    info.createFlags = 0;
    info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
    info.enabledApiLayerNames = layers.empty() ? nullptr : layers.data();
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.enabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

    strncpy(info.applicationInfo.applicationName, _xrTraits.applicationName.c_str(), std::min(static_cast<int>(_xrTraits.applicationName.size() + 1), XR_MAX_APPLICATION_NAME_SIZE));
    info.applicationInfo.applicationVersion = _xrTraits.applicationVersion;
    strncpy(info.applicationInfo.engineName, _xrTraits.engineName.c_str(), std::min(static_cast<int>(_xrTraits.engineName.size() + 1), XR_MAX_ENGINE_NAME_SIZE));
    info.applicationInfo.apiVersion = _xrTraits.apiVersion;
    info.applicationInfo.engineVersion = _xrTraits.engineVersion;

    xr_check(xrCreateInstance(&info, &_instance), "Failed to create XR Instance");
    xr_check(xrGetInstanceProperties(_instance, &_instanceProperties), "Failed to get XR Instance properties");
  }
  void OpenXRInstance::destroyInstance()
  {
    if (!_instance)
    {
      throw Exception({ "OpenXRInstance: Instance not initialised" });
    }
    xr_check(xrDestroyInstance(_instance), "Failed to destroy XR Instance");
    _instance = nullptr;
  }

  void OpenXRInstance::getSystem()
  {
    XrSystemGetInfo info;
    info.type = XR_TYPE_SYSTEM_GET_INFO;
    info.next = nullptr;
    info.formFactor = _xrTraits.formFactor;

    XrSystemId system;
    xr_check(xrGetSystem(_instance, &info, &_system), "Failed to get OpenXR system");

    XrSystemProperties props;
    props.type = XR_TYPE_SYSTEM_PROPERTIES;
    props.next = nullptr;
    props.graphicsProperties = { 0 };
    props.trackingProperties = { 0 };
    xr_check(xrGetSystemProperties(_instance, _system, &props), "Failed to get OpenXR system properties");
  }
  void OpenXRInstance::validateTraits()
  {
    // Form factor validated by getSystem

    // View configuration type
    {
      std::vector<XrViewConfigurationType> types;
      uint32_t count = 0;
      xr_check(xrEnumerateViewConfigurations(_instance, _system, 0, &count, nullptr));
      types.resize(count);
      xr_check(xrEnumerateViewConfigurations(_instance, _system, types.size(), &count, types.data()));
      if (std::find(types.begin(), types.end(), _xrTraits.viewConfigurationType) == types.end())
      {
        throw Exception({ "View configuration type not supported" });
      }
    }

    // Environment blend mode
    {
      std::vector<XrEnvironmentBlendMode> modes;
      uint32_t count = 0;
      xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, _xrTraits.viewConfigurationType, 0, &count, nullptr));
      modes.resize(count);
      xr_check(xrEnumerateEnvironmentBlendModes(_instance, _system, _xrTraits.viewConfigurationType, modes.size(), &count, modes.data()));
      if (std::find(modes.begin(), modes.end(), _xrTraits.environmentBlendMode) == modes.end())
      {
        throw Exception({ "Environment blend mode not supported" });
      }
    }
  }
  void OpenXRInstance::getViewConfiguration() {
    xr_check(xrGetViewConfigurationProperties(_instance, _system, _xrTraits.viewConfigurationType, &_viewConfigurationProperties));
    uint32_t count = 0;
    xr_check(xrEnumerateViewConfigurationViews(_instance, _system, _xrTraits.viewConfigurationType, 0, &count, nullptr));
    _viewConfigurationViews.resize(count);
    xr_check(xrEnumerateViewConfigurationViews(_instance, _system, _xrTraits.viewConfigurationType, _viewConfigurationViews.size(), &count, _viewConfigurationViews.data()));
  }

  void OpenXRInstance::createGraphicsBinding() {
    if (_graphicsBinding) {
      throw Exception({ "openXRInstance: Graphics binding already initialised" });
    }

    _graphicsBinding = OpenXRGraphicsBindingVulkan2::create(_instance, _system, _xrTraits, _vkTraits);
  }

  void OpenXRInstance::destroyGraphicsBinding() {
    if (!_graphicsBinding) {
      throw Exception({ "openXRInstance: Graphics binding not initialised" });
    }

    _graphicsBinding = 0;
  }

  void OpenXRInstance::createSession() {
    if (_session) {
      throw Exception({ "openXRInstance: Session already initialised" });
    }

    _session = OpenXRSession::create(_instance, _system, _graphicsBinding, _xrTraits.swapchainFormat, _viewConfigurationViews);
  }

  void OpenXRInstance::destroySession() {
    if (!_session) {
      throw Exception({ "openXRInstance: Session not initialised" });
    }

    _session = 0;
  }
}
