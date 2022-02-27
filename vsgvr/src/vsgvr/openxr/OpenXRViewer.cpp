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

#include <vsgvr/UpdateVRVisitor.h>
#include <vsgvr/openxr/OpenXRViewer.h>

#include <vsgvr/VRProjectionMatrix.h>
#include <vsgvr/VRViewMatrix.h>

#include <vsg/core/Exception.h>
#include <vsg/traversals/ComputeBounds.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/View.h>

namespace vsgvr {
OpenXRViewer::OpenXRViewer(vsg::ref_ptr<vsgvr::VRContext> ctx,
                   vsg::ref_ptr<vsg::WindowTraits> windowTraits)
    : m_ctx(ctx) {
  createDesktopWindow(windowTraits);
}

void OpenXRViewer::update() {
  vsg::Viewer::update();
  return;

  /*

  // Update all tracked devices
  m_ctx->update();
  vsg::ref_ptr<vsg::Visitor> v(new vsgvr::UpdateVRVisitor(m_ctx));
  for (auto &view : views)
    view->accept(*v);

  // Rotate everything into the openVR space
  auto vsgWorldToOVRWorld = vsg::rotate(-vsg::PI / 2.0, 1.0, 0.0, 0.0);
  // And flip the viewport
  vsg::dmat4 viewAxesMat(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

  // Update each of the vr cameras
  float nearPlane = 0.01f;
  float farPlane = 20.0f;
  auto projectionMatrices = m_ctx->getProjectionMatrices(nearPlane, farPlane);
  auto eyeToHeadTransforms = m_ctx->getEyeToHeadTransforms();

  // View matrices for each eye are provided, but assume
  // data is in the openVR coordinate space (y-up, z-backward)
  auto hmd = m_ctx->hmd();
  if( hmd )
  {
    auto& hmdToWorld = m_ctx->hmd()->deviceToAbsoluteMatrix;
    auto worldToHmd = vsg::inverse(hmdToWorld);

    for (auto i = 0u; i < m_ctx->numberOfHmdImages(); ++i) {
      // Projection matrices are fairly simple
      auto& proj = projectionMatrices[i];
      m_hmdCameras[i]->projectionMatrix =
          vsgvr::VRProjectionMatrix::create(proj);

      auto hmdToEye = vsg::inverse(eyeToHeadTransforms[i]);
      auto m = hmdToEye * worldToHmd;
      auto viewMat = viewAxesMat * m * vsgWorldToOVRWorld;
      m_hmdCameras[i]->viewMatrix = vsgvr::VRViewMatrix::create(viewMat);
    }

    // For now bind the mirror window to one of the eyes. The desktop view could
    // have any projection, and be bound to the hmd's position.
    auto& proj = projectionMatrices[0];
    m_desktopCamera->projectionMatrix =
        vsgvr::VRProjectionMatrix::create(proj);

    auto hmdToEye = vsg::inverse(eyeToHeadTransforms[0]);
    auto m = hmdToEye * worldToHmd;
    auto viewMat = viewAxesMat * m * vsgWorldToOVRWorld;
    m_desktopCamera->viewMatrix = vsgvr::VRViewMatrix::create(viewMat);
  }
  */
}

void OpenXRViewer::present() {
  // Call to base class (Desktop window presentation)
  vsg::Viewer::present();

/*
  // VR Presentation
  submitVRFrames();

  // TODO: The pose updates here aren't perfect. This should be updated
  // to be async, using 'explicit timings'
  // https://github.com/ValveSoftware/openvr/wiki/Vulkan#explicit-timing
  // https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetDeviceToAbsoluteTrackingPose
  m_ctx->waitGetPoses();
*/
}

void OpenXRViewer::addWindow(vsg::ref_ptr<vsg::Window>) {
  // Not allowed - The VRViewer manages its own mirror window
}

void OpenXRViewer::createDesktopWindow(
    vsg::ref_ptr<vsg::WindowTraits> windowTraits) {
  // Check what extensions are needed by the backend
  {
    // TODO: Rework needed here - OpenXR needs to be able to specify the physical device, or directly create the vulkan instance
    // A temporary window, just to get an instance
    auto tempWindow = vsg::Window::create(vsg::WindowTraits::create(*windowTraits));

    // TODO: Initially queried the extensions and re-initialised the window. With OpenXR performs differnet validation steps.
    auto vrRequiredInstanceExtensions = m_ctx->instanceExtensionsRequired(
      tempWindow->getOrCreateInstance()->apiVersion);
    auto vrRequiredDeviceExtensions = m_ctx->deviceExtensionsRequired(
        tempWindow->getOrCreateInstance(),
        tempWindow->getOrCreatePhysicalDevice());
    for (auto &ext : vrRequiredInstanceExtensions)
      windowTraits->instanceExtensionNames.push_back(ext.c_str());
    for (auto &ext : vrRequiredDeviceExtensions)
      windowTraits->deviceExtensionNames.push_back(ext.c_str());
  }

  // This viewer renders HMD output as part of the desktop render graph
  // As such, vsync must be disabled, to avoid the headset being limited
  // to the desktop monitor's refresh rate.
  // TODO: In OpenXR see XR_KHR_vulkan_enable2, but that would need the opposite setup for desktop window.
  windowTraits->swapchainPreferences.presentMode =
      VK_PRESENT_MODE_IMMEDIATE_KHR;
  m_desktopWindow = vsg::Window::create(windowTraits);
  if (!m_desktopWindow) {
    throw vsg::Exception{"Failed to create desktop mirror window"};
  }
  vsg::Viewer::addWindow(m_desktopWindow);
}

/*
std::vector<vsg::ref_ptr<vsg::CommandGraph>>
OpenXRViewer::createCommandGraphsForView(vsg::ref_ptr<vsg::Node> vsg_scene) {
  // Create the framebuffers and command graph for HMD view
  // HMD is rendered through one or more cameras bound to each eye / input image
  // to the vr backend
  auto numImages = m_ctx->numberOfHmdImages();

  vsg::ref_ptr<vsg::CompileTraversal> compile = vsg::CompileTraversal::create(m_desktopWindow);
  uint32_t hmdWidth = 0, hmdHeight = 0;
  m_ctx->getRecommendedTargetSize(hmdWidth, hmdHeight);
  VkExtent2D hmdExtent{hmdWidth, hmdHeight};

  hmdCommandGraph = vsg::CommandGraph::create(m_desktopWindow);
  auto desktopCommandGraph = vsg::CommandGraph::create(m_desktopWindow);

  for (auto imgI = 0u; imgI < numImages; ++imgI) {
    auto image = HMDImage();
    auto camera = createCameraForScene(vsg_scene, hmdExtent);
    m_hmdCameras.push_back(camera);
    auto renderGraph =
        createHmdRenderGraph(m_desktopWindow->getDevice(), *compile->contexts.front(),
                             hmdExtent, image, m_desktopWindow->clearColor());
    hmdImages.push_back(image);
    auto view = vsg::View::create(camera, vsg_scene);
    views.push_back(view);
    renderGraph->addChild(view);
    hmdCommandGraph->addChild(renderGraph);
  }

  // Create render graph for desktop window
  m_desktopCamera =
      createCameraForScene(vsg_scene, m_desktopWindow->extent2D());
  auto desktopRenderGraph = vsg::createRenderGraphForView(
      m_desktopWindow, m_desktopCamera, vsg_scene);
  desktopCommandGraph->addChild(desktopRenderGraph);

  return {desktopCommandGraph, hmdCommandGraph};
}*/

/*
vsg::ref_ptr<vsg::Camera>
OpenXRViewer::createCameraForScene(vsg::ref_ptr<vsg::Node> scene,
                               const VkExtent2D &extent) {
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

vsg::ref_ptr<vsg::RenderGraph>
OpenXRViewer::createHmdRenderGraph(vsg::Device *device, vsg::Context &context,
                               const VkExtent2D &extent, HMDImage &img,
                               VkClearColorValue &clearColour) {
  VkExtent3D attachmentExtent{extent.width, extent.height, 1};

  img.width = extent.width;
  img.height = extent.height;

  // Attachments
  // create image for color attachment
  img.colourImage = vsg::Image::create();
  img.colourImageInfo = vsg::ImageInfo::create();
  img.colourImage->imageType = VK_IMAGE_TYPE_2D;
  img.colourImage->extent = attachmentExtent;
  img.colourImage->mipLevels = 1;
  img.colourImage->arrayLayers = 1;
  img.colourImage->format = hmdImageFormat;
  img.colourImage->samples = VK_SAMPLE_COUNT_1_BIT;
  img.colourImage->tiling = VK_IMAGE_TILING_OPTIMAL;
  img.colourImage->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT |
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  img.colourImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img.colourImage->flags = 0;
  img.colourImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  img.colourImageInfo->sampler = nullptr;
  img.colourImageInfo->imageView =
      vsg::createImageView(context, img.colourImage, VK_IMAGE_ASPECT_COLOR_BIT);
  img.colourImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;

  // create depth buffer
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  img.depthImage = vsg::Image::create();
  img.depthImageInfo = vsg::ImageInfo::create();
  img.depthImage->imageType = VK_IMAGE_TYPE_2D;
  img.depthImage->extent = attachmentExtent;
  img.depthImage->mipLevels = 1;
  img.depthImage->arrayLayers = 1;
  img.depthImage->samples = VK_SAMPLE_COUNT_1_BIT;
  img.depthImage->format = depthFormat;
  img.depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
  img.depthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  img.depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img.depthImage->flags = 0;
  img.depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  // XXX Does layout matter?
  img.depthImageInfo->sampler = nullptr;
  img.depthImageInfo->imageView =
      vsg::createImageView(context, img.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
  img.depthImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;

  // attachment descriptions
  vsg::RenderPass::Attachments attachments(2);
  // Color attachment
  attachments[0].format = hmdImageFormat;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  // Depth attachment
  attachments[1].format = depthFormat;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  vsg::AttachmentReference colorReference = {
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  vsg::AttachmentReference depthReference = {
      1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
  vsg::RenderPass::Subpasses subpassDescription(1);
  subpassDescription[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription[0].colorAttachments.emplace_back(colorReference);
  subpassDescription[0].depthStencilAttachments.emplace_back(depthReference);

  vsg::RenderPass::Dependencies dependencies(2);

  // XXX This dependency is copied from the offscreenrender.cpp
  // example. I don't completely understand it, but I think it's
  // purpose is to create a barrier if some earlier render pass was
  // using this framebuffer's attachment as a texture.
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // This is the heart of what makes Vulkan offscreen rendering
  // work: render passes that follow are blocked from using this
  // passes' color attachment in their fragment shaders until all
  // this pass' color writes are finished.
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  auto renderPass = vsg::RenderPass::create(device, attachments,
                                            subpassDescription, dependencies);

  // Framebuffer
  auto fbuf =
      vsg::Framebuffer::create(renderPass,
                               vsg::ImageViews{img.colourImageInfo->imageView,
                                               img.depthImageInfo->imageView},
                               extent.width, extent.height, 1);

  auto rendergraph = vsg::RenderGraph::create();
  rendergraph->renderArea.offset = VkOffset2D{0, 0};
  rendergraph->renderArea.extent = extent;
  rendergraph->framebuffer = fbuf;

  rendergraph->clearValues.resize(2);
  rendergraph->clearValues[0].color = clearColour;
  rendergraph->clearValues[1].depthStencil = VkClearDepthStencilValue{0.0f, 0};

  return rendergraph;
}

void OpenXRViewer::submitVRFrames() {
  std::vector<VkImage> images;
  for (auto &img : hmdImages)
    images.push_back(
        img.colourImage->vk(m_desktopWindow->getDevice()->deviceID));

  m_ctx->submitFrames(images, m_desktopWindow->getDevice()->getDevice(),
                      m_desktopWindow->getPhysicalDevice()->getPhysicalDevice(),
                      m_desktopWindow->getInstance()->getInstance(),
                      m_desktopWindow->getDevice()
                          ->getQueue(hmdCommandGraph->queueFamily)
                          ->queue(),
                      hmdCommandGraph->queueFamily, hmdImages.front().width,
                      hmdImages.front().height, hmdImageFormat,
                      VK_SAMPLE_COUNT_1_BIT);
}
*/
} // namespace vsgvr
