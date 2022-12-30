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

#include <vsgvr/app/CompositionLayers.h>

#include <vsgvr/xr/ViewMatrix.h>
#include <vsgvr/xr/ProjectionMatrix.h>

#include <vsgvr/xr/Instance.h>
#include <vsgvr/xr/Swapchain.h>
#include <vsgvr/xr/Session.h>
#include <vsgvr/xr/GraphicsBindingVulkan.h>

#include <vsg/app/View.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/ui/FrameStamp.h>
#include <vsg/core/Exception.h>
#include <vsg/vk/SubmitCommands.h>
#include <vsg/commands/PipelineBarrier.h>

#include "../xr/Macros.cpp"

namespace vsgvr {
  CompositionLayer::CompositionLayer(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) 
    : _instance(instance)
    , _xrTraits(xrTraits)
  {}
  CompositionLayer::~CompositionLayer() {}


  void CompositionLayer::createSwapchains(vsg::ref_ptr<vsgvr::Session> session,vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> graphicsBinding, std::vector< SwapchainImageRequirements> imageRequirements)
  {
    if (!_viewData.empty()) throw vsg::Exception({ "Swapchain already initialised" });

    for (auto& viewConfig : imageRequirements)
    {
      PerViewData v;
      v.swapchain = Swapchain::create(session->getSession(), _xrTraits->swapchainFormat, viewConfig.width, viewConfig.height, viewConfig.sampleCount, graphicsBinding);

      auto extent = v.swapchain->getExtent();
      VkSampleCountFlagBits framebufferSamples;
      switch (viewConfig.sampleCount)
      {
      case 1:
        framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
      case 2:
        framebufferSamples = VK_SAMPLE_COUNT_2_BIT;
        break;
      case 4:
        framebufferSamples = VK_SAMPLE_COUNT_4_BIT;
        break;
      case 8:
        framebufferSamples = VK_SAMPLE_COUNT_8_BIT;
        break;
      case 16:
        framebufferSamples = VK_SAMPLE_COUNT_16_BIT;
        break;
      case 32:
        framebufferSamples = VK_SAMPLE_COUNT_32_BIT;
        break;
      case 64:
        framebufferSamples = VK_SAMPLE_COUNT_64_BIT;
        break;
      default:
        framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
      }

      bool multisampling = framebufferSamples != VK_SAMPLE_COUNT_1_BIT;
      if (multisampling)
      {
        v.multisampleImage = vsg::Image::create();
        v.multisampleImage->imageType = VK_IMAGE_TYPE_2D;
        v.multisampleImage->format = _xrTraits->swapchainFormat;
        v.multisampleImage->extent.width = extent.width;
        v.multisampleImage->extent.height = extent.height;
        v.multisampleImage->extent.depth = 1;
        v.multisampleImage->mipLevels = 1;
        v.multisampleImage->arrayLayers = 1;
        v.multisampleImage->samples = framebufferSamples;
        v.multisampleImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        v.multisampleImage->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        v.multisampleImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        v.multisampleImage->flags = 0;
        v.multisampleImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        v.multisampleImage->compile(graphicsBinding->getVkDevice());
        v.multisampleImage->allocateAndBindMemory(graphicsBinding->getVkDevice());

        v.multisampleImageView = vsg::ImageView::create(v.multisampleImage, VK_IMAGE_ASPECT_COLOR_BIT);
        v.multisampleImageView->compile(graphicsBinding->getVkDevice());
      }

      bool requiresDepthRead = false; // (_traits->depthImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;

      // Window::_initRenderPass
      // TODO: Test single & multisampled rendering
      {
        if (framebufferSamples == VK_SAMPLE_COUNT_1_BIT)
        {
          v.renderPass = vsg::createRenderPass(graphicsBinding->getVkDevice(), _xrTraits->swapchainFormat, VK_FORMAT_D32_SFLOAT, requiresDepthRead);
        }
        else
        {
          v.renderPass = vsg::createMultisampledRenderPass(graphicsBinding->getVkDevice(), _xrTraits->swapchainFormat, VK_FORMAT_D32_SFLOAT, framebufferSamples, requiresDepthRead);
        }
      }

      bool requiresDepthResolve = (multisampling && requiresDepthRead);

      // create depth buffer
      v.depthImage = vsg::Image::create();
      v.depthImage->imageType = VK_IMAGE_TYPE_2D;
      v.depthImage->extent.width = extent.width;
      v.depthImage->extent.height = extent.height;
      v.depthImage->extent.depth = 1;
      v.depthImage->mipLevels = 1;
      v.depthImage->arrayLayers = 1;
      v.depthImage->format = VK_FORMAT_D32_SFLOAT; // _depthFormat;
      v.depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
      v.depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      v.depthImage->samples = framebufferSamples;
      v.depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      v.depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // _traits->depthImageUsage;

      v.depthImage->compile(graphicsBinding->getVkDevice());
      v.depthImage->allocateAndBindMemory(graphicsBinding->getVkDevice());

      v.depthImageView = vsg::ImageView::create(v.depthImage);
      v.depthImageView->compile(graphicsBinding->getVkDevice());

      if (requiresDepthResolve)
      {
        v.multisampleDepthImage = v.depthImage;
        v.multisampleDepthImageView = v.depthImageView;

        // create depth buffer
        v.depthImage = vsg::Image::create();
        v.depthImage->imageType = VK_IMAGE_TYPE_2D;
        v.depthImage->extent.width = extent.width;
        v.depthImage->extent.height = extent.height;
        v.depthImage->extent.depth = 1;
        v.depthImage->mipLevels = 1;
        v.depthImage->arrayLayers = 1;
        v.depthImage->format = VK_FORMAT_D32_SFLOAT; // _depthFormat;
        v.depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        v.depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        v.depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // _traits->depthImageUsage;
        v.depthImage->samples = VK_SAMPLE_COUNT_1_BIT;
        v.depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        v.depthImage->compile(graphicsBinding->getVkDevice());
        v.depthImage->allocateAndBindMemory(graphicsBinding->getVkDevice());

        v.depthImageView = vsg::ImageView::create(v.depthImage);
        v.depthImageView->compile(graphicsBinding->getVkDevice());
      }

      int graphicsFamily = graphicsBinding->getVkPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);

      // set up framebuffer and associated resources
      const auto& imageViews = v.swapchain->getImageViews();

      for (size_t i = 0; i < imageViews.size(); ++i)
      {
        vsg::ImageViews attachments;
        if (v.multisampleImageView)
        {
          attachments.push_back(v.multisampleImageView);
        }
        attachments.push_back(imageViews[i]);

        if (v.multisampleDepthImageView)
        {
          attachments.push_back(v.multisampleDepthImageView);
        }
        attachments.push_back(v.depthImageView);

        vsg::ref_ptr<vsg::Framebuffer> fb = vsg::Framebuffer::create(v.renderPass, attachments, extent.width, extent.height, 1);

        v.frames.push_back({ imageViews[i], fb });
      }

      {
        // ensure image attachments are setup on GPU.
        vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(graphicsBinding->getVkDevice(), graphicsFamily);
        submitCommandsToQueue(commandPool, graphicsBinding->getVkDevice()->getQueue(graphicsFamily), [&](vsg::CommandBuffer& commandBuffer) {
          auto depthImageBarrier = vsg::ImageMemoryBarrier::create(
            0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            v.depthImage,
            v.depthImageView->subresourceRange);

          auto pipelineBarrier = vsg::PipelineBarrier::create(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0, depthImageBarrier);
          pipelineBarrier->record(commandBuffer);

          if (multisampling)
          {
            auto msImageBarrier = vsg::ImageMemoryBarrier::create(
              0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
              VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
              v.multisampleImage,
              VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
            auto msPipelineBarrier = vsg::PipelineBarrier::create(
              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              0, msImageBarrier);
            msPipelineBarrier->record(commandBuffer);
          }
          });
      }

      _viewData.push_back(std::move(v));
    }
  }

  void CompositionLayer::destroySwapchains()
  {
    _viewData.clear();
  }

  vsg::CommandGraphs CompositionLayer::createCommandGraphsForView(vsg::ref_ptr<vsgvr::Session> session, vsg::ref_ptr<vsg::Node> vsg_scene, std::vector<vsg::ref_ptr<vsg::Camera>>& cameras, bool assignHeadlight) {
    // * vsg::CommandGraph::createCommandGraphForView
    // * vsg::RenderGraph::createRenderGraphForView

    // vsgvr-specific: Work out what views the composition layer requires
    // from OpenXR, and create swapchains as needed.
    populateLayerSpecificData(_instance, _xrTraits);
    auto swapchainImageRequirements = getSwapchainImageRequirements();
    auto graphicsBinding = session->getGraphicsBinding();
    createSwapchains(session, graphicsBinding, swapchainImageRequirements);

    std::vector<vsg::ref_ptr<vsg::CommandGraph>> commandGraphs;

    // TODO: Arguably the camera is per-view, but the render graph itself is otherwise identical. For now have a single render graph, and modify the camera as needed for each eye/view.
    // TODO: To separate the view-specific data, would need multiple recordAndSubmitTasks? - When rendering, need to associate tasks with each view, if duplicated.
    // for( auto i = 0u; i < _viewConfigurationViews.size(); ++i )
    auto i = 0u;
    {
      // TODO: Flexibility on render resolution - For now hardcoded to recommended throughout
      VkExtent2D hmdExtent{
        swapchainImageRequirements[i].width,
        swapchainImageRequirements[i].height,
      };

      // vsg::createCommandGraphForView
      auto hmdCommandGraph = vsg::CommandGraph::create(graphicsBinding->getVkDevice(),
        graphicsBinding->getVkPhysicalDevice()->getQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT));

      if (cameras.empty())
      {
        // User didn't provide a camera, so create one as needed
        // Parameters here are a placeholder - Assuming that the composition layer
        // controls the camera matrices later.
        auto lookAt = vsg::LookAt::create(vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 0.0, 1.0), vsg::dvec3(0.0, 1.0, 0.0));
        auto perspective = vsg::Perspective::create(30.0, 1.0, 0.01, 10.0);
        auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(hmdExtent));
        cameras.push_back(camera);
      }
      auto camera = cameras.front();

      auto view = vsg::View::create(camera);
      // TODO: Need to check how this interacts with the multi-view rendering. Does this mean that each eye views the scene with
      //       a different light source? As that may look odd..
      if (assignHeadlight) view->addChild(vsg::createHeadlight());
      if (vsg_scene) view->addChild(vsg_scene);

      // Set up the render graph
      auto renderGraph = vsg::RenderGraph::create();
      renderGraph->addChild(view);

      renderGraph->framebuffer = frames(i)[0].framebuffer;
      renderGraph->previous_extent = getSwapchain(i)->getExtent();
      renderGraph->renderArea.offset = { 0, 0 };
      renderGraph->renderArea.extent = getSwapchain(i)->getExtent();

      // TODO: Will need to pass correct values, assume it comes from the window traits / equivalent?
      renderGraph->setClearValues();
      hmdCommandGraph->addChild(renderGraph);
      commandGraphs.push_back(std::move(hmdCommandGraph));
    }

    return commandGraphs;
  }

  void CompositionLayer::assignRecordAndSubmitTask(std::vector<vsg::ref_ptr<vsg::CommandGraph>> in_commandGraphs)
  {
    struct DeviceQueueFamily
    {
      vsg::Device* device = nullptr;
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
    std::map<DeviceQueueFamily, vsg::CommandGraphs> deviceCommandGraphsMap;
    for (auto& commandGraph : in_commandGraphs)
    {
      deviceCommandGraphsMap[DeviceQueueFamily{ commandGraph->device.get(), commandGraph->queueFamily, commandGraph->presentFamily }].emplace_back(commandGraph);
    }

    // create the required RecordAndSubmitTask and any Presentation objects that are required for each set of CommandGraphs
    for (auto& [deviceQueueFamily, commandGraphs] : deviceCommandGraphsMap)
    {
      // make sure the secondary CommandGraphs appear first in the commandGraphs list so they are filled in first
      vsg::CommandGraphs primary_commandGraphs;
      vsg::CommandGraphs secondary_commandGraphs;
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

      uint32_t numBuffers = 1;
      auto device = deviceQueueFamily.device;
      uint32_t transferQueueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_TRANSFER_BIT);

      // with don't have a presentFamily so this set of commandGraphs aren't associated with a window
      // set up Submission with CommandBuffer and signals
      auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers);
      recordAndSubmitTask->commandGraphs = commandGraphs;
      recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
      _recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

      recordAndSubmitTask->earlyTransferTask->transferQueue = device->getQueue(transferQueueFamily);
      recordAndSubmitTask->lateTransferTask->transferQueue = device->getQueue(transferQueueFamily);
    }
  }


  void CompositionLayer::advanceToNextFrame()
  {
    for (auto& task : _recordAndSubmitTasks)
    {
      task->advance();
    }
  }


  void CompositionLayer::compile(vsg::ref_ptr<vsg::ResourceHints> hints)
  {
    if (_recordAndSubmitTasks.empty())
    {
      return;
    }

    for (auto& k : _recordAndSubmitTasks)
    {
      // TODO: CompileManager requires a child class of Viewer
      // if (!compileManager) compileManager = CompileManager::create(*this, hints);

      bool containsPagedLOD = false;
      vsg::ref_ptr<vsg::DatabasePager> databasePager;

      struct DeviceResources
      {
        vsg::CollectResourceRequirements collectResources;
        vsg::ref_ptr<vsg::CompileTraversal> compile;
      };

      // find which devices are available and the resources required for then,
      using DeviceResourceMap = std::map<vsg::ref_ptr<vsg::Device>, DeviceResources>;
      DeviceResourceMap deviceResourceMap;
      for (auto& task : _recordAndSubmitTasks)
      {
        auto& collectResources = deviceResourceMap[task->device].collectResources;
        for (auto& commandGraph : task->commandGraphs)
        {
          commandGraph->accept(collectResources);
        }

        if (task->databasePager && !databasePager) databasePager = task->databasePager;
      }


      // allocate DescriptorPool for each Device 
      vsg::ResourceRequirements::Views views;
      for (auto& [device, deviceResource] : deviceResourceMap)
      {
        auto& collectResources = deviceResource.collectResources;
        auto& resourceRequirements = collectResources.requirements;

        if (hints) hints->accept(collectResources);

        views.insert(resourceRequirements.views.begin(), resourceRequirements.views.end());

        if (resourceRequirements.containsPagedLOD) containsPagedLOD = true;

        auto physicalDevice = device->getPhysicalDevice();

        auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT); // TODO : could we just use transfer bit?

        deviceResource.compile = vsg::CompileTraversal::create(device, resourceRequirements);

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
        auto view = const_cast<vsg::View*>(const_view);
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
            vsg::Bin::SortOrder sortOrder = (binNumber < 0) ? vsg::Bin::ASCENDING : ((binNumber == 0) ? vsg::Bin::NO_SORT : vsg::Bin::DESCENDING);
            view->bins.push_back(vsg::Bin::create(binNumber, sortOrder));
          }
        }
      }

      if (containsPagedLOD && !databasePager) databasePager = vsg::DatabasePager::create();

      // create the Vulkan objects
      for (auto& task : _recordAndSubmitTasks)
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
      for (auto& task : _recordAndSubmitTasks)
      {
        if (task->databasePager)
        {
          task->databasePager->start();
        }
      }
    }
  }

  CompositionLayerProjection::CompositionLayerProjection(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits) 
    : Inherit(instance, xrTraits)
    {}
  CompositionLayerProjection::CompositionLayerProjection(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, XrCompositionLayerFlags inFlags)
    : Inherit(instance, xrTraits)
    , flags(inFlags)
  {}
  CompositionLayerProjection::~CompositionLayerProjection() {}

  void CompositionLayerProjection::populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
  {
    if (!_viewConfigurationViews.empty())
    {
      // This is fine - User may have called manually
      // TODO: Could have more flexible layer width/height selection here, etc.
      return;
    }

    _viewConfigurationProperties = XrViewConfigurationProperties();
    _viewConfigurationProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
    _viewConfigurationProperties.next = nullptr;

    xr_check(xrGetViewConfigurationProperties(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, &_viewConfigurationProperties));
    uint32_t count = 0;
    xr_check(xrEnumerateViewConfigurationViews(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, 0, &count, nullptr));
    _viewConfigurationViews.resize(count, XrViewConfigurationView());
    for (auto& v : _viewConfigurationViews) {
      v.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
      v.next = nullptr;
    }
    xr_check(xrEnumerateViewConfigurationViews(instance->getInstance(), instance->getSystem(), xrTraits->viewConfigurationType, static_cast<uint32_t>(_viewConfigurationViews.size()), &count, _viewConfigurationViews.data()));
  }

  std::vector<CompositionLayer::SwapchainImageRequirements> CompositionLayerProjection::getSwapchainImageRequirements()
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    for (auto& config : _viewConfigurationViews)
    {
      reqs.push_back({
        config.recommendedImageRectHeight,
        config.recommendedImageRectHeight,
        config.recommendedSwapchainSampleCount
      });
    }
    return reqs;
  }

  void CompositionLayerProjection::render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp)
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
      viewLocateInfo.space = session->getSpace();
      viewLocateInfo.viewConfigurationType = _xrTraits->viewConfigurationType;
      viewLocateInfo.displayTime = frameState.predictedDisplayTime;

      auto viewState = XrViewState();
      viewState.type = XR_TYPE_VIEW_STATE;
      viewState.next = nullptr;
      uint32_t numViews = 0;
      xr_check(xrLocateViews(session->getSession(), &viewLocateInfo, &viewState, static_cast<uint32_t>(locatedViews.size()), &numViews, locatedViews.data()), "Failed to locate views");
      if (numViews != locatedViews.size()) throw vsg::Exception({ "Failed to locate views (Incorrect numViews)" });

      viewsValid = (viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) && (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT);
    }

    // Set up projection views, within a composition layer
    _layerProjectionViews.clear();
    for (auto i = 0u; i < _viewConfigurationViews.size(); ++i)
    {
      auto projectionView = XrCompositionLayerProjectionView();
      projectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
      projectionView.next = nullptr;
      projectionView.fov = locatedViews[i].fov;
      projectionView.pose = locatedViews[i].pose;
      projectionView.subImage.swapchain = getSwapchain(i)->getSwapchain();
      auto extent = getSwapchain(i)->getExtent();
      projectionView.subImage.imageRect = XrRect2Di{ {0, 0},
        {static_cast<int>(extent.width), static_cast<int>(extent.height)}
      };
      projectionView.subImage.imageArrayIndex = 0;
      _layerProjectionViews.push_back(projectionView);
    }

    // Render the scene
    for (auto i = 0u; i < _viewConfigurationViews.size(); ++i)
    {
      auto swapchain = getSwapchain(i);
      uint32_t swapChainImageIndex = 0;

      swapchain->acquireImage(swapChainImageIndex);
      XrDuration timeoutNs = 20000000;
      auto waitSuccess = swapchain->waitImage(timeoutNs);

      if (waitSuccess && viewsValid)
      {
        // reset connected ExecuteCommands
        for (auto& recordAndSubmitTask : _recordAndSubmitTasks)
        {
          for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
          {
            commandGraph->reset();
            if (!commandGraph->children.empty())
            {
              // TODO: More reliable way to fetch render graph? Maybe just store a pointer to it if there's no downside?
              if (auto renderGraph = commandGraph->children[0].cast<vsg::RenderGraph>())
              {
                renderGraph->framebuffer = frames(i)[swapChainImageIndex].framebuffer;
                if (auto vsgView = renderGraph->children[0].cast<vsg::View>())
                {
                  vsgView->camera->viewMatrix = ViewMatrix::create(locatedViews[i].pose);
                  vsgView->camera->projectionMatrix = ProjectionMatrix::create(locatedViews[i].fov, nearPlane, farPlane);
                }
              }
            }
          }
        }

        for (auto& recordAndSubmitTask : _recordAndSubmitTasks)
        {
          recordAndSubmitTask->submit(frameStamp);
        }

        swapchain->releaseImage();
      }
    }

    _compositionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    _compositionLayer.next = nullptr;
    _compositionLayer.layerFlags = flags;
    _compositionLayer.space = session->getSpace();
    _compositionLayer.viewCount = static_cast<uint32_t>(_layerProjectionViews.size());
    _compositionLayer.views = _layerProjectionViews.data();
  }

  CompositionLayerQuad::CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
    : Inherit(instance, xrTraits)
    {}
  CompositionLayerQuad::CompositionLayerQuad(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits, XrPosef inPose, XrExtent2Df inSize, XrCompositionLayerFlags inFlags, XrEyeVisibility inEyeVisibility)
    : Inherit(instance, xrTraits)
    , pose(inPose)
    , size(inSize)
    , flags(inFlags)
    , eyeVisibility(inEyeVisibility)
  {}
  CompositionLayerQuad::~CompositionLayerQuad() {}

  void CompositionLayerQuad::populateLayerSpecificData(vsg::ref_ptr<vsgvr::Instance> instance, vsg::ref_ptr<vsgvr::Traits> xrTraits)
  {
    // TODO: What to put here? Perhaps it's entirely up to the user in this case, since they're just rendering <a quad> in space
    //       Image size for swapchain of course matters, but that'll just be the image resolution that's passed to OpenXR, before
    //       composition - There's nothing in OpenXR to recommend a size for us here.
  }

  std::vector<CompositionLayer::SwapchainImageRequirements> CompositionLayerQuad::getSwapchainImageRequirements()
  {
    std::vector<CompositionLayer::SwapchainImageRequirements> reqs;
    
    reqs.push_back({1920, 1080, 1});
    return reqs;
  }
  void CompositionLayerQuad::render(vsg::ref_ptr<vsgvr::Session> session, XrFrameState frameState, vsg::ref_ptr<vsg::FrameStamp> frameStamp)
  {
    // Render the scene
    auto swapchainI = 0;
    auto swapchain = getSwapchain(swapchainI);
    uint32_t swapChainImageIndex = 0;

    swapchain->acquireImage(swapChainImageIndex);
    XrDuration timeoutNs = 20000000;
    auto waitSuccess = swapchain->waitImage(timeoutNs);

    if (waitSuccess)
    {
      // reset connected ExecuteCommands
      for (auto& recordAndSubmitTask : _recordAndSubmitTasks)
      {
        for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
        {
          commandGraph->reset();
          if (!commandGraph->children.empty())
          {
            // TODO: More reliable way to fetch render graph? Maybe just store a pointer to it if there's no downside?
            if (auto renderGraph = commandGraph->children[0].cast<vsg::RenderGraph>())
            {
              renderGraph->framebuffer = frames(swapchainI)[swapChainImageIndex].framebuffer;
            }
          }
        }
      }

      for (auto& recordAndSubmitTask : _recordAndSubmitTasks)
      {
        recordAndSubmitTask->submit(frameStamp);
      }

      swapchain->releaseImage();
    }

    // Add the composition layer for the frame end info
    _compositionLayer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
    _compositionLayer.next = nullptr;
    _compositionLayer.layerFlags = flags;
    _compositionLayer.eyeVisibility = eyeVisibility;
    _compositionLayer.pose = pose;
    _compositionLayer.size = size;
    _compositionLayer.space = session->getSpace();
    _compositionLayer.subImage.swapchain = swapchain->getSwapchain();
    auto extent = swapchain->getExtent();
    _compositionLayer.subImage.imageRect = XrRect2Di{ {0, 0},
      {static_cast<int>(extent.width), static_cast<int>(extent.height)}
    };
    _compositionLayer.subImage.imageArrayIndex = 0;
  }

}
