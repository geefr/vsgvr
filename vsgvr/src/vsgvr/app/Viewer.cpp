/*
Copyright(c) 2022 Gareth Francis

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vsgvr/app/Viewer.h>
#include <vsgvr/xr/ViewMatrix.h>
#include <vsgvr/xr/ProjectionMatrix.h>

#include <vsgvr/actions/ActionPoseBinding.h>

#include <vsg/core/Exception.h>

#include <openxr/openxr_reflection.h>
#include "../xr/Macros.cpp"

#include <iostream>

#include <vsg/utils/ComputeBounds.h>
#include <vsg/app/CompileTraversal.h>
#include <vsg/app/View.h>
#include <vsg/ui/UIEvent.h>

using namespace vsg;

namespace vsgvr
{
  Viewer::Viewer(vsg::ref_ptr<Instance> xrInstance, vsg::ref_ptr<Traits> xrTraits, vsg::ref_ptr<GraphicsBindingVulkan> graphicsBinding)
    : _instance(xrInstance)
    , _xrTraits(xrTraits)
    , _graphicsBinding(graphicsBinding)
  {
    getViewConfiguration();
    createSession();
  }

  Viewer::~Viewer()
  {
    shutdownAll();
  }

  void Viewer::shutdownAll()
  {
    if (_session) destroySession();
  }

  auto Viewer::pollEvents() -> PollEventsResult
  {
    if (!_instance) return PollEventsResult::NotRunning;

    _eventHandler.pollEvents(_instance, _session);

    if (!_session) return PollEventsResult::NotRunning;

    if( _firstUpdate )
    {
      createActionSpacesAndAttachActionSets();
      _firstUpdate = false;
    }

    switch (_session->getSessionState())
    {
    case XR_SESSION_STATE_IDLE:
      return PollEventsResult::RuntimeIdle;
    case XR_SESSION_STATE_READY:
      if (!_session->getSessionRunning())
      {
        // Begin session. Transition to synchronised after a few begin/end frames
        _session->beginSession(_xrTraits->viewConfigurationType);
      }
      return PollEventsResult::RunningDontRender;
    case XR_SESSION_STATE_SYNCHRONIZED:
      return PollEventsResult::RunningDontRender;
    case XR_SESSION_STATE_VISIBLE:
    case XR_SESSION_STATE_FOCUSED:
      syncActions();
      return PollEventsResult::RunningDoRender;
    case XR_SESSION_STATE_STOPPING:
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
    return PollEventsResult::RunningDontRender;
  }

  bool Viewer::advanceToNextFrame()
  {
    // Viewer::acquireNextFrame
    _frameState = XrFrameState();
    _frameState.type = XR_TYPE_FRAME_STATE;
    _frameState.next = nullptr;

    // TODO: Return statuses - session/instance loss fairly likely here
    xr_check(xrWaitFrame(_session->getSession(), nullptr, &_frameState));
    xr_check(xrBeginFrame(_session->getSession(), nullptr));

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
    return static_cast<bool>(_frameState.shouldRender);
  }

  void Viewer::recordAndSubmit()
  {
    // Locate the views (at predicted frame time), to extract view/proj matrices, and fov
    std::vector<XrView> locatedViews(_viewConfigurationViews.size(), XrView());
    auto viewsValid = false;
    for (auto& v : locatedViews)
    {
      v.type = XR_TYPE_VIEW;
      v.next = nullptr;
    }
    {
      auto viewLocateInfo = XrViewLocateInfo();
      viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
      viewLocateInfo.next = nullptr;
      viewLocateInfo.space = _session->getSpace();
      viewLocateInfo.viewConfigurationType = _xrTraits->viewConfigurationType;
      viewLocateInfo.displayTime = _frameState.predictedDisplayTime;

      auto viewState = XrViewState();
      viewState.type = XR_TYPE_VIEW_STATE;
      viewState.next = nullptr;
      uint32_t numViews = 0;
      xr_check(xrLocateViews(_session->getSession(), &viewLocateInfo, &viewState, static_cast<uint32_t>(locatedViews.size()), &numViews, locatedViews.data()), "Failed to locate views");
      if (numViews != locatedViews.size()) throw Exception({ "Failed to locate views (Incorrect numViews)" });

      viewsValid = (viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) && (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT);
    }

    // Set up projection views, within a composition layer
    // TODO: For now, using a single composition layer - Elements such as a skybox could be directly passed to OpenXR if desired
    _layerProjectionViews.clear();
    for( auto i = 0u; i < _viewConfigurationViews.size(); ++i )
    {
      auto projectionView = XrCompositionLayerProjectionView();
      projectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
      projectionView.next = nullptr;
      projectionView.fov = locatedViews[i].fov;
      projectionView.pose = locatedViews[i].pose;
      projectionView.subImage.swapchain = _session->getSwapchain(i)->getSwapchain();
      auto extent = _session->getSwapchain(i)->getExtent();
      projectionView.subImage.imageRect = XrRect2Di { {0, 0},
        {static_cast<int>(extent.width), static_cast<int>(extent.height)}
      };
      projectionView.subImage.imageArrayIndex = 0;
      _layerProjectionViews.push_back(projectionView);
    }

    _layerProjection = XrCompositionLayerProjection();
    _layerProjection.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    _layerProjection.next = nullptr;

    // Render the scene
    for (auto i = 0u; i < _viewConfigurationViews.size(); ++i)
    {
      auto swapchain = _session->getSwapchain(i);
      uint32_t swapChainImageIndex = 0;
      
      swapchain->acquireImage(swapChainImageIndex);
      XrDuration timeoutNs = 20000000;
      auto waitSuccess = swapchain->waitImage(timeoutNs);

      if( waitSuccess && viewsValid )
      {
        // reset connected ExecuteCommands
        for (auto& recordAndSubmitTask : recordAndSubmitTasks)
        {
          for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
          {
            commandGraph->reset();
            if (!commandGraph->children.empty() )
            {
              // TODO: More reliable way to fetch render graph? Maybe just store a pointer to it if there's no downside?
              if (auto renderGraph = commandGraph->children[0].cast<RenderGraph>())
              {
                renderGraph->framebuffer = _session->frames(i)[swapChainImageIndex].framebuffer;
                if (auto vsgView = renderGraph->children[0].cast<View>())
                {
                  vsgView->camera->viewMatrix = ViewMatrix::create(locatedViews[i].pose);
                  vsgView->camera->projectionMatrix = ProjectionMatrix::create(locatedViews[i].fov, 0.05, 100.0);
                }
              }
            }
        
          }
        }

        for (auto& recordAndSubmitTask : recordAndSubmitTasks)
        {
          recordAndSubmitTask->submit(_frameStamp);
        }

        swapchain->releaseImage();
      }
    }

    _layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
    _layerProjection.space = _session->getSpace();
    _layerProjection.viewCount = static_cast<uint32_t>(_layerProjectionViews.size());
    _layerProjection.views = _layerProjectionViews.data();
    _layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&_layerProjection));
  }

  void Viewer::releaseFrame()
  {
    auto info = XrFrameEndInfo();
    info.type = XR_TYPE_FRAME_END_INFO;
    info.next = nullptr;
    info.displayTime = _frameState.predictedDisplayTime;
    info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    info.layerCount = static_cast<uint32_t>(_layers.size());
    info.layers = _layers.data();

    xr_check(xrEndFrame(_session->getSession(), &info));

    _layers.clear();
  }

  vsg::ref_ptr<vsg::Camera>
    Viewer::createCamera(const VkExtent2D& extent) {
    // Create an initial camera - OpenXR will provide us with matrices later,
    // so the parameters here don't matter
    auto lookAt = vsg::LookAt::create(vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 0.0, 1.0), vsg::dvec3(0.0, 1.0, 0.0));
    auto perspective = vsg::Perspective::create(
      30.0,
      static_cast<double>(extent.width) / static_cast<double>(extent.height),
      0.01, 100.0);
    return vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(extent));
  }

  vsg::CommandGraphs Viewer::createCommandGraphsForView(vsg::ref_ptr<vsg::Node> vsg_scene, std::vector<vsg::ref_ptr<vsg::Camera>>& cameras, bool assignHeadlight) {
    // * vsg::CommandGraph::createCommandGraphForView
    // * vsg::RenderGraph::createRenderGraphForView

    std::vector<vsg::ref_ptr<vsg::CommandGraph>> commandGraphs;

    // TODO: Arguably the camera is per-view, but the render graph itself is otherwise identical. For now have a single render graph, and modify the camera as needed for each eye/view.
    // TODO: To separate the view-specific data, would need multiple recordAndSubmitTasks? - When rendering, need to associate tasks with each view, if duplicated.
    // for( auto i = 0u; i < _viewConfigurationViews.size(); ++i )
    auto i = 0u;
    {
      // TODO: Flexibility on render resolution - For now hardcoded to recommended throughout
      VkExtent2D hmdExtent{
        _viewConfigurationViews[i].recommendedImageRectWidth,
        _viewConfigurationViews[i].recommendedImageRectHeight,
      };

      // vsg::createCommandGraphForView
      auto hmdCommandGraph = CommandGraph::create(_graphicsBinding->getVkDevice(), 
                              _graphicsBinding->getVkPhysicalDevice()->getQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT));

      auto camera = createCamera(hmdExtent);
      cameras.push_back(camera);

      auto view = View::create(camera);
      // TODO: Need to check how this interacts with the multi-view rendering. Does this mean that each eye views the scene with
      //       a different light source? As that may look odd..
      if (assignHeadlight) view->addChild(createHeadlight());
      if (vsg_scene) view->addChild(vsg_scene);

      // Set up the render graph
      auto renderGraph = RenderGraph::create();
      renderGraph->addChild(view);

      if (_session)
      {
        renderGraph->framebuffer = _session->frames(i)[0].framebuffer;
        renderGraph->previous_extent = _session->getSwapchain(i)->getExtent();
        renderGraph->renderArea.offset = {0, 0};
        renderGraph->renderArea.extent = _session->getSwapchain(i)->getExtent();

        // TODO: Will need to pass correct values, assume it comes from the window traits / equivalent?
        renderGraph->setClearValues();
      }
      hmdCommandGraph->addChild(renderGraph);
      commandGraphs.push_back(std::move(hmdCommandGraph));
    }

    return commandGraphs;
  }

  void Viewer::compile(ref_ptr<ResourceHints> hints)
  {
    // TODO: This should be unified with the vsg Viewer::compile
    //       there's no real need for it to be copied here
    if (recordAndSubmitTasks.empty())
    {
      return;
    }

    // TODO: CompileManager requires a child class of Viewer
    // if (!compileManager) compileManager = CompileManager::create(*this, hints);

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
        auto& collectResources = deviceResourceMap[task->device].collectResources;
        for (auto& commandGraph : task->commandGraphs)
        {

            commandGraph->accept(collectResources);
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

      auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT); // TODO : could we just use transfer bit?

      deviceResource.compile = CompileTraversal::create(device, resourceRequirements);

      // CT TODO need to reorganize this whole section
      for (auto& context : deviceResource.compile->contexts)
      {
        context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        context->graphicsQueue = device->getQueue(queueFamily);
        context->reserve(resourceRequirements);
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
        auto& deviceResource = deviceResourceMap[task->device];
        auto& resourceRequirements = deviceResource.collectResources.requirements;

        bool task_containsPagedLOD = false;

        for (auto& commandGraph : task->commandGraphs)
        {
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
            // TODO: CompileManager requires a child class of Viewer
            // task->databasePager->compileManager = compileManager;
        }

        if (task->earlyTransferTask)
        {
            task->earlyTransferTask->assign(resourceRequirements.earlyDynamicData);
        }
        if (task->lateTransferTask)
        {
            task->lateTransferTask->assign(resourceRequirements.lateDynamicData);
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


  void Viewer::assignRecordAndSubmitTask(std::vector<vsg::ref_ptr<vsg::CommandGraph>> in_commandGraphs)
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
        if (commandGraph->level() == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
          primary_commandGraphs.emplace_back(commandGraph);
        else
          secondary_commandGraphs.emplace_back(commandGraph);
      }
      if (!secondary_commandGraphs.empty())
      {
        commandGraphs = secondary_commandGraphs;
        commandGraphs.insert(commandGraphs.end(), primary_commandGraphs.begin(), primary_commandGraphs.end());
      }

      uint32_t numBuffers = 1; // TODO: Needs to relate to OpenXR swapchain surely

      auto device = deviceQueueFamily.device;
      uint32_t transferQueueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_TRANSFER_BIT);

      // with don't have a presentFamily so this set of commandGraphs aren't associated with a window
      // set up Submission with CommandBuffer and signals
      auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers);
      recordAndSubmitTask->commandGraphs = commandGraphs;
      recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
      recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

      recordAndSubmitTask->earlyTransferTask->transferQueue = device->getQueue(transferQueueFamily);
      recordAndSubmitTask->lateTransferTask->transferQueue = device->getQueue(transferQueueFamily);

    }
  }

 
  void Viewer::getViewConfiguration() {
    _viewConfigurationProperties = XrViewConfigurationProperties();
    _viewConfigurationProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
    _viewConfigurationProperties.next = nullptr;

    xr_check(xrGetViewConfigurationProperties(_instance->getInstance(), _instance->getSystem(), _xrTraits->viewConfigurationType, &_viewConfigurationProperties));
    uint32_t count = 0;
    xr_check(xrEnumerateViewConfigurationViews(_instance->getInstance(), _instance->getSystem(), _xrTraits->viewConfigurationType, 0, &count, nullptr));
    _viewConfigurationViews.resize(count, XrViewConfigurationView());
    for (auto& v : _viewConfigurationViews) {
      v.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
      v.next = nullptr;
    }
    xr_check(xrEnumerateViewConfigurationViews(_instance->getInstance(), _instance->getSystem(), _xrTraits->viewConfigurationType, static_cast<uint32_t>(_viewConfigurationViews.size()), &count, _viewConfigurationViews.data()));
  }

  void Viewer::syncActions()
  {
    if( activeActionSets.empty() ) return;

    // Sync the active action sets
    auto info = XrActionsSyncInfo();
    info.type = XR_TYPE_ACTIONS_SYNC_INFO;
    info.next = nullptr;
    std::vector<XrActiveActionSet> d;
    for( auto& actionSet : activeActionSets ) d.push_back({actionSet->getActionSet(), XR_NULL_PATH});
    info.countActiveActionSets = d.size();
    info.activeActionSets = d.data();
    xr_check(xrSyncActions(_session->getSession(), &info));

    // Extract action and pose information
    for (auto& actionSet : activeActionSets)
    {
      for (auto& action : actionSet->actions)
      {
        if (auto a = action.cast<ActionPoseBinding>())
        {
          auto location = XrSpaceLocation();
          location.type = XR_TYPE_SPACE_LOCATION;
          location.next = nullptr;
          xr_check(xrLocateSpace(a->getActionSpace(), _session->getSpace(), _frameState.predictedDisplayTime, &location));
          a->setSpaceLocation(location);
        }

        auto subPaths = action->getSubPaths();
        if( subPaths.empty() ) {
          action->syncInputState(_instance, _session);
        } else {
          for( auto& p : subPaths ) action->syncInputState(_instance, _session, p);
        }
      }
    }
  }

  void Viewer::createActionSpacesAndAttachActionSets()
  {
    if( !_attachedActionSets.empty() ) throw vsg::Exception({"Action spaces have already been attached"});
    // Attach action sets to the session
    if( !actionSets.empty() )
    {
      for( auto& actionSet : actionSets ) _attachedActionSets.push_back(actionSet->getActionSet());

      auto info = XrSessionActionSetsAttachInfo();
      info.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
      info.next = nullptr;
      info.countActionSets = _attachedActionSets.size();
      info.actionSets = _attachedActionSets.data();
      xr_check(xrAttachSessionActionSets(_session->getSession(), &info), "Failed to attach action sets to session");

      for( auto& actionSet : actionSets )
      {
        for( auto& action : actionSet->actions )
        {
          if( auto a = action.cast<ActionPoseBinding>() )
          {
            a->createActionSpace(_session);
          }
        }
      }
    }
  }

  void Viewer::destroyActionSpaces()
  {
    for( auto& actionSet : actionSets )
    {
      for( auto& action : actionSet->actions )
      {
        if( auto a = action.cast<ActionPoseBinding>() )
        {
          if( a->getActionSpace() ) a->destroyActionSpace();
        }
      }
    }
  }

  void Viewer::createSession() {
    if (_session) {
      throw Exception({ "Viewer: Session already initialised" });
    }
    _session = Session::create(_instance, _graphicsBinding, _xrTraits->swapchainFormat, _viewConfigurationViews);
  }

  void Viewer::destroySession() {
    if (!_session) {
      throw Exception({ "Viewer: Session not initialised" });
    }
    destroyActionSpaces();
    _session = 0;
  }
}

