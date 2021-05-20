
#include <vsg/all.h>

#include <iostream>
#include <memory>
#include <string>

#include "models/controller/model_controller.cpp"
#include "models/floorpad/model_floorpad.cpp"

vsg::ref_ptr<vsg::Window> window;
vsg::ref_ptr<vsg::Camera> desktopCamera;

vsg::ref_ptr<vsg::Camera> createCameraForScene(vsg::Node *scenegraph, const VkExtent2D &extent)
{
  // compute the bounds of the scene graph to help position camera
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

vsg::ref_ptr<vsg::Viewer> initVSG(int argc, char **argv, vsg::ref_ptr<vsg::Group> sceneRoot)
{
  auto windowTraits = vsg::WindowTraits::create();
  windowTraits->windowTitle = "VSG Test (No VR)";

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

  sceneRoot->addChild(model_floorpad());

  // Create the viewer + desktop window
  auto viewer = vsg::Viewer::create();
  window = vsg::Window::create(windowTraits);
  if (!window)
  {
    std::cout << "Failed to create window" << std::endl;
    return {};
  }

  viewer->addWindow(window);
  // add close handler to respond the close window button and pressing escape
  viewer->addEventHandler(vsg::CloseHandler::create(viewer));

  // Create render graph for desktop window
  desktopCamera = createCameraForScene(sceneRoot, window->extent2D());
  auto desktopRenderGraph = vsg::createRenderGraphForView(window, desktopCamera, sceneRoot);

  auto desktopCommandGraph = vsg::CommandGraph::create(window);
  desktopCommandGraph->addChild(desktopRenderGraph);

  viewer->assignRecordAndSubmitTaskAndPresentation({desktopCommandGraph});

  return viewer;
}

int main(int argc, char **argv)
{
  try
  {
    vsg::ref_ptr<vsg::Group> scene(new vsg::Group());

    auto viewer = initVSG(argc, argv, scene);
    if (!viewer)
      return EXIT_FAILURE;

    viewer->compile();

    // Render loop
    while (viewer->advanceToNextFrame())
    {
      // Input handling for VSG
      viewer->handleEvents();

      // Other VSG updates
      viewer->update();

      // TODO: Update render passes for left/right eyes here
      // VSG render + present to desktop window
      viewer->recordAndSubmit();
      viewer->present();
    }
  }
  catch (vsg::Exception &e)
  {
    std::cerr << "Exception: " << e.message << std::endl;
  }

  return EXIT_SUCCESS;
}
