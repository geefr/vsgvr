
#include <vsg/all.h>

#include <openvr.h>
#include "vr/env.h"
#include "vr/device.h"
#include "vr/controller.h"

#include "updatevrvisitor.h"
#include "rawmatrices.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <fmt/core.h>

vsg::ref_ptr<vsg::Window> window;
vsg::ref_ptr<vsg::Camera> hmdCameraLeft;
vsg::ref_ptr<vsg::Camera> hmdCameraRight;
vsg::ref_ptr<vsg::Camera> desktopCamera;

struct HMDImage
{
  vsg::ImageInfo colourImageInfo;
  vsg::ref_ptr<vsg::Image> colourImage;
  vsg::ImageInfo depthImageInfo;
  vsg::ref_ptr<vsg::Image> depthImage;
  uint32_t width;
  uint32_t height;
};
HMDImage hmdImageLeft;
HMDImage hmdImageRight;
vsg::ref_ptr<vsg::CommandGraph> hmdCommandGraph;
auto hmdImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

std::list<std::string> vrRequiredInstanceExtensions;
std::list<std::string> vrRequiredDeviceExtensions;

vsg::dmat4 openVRMatToVSG(glm::dmat4 mat)
{
  vsg::dmat4 result(glm::value_ptr(mat));
  return result;
}

/// Create the render graph for the headset - Rendering into an image, which will be handed off to openvr
vsg::ref_ptr<vsg::RenderGraph> createHmdRenderGraph(vsg::Device *device, vsg::Context &context, const VkExtent2D &extent, HMDImage &img, VkClearColorValue& clearColour)
{
  VkExtent3D attachmentExtent{extent.width, extent.height, 1};

  img.width = extent.width;
  img.height = extent.height;

  // Attachments
  // create image for color attachment
  img.colourImage = vsg::Image::create();
  img.colourImage->imageType = VK_IMAGE_TYPE_2D;
  img.colourImage->extent = attachmentExtent;
  img.colourImage->mipLevels = 1;
  img.colourImage->arrayLayers = 1;
  img.colourImage->format = hmdImageFormat;
  img.colourImage->samples = VK_SAMPLE_COUNT_1_BIT;
  img.colourImage->tiling = VK_IMAGE_TILING_OPTIMAL;
  img.colourImage->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  img.colourImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img.colourImage->flags = 0;
  img.colourImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  img.colourImageInfo.sampler = nullptr;
  img.colourImageInfo.imageView = vsg::createImageView(context, img.colourImage, VK_IMAGE_ASPECT_COLOR_BIT);
  img.colourImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

  // create depth buffer
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  img.depthImage = vsg::Image::create();
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
  img.depthImageInfo.sampler = nullptr;
  img.depthImageInfo.imageView = vsg::createImageView(context, hmdImageLeft.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
  img.depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

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

  VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference depthReference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
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

  auto renderPass = vsg::RenderPass::create(device, attachments, subpassDescription, dependencies);

  // Framebuffer
  auto fbuf = vsg::Framebuffer::create(renderPass, vsg::ImageViews{img.colourImageInfo.imageView, img.depthImageInfo.imageView}, extent.width, extent.height, 1);

  auto rendergraph = vsg::RenderGraph::create();
  rendergraph->renderArea.offset = VkOffset2D{0, 0};
  rendergraph->renderArea.extent = extent;
  rendergraph->framebuffer = fbuf;

  rendergraph->clearValues.resize(2);
  rendergraph->clearValues[0].color = clearColour;
  rendergraph->clearValues[1].depthStencil = VkClearDepthStencilValue{1.0f, 0};

  return rendergraph; 
}

void submitVRFrames(vrhelp::Env* vr, vsg::Window* window)
{
  auto width = hmdImageLeft.width;
  auto height = hmdImageLeft.height;

  auto lImg = hmdImageLeft.colourImage->vk(window->getDevice()->deviceID);
  auto rImg = hmdImageRight.colourImage->vk(window->getDevice()->deviceID);

  vr->submitFrames(
    lImg,
    rImg,
    *window->getDevice(),
    *window->getPhysicalDevice(),
    *window->getInstance(),
    *window->getDevice()->getQueue(hmdCommandGraph->queueFamily),
    hmdCommandGraph->queueFamily,
    width,
    height,
    hmdImageFormat,
    VK_SAMPLE_COUNT_1_BIT
  );
}

vsg::ref_ptr<vsg::Camera> createCameraForScene(vsg::Node *scenegraph, const VkExtent2D &extent)
{
  // Create an initial camera - Both the desktop and hmd cameras are intialised like this
  // but their parameters will be updated each frame based on the hmd's pose/matrices
  vsg::ComputeBounds computeBounds;
  scenegraph->accept(computeBounds);
  vsg::dvec3 centre = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
  double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
  double nearFarRatio = 0.001;

  // set up the camera
  auto lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, 0.0, radius * 3.5),
                                    centre, vsg::dvec3(0.0, 1.0, 0.0));

  auto perspective = vsg::Perspective::create(30.0, static_cast<double>(extent.width) / static_cast<double>(extent.height),
                                              nearFarRatio * radius, radius * 4.5);

  return vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(extent));
}

vsg::ref_ptr<vsg::Viewer> initVSG(int argc, char **argv, vsg::ref_ptr<vsg::Group> sceneRoot, vrhelp::Env *vr)
{
  // Initialise the application, and desktop window
  auto windowTraits = vsg::WindowTraits::create();
  windowTraits->windowTitle = "VSG Test (With VR)";

  vsg::CommandLine arguments(&argc, argv);
  windowTraits->debugLayer = arguments.read({"--debug", "-d"});
  windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
  if (arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height))
  {
    windowTraits->fullscreen = false;
  }
  if (arguments.errors())
  {
    arguments.writeErrorMessages((std::cerr));
    return {};
  }

  // Check what extensions are needed by OpenVR
  // TODO: We need a physical device to check extensions, but also need a list of extensions before creating the window
  // Expect this needs to move down into say an HmdWindow class? For now use a dummy window just to get a device..
  {
    auto tempWindow = vsg::Window::create(vsg::WindowTraits::create());
    vrRequiredInstanceExtensions = vr->instanceExtensionsRequired();
    vrRequiredDeviceExtensions = vr->deviceExtensionsRequired(*tempWindow->getOrCreatePhysicalDevice());
    for( auto& ext : vrRequiredInstanceExtensions ) windowTraits->instanceExtensionNames.push_back(ext.c_str());
    for( auto& ext : vrRequiredDeviceExtensions ) windowTraits->deviceExtensionNames.push_back(ext.c_str());
  }

  // As this example renders the HMD images as part of the main desktop viewer it's important that vsync
  // be disabled. Otherwise the HMD's maximum framerate will be the same as the desktop monitor.
  // Other desktop-related settings may have an impact on headset output with this setup..
  windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

  // Load any vsg files into the scene graph
  vsg::Path path;
  for (auto i = 1; i < argc; ++i)
  {
    vsg::Path filename = arguments[i];
    path = vsg::filePath(filename);

    auto obj = vsg::read_cast<vsg::Node>(filename);
    if (obj)
    {
      sceneRoot->addChild(obj);
    }
  }

  auto worldNode = vsg::read_cast<vsg::Node>("world.vsgt");
  if( worldNode ) {
    sceneRoot->addChild(worldNode);
  } else {
    fmt::print("Failed to read world.vsgt\n");
  }

  // Create the viewer + desktop window
  auto viewer = vsg::Viewer::create();
  window = vsg::Window::create(windowTraits);
  if (!window)
  {
    std::cout << "Failed to create window" << std::endl;
    return {};
  }


  viewer->addWindow(window);
  viewer->addEventHandler(vsg::CloseHandler::create(viewer));

  // Create the framebuffers and render graph for HMD view
  // HMD is rendered through 2 cameras (bound to each eye), rendering into vulkan images, and presented through the vr api
  vsg::CompileTraversal compile(window);
  uint32_t hmdWidth = 0, hmdHeight = 0;
  vr->getRecommendedTargetSize(hmdWidth, hmdHeight);
  VkExtent2D hmdExtent{hmdWidth, hmdHeight};

  hmdCameraLeft = createCameraForScene(sceneRoot, hmdExtent);
  auto hmdRenderGraphLeft = createHmdRenderGraph(window->getDevice(), compile.context, hmdExtent, hmdImageLeft, window->clearColor());
  auto hmdViewLeft = vsg::View::create(hmdCameraLeft, sceneRoot);
  hmdRenderGraphLeft->addChild(hmdViewLeft);

  hmdCameraRight = createCameraForScene(sceneRoot, hmdExtent);  
  auto hmdRenderGraphRight = createHmdRenderGraph(window->getDevice(), compile.context, hmdExtent, hmdImageRight, window->clearColor());
  auto hmdViewRight = vsg::View::create(hmdCameraRight, sceneRoot);
  hmdRenderGraphRight->addChild(hmdViewRight);

  // Create render graph for desktop window
  desktopCamera = createCameraForScene(sceneRoot, window->extent2D());
  auto desktopRenderGraph = vsg::createRenderGraphForView(window, desktopCamera, sceneRoot);

  // Based on separateCommandGraph == true in vsgrendertotexture example
  // TODO: In an ideal scenario the desktop & hmd render passes would be independent,
  // and running at different framerates. Don't think that's possible with this setup.
  hmdCommandGraph = vsg::CommandGraph::create(window);
  hmdCommandGraph->addChild(hmdRenderGraphLeft);
  hmdCommandGraph->addChild(hmdRenderGraphRight);

  // TODO: If the desktop is a mirror of the headset, then it should be -> Should just blit the headset's images rather than a whole camera
  auto desktopCommandGraph = vsg::CommandGraph::create(window);
  desktopCommandGraph->addChild(desktopRenderGraph);

  viewer->assignRecordAndSubmitTaskAndPresentation({hmdCommandGraph, desktopCommandGraph});

  return viewer;
}

auto initVR(vsg::ref_ptr<vsg::Group> scene)
{
  std::unique_ptr<vrhelp::Env> env(new vrhelp::Env(vr::ETrackingUniverseOrigin::TrackingUniverseStanding));

  // Add controller models to the scene
  vsg::ref_ptr<vsg::Group> leftNode = vsg::MatrixTransform::create();
  vsg::ref_ptr<vsg::Group> rightNode = vsg::MatrixTransform::create();
  auto controllerNode = vsg::read_cast<vsg::Node>("controller.vsgt");
  if( controllerNode ) {
    leftNode->addChild(controllerNode);
    rightNode->addChild(controllerNode);
  } else {
    fmt::print("Failed to read controller.vsgt\n");
  }
  
  leftNode->setValue(OpenVRIDTag, LeftControllerID);
  rightNode->setValue(OpenVRIDTag, RightControllerID);
  
  scene->addChild(leftNode);
  scene->addChild(rightNode);

  return env;
}

// Take parameters from openvr, update the vsg scene elements
void updateSceneWithVRState(vrhelp::Env *vr, vsg::ref_ptr<vsg::Group> scene)
{
  auto left = vr->leftHand();
  auto right = vr->rightHand();
  auto hmd = vr->hmd();

  if (left)
  {
    if (left->mButtonPressed[vr::k_EButton_SteamVR_Trigger])
    {
      std::cerr << "Left trigger pressed\n";
      auto lPos = left->positionAbsolute();
      fmt::print("Left Position: {},{},{}\n", lPos[0], lPos[1], lPos[2]);
    }
  }
  if (right)
  {
    if (right->mButtonPressed[vr::k_EButton_SteamVR_Trigger])
    {
      std::cerr << "Right trigger pressed\n";
      auto rPos = right->positionAbsolute();
      fmt::print("Right Position: {},{},{}\n", rPos[0], rPos[1], rPos[2]);
    }
  }

  // Update models
  vsg::ref_ptr<vsg::Visitor> v(new UpdateVRVisitor(vr, left, right, hmd));
  scene->accept(*v);

  // VSG space is x right, z up, y forward
  // OpenVR space is x right, y up, z backward 
  // auto axesMat = vsg::dmat4(
  //   1.0, 0.0, 0.0, 0.0,
  //   0.0, 0.0, 1.0, 0.0,
  //   0.0, -1.0, 0.0, 0.0,
  //   0.0, 0.0, 0.0, 1.0  
  // );

  // auto axesMat2 = vsg::dmat4(
  //   1.0, 0.0, 0.0, 0.0,
  //   0.0, -1.0, 0.0, 0.0,
  //   0.0, 0.0, 1.0, 0.0,
  //   0.0, 0.0, 0.0, 1.0  
  // );

  // auto axesMatProj = vsg::dmat4(
  //   1.0, 0.0, 0.0, 0.0,
  //   0.0, 1.0, 0.0, 0.0,
  //   0.0, 0.0, 1.0, 0.0,
  //   0.0, 0.0, 0.0, 1.0  
  // );

  // Projection matrices are relatively simple
  // TODO: Are the 'raw' matrix classes needed any more? Can the mats be set directly in stock vsg?
  // TODO: There's something wrong with the projection matrices - hmd view is good but lighting acts weird. Maybe this needs an axes correction?
  float nearPlane = 0.001f;
  float farPlane = 10.0f;

  auto leftProj = vr->getProjectionMatrix(vr::EVREye::Eye_Left, nearPlane, farPlane);

  // auto leftProj = glm::perspective(90.0f, 1.0f, nearPlane, farPlane);

  vsg::ref_ptr<vsg::ProjectionMatrix> vsgProjLeft(new RawProjectionMatrix( /* axesMatProj */ openVRMatToVSG(leftProj)));
  hmdCameraLeft->setProjectionMatrix(vsgProjLeft);

  auto rightProj = vr->getProjectionMatrix(vr::EVREye::Eye_Right, nearPlane, farPlane);
  vsg::ref_ptr<vsg::ProjectionMatrix> vsgProjRight(new RawProjectionMatrix( /* axesMatProj */ openVRMatToVSG(rightProj)));
  hmdCameraRight->setProjectionMatrix(vsgProjRight);

  // View matrices for each eye
  auto hmdToWorld = hmd->deviceToAbsoluteMatrix();

  // glm::dvec3 scale;
  // glm::dquat rotation;
  // glm::dvec3 translation;
  // glm::dvec3 skew_;
  // glm::dvec4 perspective_;
  // glm::decompose(hmdToWorld, scale, rotation, translation, skew_, perspective_);

  // fmt::print("Hmd To World Translation: {:.2f},{:.2f},{:.2f}\n", translation.x, translation.y, translation.z);
  // fmt::print("Hmd To World Rotation: {:.2f},{:.2f},{:.2f}\n", rotation.x, rotation.y, rotation.z);

  auto worldToHmd = glm::inverse(hmdToWorld);
  // glm::decompose(worldToHmd, scale, rotation, translation, skew_, perspective_);

  // fmt::print("World To Hmd Translation: {:.2f},{:.2f},{:.2f}\n", translation.x, translation.y, translation.z);
  // fmt::print("World To Hmd Rotation: {:.2f},{:.2f},{:.2f}\n", rotation.x, rotation.y, rotation.z);

  auto vsgWorldToOVRWorld = vsg::rotate(- vsg::PI / 2.0, 1.0, 0.0, 0.0);
  vsg::dmat4 viewAxesMat(
    1, 0, 0, 0,
    0, -1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1);

  auto hmdToLeftEye = glm::inverse(vr->getEyeToHeadTransform(vr::EVREye::Eye_Right));
  auto hmdToRightEye = glm::inverse(vr->getEyeToHeadTransform(vr::EVREye::Eye_Right));
  auto viewMatLeft = viewAxesMat * openVRMatToVSG(hmdToLeftEye * worldToHmd) * vsgWorldToOVRWorld;
  auto viewMatRight = viewAxesMat * openVRMatToVSG(hmdToRightEye * worldToHmd) * vsgWorldToOVRWorld;

  hmdCameraLeft->setViewMatrix(vsg::ref_ptr<vsg::ViewMatrix>(new RawViewMatrix(viewMatLeft)));
  hmdCameraRight->setViewMatrix(vsg::ref_ptr<vsg::ViewMatrix>(new RawViewMatrix(viewMatRight)));

  // Bind the desktop camera to one of the eyes
  // TODO: This should probably just be a perspective camera, that happens to track the user's head,
  // doesn't need to be the actual projection used by each eye.
  desktopCamera->setProjectionMatrix(vsgProjLeft);
  desktopCamera->setViewMatrix(vsg::ref_ptr<vsg::ViewMatrix>(new RawViewMatrix(viewMatLeft)));
}

int main(int argc, char **argv)
{
  try
  {
    // The VSG scene, plus desktop window
    vsg::ref_ptr<vsg::Group> scene(new vsg::Group());

    // OpenVR context - TODO: Will terminate if vr isn't active
    auto vr = initVR(scene);

    auto viewer = initVSG(argc, argv, scene, vr.get());
    if (!viewer)
      return EXIT_FAILURE;

    // Setup initial state
    updateSceneWithVRState(vr.get(), scene);

    viewer->compile();

    // Render loop
    while (viewer->advanceToNextFrame())
    {
      // Input handling for VSG
      viewer->handleEvents();

      // OpenVR state update
      vr->update();

      // TODO: This is far from the optimal approach, but simple to use for now
      // TODO: Update VSG state for controller positions/etc
      updateSceneWithVRState(vr.get(), scene);

      // Other VSG updates
      viewer->update();

      // TODO: Update render passes for left/right eyes here
      // VSG render + present to desktop window
      viewer->recordAndSubmit();
      viewer->present();

      // VR presentation
      submitVRFrames(vr.get(), viewer->windows().front());

      // TODO: The pose updates here aren't great. This should be updated
      // to be async, using 'explicit timings'
      // https://github.com/ValveSoftware/openvr/wiki/Vulkan#explicit-timing
      // https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetDeviceToAbsoluteTrackingPose
      vr->waitGetPoses();
    }
  }
  catch (vsg::Exception& e)
  {
    fmt::print("Exception: {}\n", e.message);
  }
  catch (std::exception& e)
  {
    fmt::print("Exception: {}\n", e.what());
  }

  return EXIT_SUCCESS;
}
