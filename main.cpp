
#include <vsg/all.h>

#include <openvr/openvr.h>
#include "vr/env.h"
#include "vr/device.h"
#include "vr/controller.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <memory>
#include <string>

#include "models/controller/model_controller.cpp"
#include "models/floorpad/model_floorpad.cpp"

const std::string OpenVRIDTag = "OpenVR ID";
const std::string LeftControllerID = "Left Controller";
const std::string RightControllerID = "Right Controller";

vsg::ref_ptr<vsg::Window> window;
vsg::ref_ptr<vsg::Options> options;
vsg::ref_ptr<vsg::Camera> hmdCamera;
vsg::ref_ptr<vsg::Camera> desktopCamera;

struct HMDImage
{
  vsg::ImageInfo colourImageInfo;
  vsg::ref_ptr<vsg::Image> colourImage;
  vsg::ImageInfo depthImageInfo;
  vsg::ref_ptr<vsg::Image> depthImage;
};
HMDImage hmdImageLeft;
HMDImage hmdImageRight;

auto imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

vsg::dmat4 openVRMatToVSG( glm::dmat4 mat ) {
  vsg::dmat4 result( glm::value_ptr(mat) );
  return result;
}

class RawProjectionMatrix : public vsg::ProjectionMatrix
{
public:
  RawProjectionMatrix( vsg::dmat4 mat ) : mMat(mat) {}
  void get(vsg::mat4& matrix) const override { matrix = mMat; }
  void get(vsg::dmat4& matrix) const override { matrix = mMat; }
private:
  vsg::dmat4 mMat;
};

class RawViewMatrix : public vsg::ViewMatrix
{
public:
  RawViewMatrix( vsg::dmat4 mat ) : mMat(mat) {}
  void get(vsg::mat4& matrix) const override { matrix = mMat; }
  void get(vsg::dmat4& matrix) const override { matrix = mMat; }
private:
  vsg::dmat4 mMat;
};

class UpdateVRVisitor : public vsg::Visitor
{
public:
  UpdateVRVisitor() = delete;
  UpdateVRVisitor(vrhelp::Env* vr, vrhelp::Controller* left, vrhelp::Controller* right, vrhelp::Device* hmd)
    : mVr(vr), mLeft(left), mRight(right), mHmd(hmd) {}

  virtual void apply(vsg::Group& o) override {
    std::string v;
    if( mLeft ) {
      if( o.getValue(OpenVRIDTag, v) && v == LeftControllerID ) {
	if( auto txf = o.cast<vsg::MatrixTransform>() ) {
  	  auto mat = mLeft->deviceToAbsoluteMatrix();
          vsg::dmat4 vmat(glm::value_ptr(mat));
          txf->setMatrix(vmat);
	}
      }
    }
    if( mRight ) {
      if( o.getValue(OpenVRIDTag, v) && v == RightControllerID ) {
	if( auto txf = o.cast<vsg::MatrixTransform>() ) {
  	  auto mat = mRight->deviceToAbsoluteMatrix();
          vsg::dmat4 vmat(glm::value_ptr(mat));
          txf->setMatrix(vmat);
	}
      }
    }
  }

  
private:
  vrhelp::Env* mVr = nullptr;
  vrhelp::Controller* mLeft = nullptr;
  vrhelp::Controller* mRight = nullptr;
  vrhelp::Device* mHmd = nullptr;
};

class SubmitOpenVRCommand : public vsg::Command
{
public:
	SubmitOpenVRCommand(vrhelp::Env* vr)
	  : mVr(vr)
	{}

	void read(vsg::Input& input) override {}
	void write(vsg::Output& output) const override {}

	void compile(vsg::Context& context) override {}

	void record(vsg::CommandBuffer& commandBuffer) const override {
	  // Submit to SteamVR
		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0f;
		bounds.uMax = 1.0f;
		bounds.vMin = 0.0f;
		bounds.vMax = 1.0f;
		
		vr::VRVulkanTextureData_t vulkanData;
		vulkanData.m_nImage = ( uint64_t ) colourImage.imageView->vk(window->getOrCreateDevice()->deviceID);
		vulkanData.m_pDevice = ( VkDevice_T * ) window->getOrCreateDevice()->getDevice();
		vulkanData.m_pPhysicalDevice = ( VkPhysicalDevice_T * ) window->getOrCreateDevice()->getPhysicalDevice()->getPhysicalDevice();
		vulkanData.m_pInstance = ( VkInstance_T *) window->getOrCreateDevice()->getInstance()->getInstance(); 
		vulkanData.m_pQueue = ( VkQueue_T * ) window->getOrCreateDevice()->getQueue(0)->queue(); // TODO: use the correct queue ID
		vulkanData.m_nQueueFamilyIndex = 0; // TODO: use the correct queue ID

    uint32_t hmdWidth=0, hmdHeight=0;
    mVr->getRecommendedTargetSize(hmdWidth, hmdHeight);
  
		vulkanData.m_nWidth = hmdWidth;
		vulkanData.m_nHeight = hmdHeight;
		vulkanData.m_nFormat = imageFormat;
		vulkanData.m_nSampleCount = 1;
		
		vr::Texture_t texture = { &vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto };
		vr::VRCompositor()->Submit( vr::Eye_Left, &texture, &bounds ); 
		
		// vulkanData.m_nImage = ( uint64_t ) m_rightEyeDesc.m_pImage;
		vulkanData.m_nImage = ( uint64_t ) colourImage.imageView->vk(window->getOrCreateDevice()->deviceID);
		vr::VRCompositor()->Submit( vr::Eye_Right, &texture, &bounds );
	}
	
protected:
	virtual ~SubmitOpenVRCommand() {}
	vrhelp::Env* mVr = nullptr;
};

/// Create the render graph for the headset - Rendering into an image, which will be handed off to openvr
vsg::ref_ptr<vsg::RenderGraph> createHmdRenderGraph(vsg::Device* device, vsg::Context& context, const VkExtent2D& extent, HMDImage& img)
{
    VkExtent3D attachmentExtent{extent.width, extent.height, 1};
    // Attachments
    // create image for color attachment
    img.colourImage = vsg::Image::create();
    img.colourImage->imageType = VK_IMAGE_TYPE_2D;
    img.colourImage->extent = attachmentExtent;
    img.colourImage->mipLevels = 1;
    img.colourImage->arrayLayers = 1;
    img.colourImage->format = imageFormat;
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
    img.depthImageInfo.imageView = vsg::createImageView(context, depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
    img.depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    // attachment descriptions
    vsg::RenderPass::Attachments attachments(2);
    // Color attachment
    attachments[0].format = imageFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
    auto fbuf = vsg::Framebuffer::create(renderPass, vsg::ImageViews{colorImageInfo.imageView, depthImageInfo.imageView}, extent.width, extent.height, 1);

    auto rendergraph = vsg::RenderGraph::create();
    rendergraph->renderArea.offset = VkOffset2D{0, 0};
    rendergraph->renderArea.extent = extent;
    rendergraph->framebuffer = fbuf;

    rendergraph->clearValues.resize(2);
    rendergraph->clearValues[0].color = { {0.4f, 0.2f, 0.4f, 1.0f} };
    rendergraph->clearValues[1].depthStencil = VkClearDepthStencilValue{1.0f, 0};

    return rendergraph;
}

vsg::ref_ptr<vsg::Camera> createCameraForScene(vsg::Node* scenegraph, const VkExtent2D& extent)
{
	// TODO: camera parameters will be replaced quickly with the hmd's parameters,
	// so most of this is redundant
	
    // compute the bounds of the scene graph to help position camera
    vsg::ComputeBounds computeBounds;
    scenegraph->accept(computeBounds);
    vsg::dvec3 centre = (computeBounds.bounds.min+computeBounds.bounds.max)*0.5;
    double radius = vsg::length(computeBounds.bounds.max-computeBounds.bounds.min)*0.6;
    double nearFarRatio = 0.001;

    // set up the camera
    auto lookAt = vsg::LookAt::create(centre+vsg::dvec3(0.0, 0.0, radius * 3.5),
                                      centre, vsg::dvec3(0.0, 1.0, 0.0));

    auto perspective = vsg::Perspective::create(30.0, static_cast<double>(extent.width) / static_cast<double>(extent.height),
                                                nearFarRatio*radius, radius * 4.5);

    return vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(extent));
}

vsg::ref_ptr<vsg::Viewer> initVSG(int argc, char** argv, vsg::ref_ptr<vsg::Group> sceneRoot, vrhelp::Env* vr) {
  // vsg default args + out custom ones
  options = vsg::Options::create();

  auto windowTraits = vsg::WindowTraits::create();
  windowTraits->windowTitle = "VSG_VR_Test";

  vsg::CommandLine arguments(&argc, argv);
  arguments.read(options);
  windowTraits->debugLayer = arguments.read({"--debug","-d"});
  windowTraits->apiDumpLayer = arguments.read({"--api","-a"});
  if (arguments.read("--double-buffer")) windowTraits->swapchainPreferences.imageCount = 2;
  if (arguments.read("--triple-buffer")) windowTraits->swapchainPreferences.imageCount = 3; // default
  if (arguments.read("--IMMEDIATE")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  if (arguments.read("--FIFO")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  if (arguments.read("--FIFO_RELAXED")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
  if (arguments.read("--MAILBOX")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
  if (arguments.read({"-t", "--test"})) { windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; windowTraits->fullscreen = true; }
  if (arguments.read({"--st", "--small-test"})) { windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; windowTraits->width = 192, windowTraits->height = 108; windowTraits->decoration = false; }
  if (arguments.read({"--fullscreen", "--fs"})) windowTraits->fullscreen = true;
  if (arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height)) { windowTraits->fullscreen = false; }
  if (arguments.read({"--no-frame", "--nf"})) windowTraits->decoration = false;
  if (arguments.read("--or")) windowTraits->overrideRedirect = true;
  if (arguments.read("--d32")) windowTraits->depthFormat = VK_FORMAT_D32_SFLOAT;
  arguments.read("--screen", windowTraits->screenNum);
  arguments.read("--display", windowTraits->display);
  arguments.read("--samples", windowTraits->samples);
  if( arguments.errors() ) {
    arguments.writeErrorMessages((std::cerr));
    return {};
  }

  // Load any vsg files into the scene graph
  vsg::Path path;
  for( auto i = 1; i < argc; ++i )
  {
    vsg::Path filename = arguments[i];
    path = vsg::filePath(filename);

    auto obj = vsg::read(filename, options);
    if( auto node = obj.cast<vsg::Node>() )
    {
      // It's a vsgt/other supported format
      sceneRoot->addChild(node);
    }
    else if( obj )
    {
      std::cout << "Unable to load object of type " << obj->className() << std::endl;
    }
    else
    {
      std::cout << "Unable to load model from file " << filename << std::endl;
    }
  }

  sceneRoot->addChild(model_floorpad());

  /*
  if( sceneRoot->getNumChildren() == 0 )
  {
    std::cout << "No model specified on command line" << std::endl;
    return {};
  }
  */

  // Create the viewer + window
  auto viewer = vsg::Viewer::create();
  window = vsg::Window::create(windowTraits);
  if( !window )
  {
    std::cout << "Failed to create window" << std::endl;
    return {};
  }

  viewer->addWindow(window);
  // add close handler to respond the close window button and pressing escape
  viewer->addEventHandler(vsg::CloseHandler::create(viewer));


  // Create the framebuffers and render graph for HMD view
  // TODO: Will need one of these for each eye
  vsg::CompileTraversal compile(window);
  uint32_t hmdWidth=0, hmdHeight=0;
  vr->getRecommendedTargetSize(hmdWidth, hmdHeight);
  VkExtent2D hmdExtent{hmdWidth, hmdHeight};
  hmdCamera = createCameraForScene(sceneRoot, hmdExtent);
  
  auto hmdRenderGraph = createOffscreenRendergraph(window, compile.context, hmdExtent, colourImage, depthImage);
  auto hmdView = vsg::View::create(hmdCamera, sceneRoot);
  hmdRenderGraph->addChild(hmdView);
  

  // Create render graph for desktop window
  desktopCamera = createCameraForScene(sceneRoot, window->extent2D());
  auto desktopRenderGraph = vsg::createRenderGraphForView(window, desktopCamera, sceneRoot);

  // separateCommandGraph == true in vsgrendertotexture example
  // TODO: is the window required here? Would we just want the 2 eye renders tied together, and render the desktop window entirely separately?
  auto hmdCommandGraph = vsg::CommandGraph::create(window);
  hmdCommandGraph->addChild(hmdRenderGraph);
  vsg::ref_ptr<vsg::Command> hmdSubmitCommand(new SubmitOpenVRCommand(vr));
  hmdCommandGraph->addChild(hmdSubmitCommand);
  auto desktopCommandGraph = vsg::CommandGraph::create(window);
  desktopCommandGraph->addChild(desktopRenderGraph);

  viewer->assignRecordAndSubmitTaskAndPresentation({hmdCommandGraph, desktopCommandGraph});

  return viewer;
}

auto initVR( vsg::ref_ptr<vsg::Group> scene ) {
  std::unique_ptr<vrhelp::Env> env(new vrhelp::Env(vr::ETrackingUniverseOrigin::TrackingUniverseStanding));

  // Create framebuffers/render targets for each eye
  // TODO: Will need to create through osg
  // but in a format openvr can handle
  // and in a way we can access the vulkan handles, and otherwise control presentation

  // Add controller models to the scene
  vsg::ref_ptr<vsg::Group> leftNode = vsg::MatrixTransform::create();
  leftNode->addChild(model_controller());
  leftNode->setValue(OpenVRIDTag, LeftControllerID);
  scene->addChild(leftNode);

  vsg::ref_ptr<vsg::Group> rightNode = vsg::MatrixTransform::create();
  rightNode->addChild(model_controller());
  rightNode->setValue(OpenVRIDTag, RightControllerID);
  scene->addChild(rightNode);

  return env;
}

void updateSceneWithVRState( vrhelp::Env* vr, vsg::ref_ptr<vsg::Group> scene ) {
  auto left = vr->leftHand();
  auto right = vr->rightHand();
  auto hmd = vr->hmd();

  if( left ) {
    if( left->mButtonPressed[vr::k_EButton_SteamVR_Trigger] ) {
      std::cerr << "Left trigger pressed\n";
    }
  }
  if( right ) {
    if( right->mButtonPressed[vr::k_EButton_SteamVR_Trigger] ) {
      std::cerr << "Right trigger pressed\n";
    }
  }

  // Update models
  auto v = new UpdateVRVisitor(vr, left, right, hmd);
  scene->accept(*v);

  // Position the camera based on HMD location
  glm::dmat4x4 hmdPoseMat(1.f);
  glm::dmat4x4 hmdPoseMatInv(1.f);

  // The headset's position in the world
  hmdPoseMat = hmd->deviceToAbsoluteMatrix();
  hmdPoseMatInv = glm::inverse(hmdPoseMat);

  // TODO: View/Projection matrices are a little tricky here due to the
  // coordinate space adjustment...
  float nearPlane = 0.001f;
  float farPlane = 10.0f;
  auto rightProj = vr->getProjectionMatrix(vr::EVREye::Eye_Right, nearPlane, farPlane);
  vsg::ref_ptr<vsg::ProjectionMatrix> vsgProj(new RawProjectionMatrix(openVRMatToVSG(rightProj)));
  
  // TODO: Obviously want to use the right parameters here
  desktopCamera->setProjectionMatrix(vsgProj);
  hmdCamera->setProjectionMatrix(vsgProj);
  
  //auto radius = 2.0;
  //vsg::ref_ptr<vsg::ProjectionMatrix> perspective = vsg::Perspective::create(30.0, 800.0 / 600.0, nearPlane / farPlane * radius, radius * 4.5);

  auto rightEyeFromHead = glm::inverse(vr->getEyeToHeadTransform(vr::EVREye::Eye_Right));
  //vsg::dmat4 axesMat(
  //  1, 0, 0, 0,
  //  0, 0, 1, 0,
  //  0, -1, 0, 0,
  //  0, 0, 0, 1);
  auto viewMat = openVRMatToVSG(rightEyeFromHead * hmdPoseMatInv); // * axesMat;

  vsg::ref_ptr<vsg::ViewMatrix> vsgView(new RawViewMatrix(viewMat));
  desktopCamera->setViewMatrix(vsgView);
  hmdCamera->setViewMatrix(vsgView);
}

int main(int argc, char** argv) {
  // The VSG scene, plus desktop window
  
  vsg::dmat4 axesMatNone(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1);
  vsg::dmat4 axesMatZY(
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, -1, 0, 0,
    0, 0, 0, 1);
  vsg::dmat4 axesMatTest(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1);
  vsg::ref_ptr<vsg::Group> scene(new vsg::MatrixTransform(axesMatTest));
  
  // vsg::ref_ptr<vsg::Group> scene(new vsg::Group());
  
  // OpenVR context - TODO: Will terminate if vr isn't active
  auto vr = initVR(scene);
  
  auto viewer = initVSG(argc, argv, scene, vr.get());
  if( !viewer ) return EXIT_FAILURE;

  // Setup initial state
  updateSceneWithVRState( vr.get(), scene );

  viewer->compile();
  
  // viewer->setupThreading();

  // Render loop
  while(viewer->advanceToNextFrame())
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
    // TODO
    // vr->submitFrames( leftTexID, rightTexID );
    // TODO: The pose updates here aren't great. Should be as async as possible, 
    // and should ask for poses x seconds into the future, 
    // when photons will be shot into the users face.
    // https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetDeviceToAbsoluteTrackingPose
    vr->waitGetPoses();
    
    // viewer->deviceWaitIdle();
  }

  return EXIT_SUCCESS;
}

