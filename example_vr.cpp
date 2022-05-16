#include <vsg/all.h>

#include <vsgvr/openxr/OpenXRInstance.h>
// #include <vsgvr/openxr/OpenXRViewer.h>

int main(int argc, char **argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // realted options to use when reading and writing files.
    auto options = vsg::Options::create();
    arguments.read(options);

    vsg::Path filename = "world.vsgt";
    if (argc > 1)
      filename = arguments[1];
    if (arguments.errors())
      return arguments.writeErrorMessages(std::cerr);

    // load the scene graph
    vsg::ref_ptr<vsg::Group> vsg_scene =
        vsg::read_cast<vsg::Group>(filename, options);
    if (!vsg_scene)
      return 0;

    // Initialise vr, and add nodes to the scene graph for each tracked device
    // TODO: If controllers are off when program starts they won't be added later
    auto xrTraits = vsgvr::OpenXrTraits();
    auto vkTraits = vsgvr::OpenXrVulkanTraits();
    auto vr = vsgvr::OpenXRInstance(xrTraits, vkTraits);

    auto controllerNodeLeft = vsg::read_cast<vsg::Node>("controller.vsgt");
    auto controllerNodeRight = vsg::read_cast<vsg::Node>("controller2.vsgt");

    // TODO: Instantiate devices and nodes in scene - Need a new system for this, more generic, nicer, etc?
    // vsgvr::createDeviceNodes(vr, vsg_scene, controllerNodeLeft, controllerNodeRight);

    // auto viewer = vsgvr::OpenXRViewer::create(vr, windowTraits);

    // add close handler to respond the close window button and pressing escape
    // viewer->addEventHandler(vsg::CloseHandler::create(viewer));

    // add the CommandGraph to render the scene
    //auto commandGraphs = viewer->createCommandGraphsForView(vsg_scene);
    //viewer->assignRecordAndSubmitTaskAndPresentation(commandGraphs);

    //// compile all Vulkan objects and transfer image, vertex and primitive data to GPU
    //viewer->compile();

    // Render loop
    //while (viewer->advanceToNextFrame()) {
    //  viewer->handleEvents();
    //  viewer->update();

    //  viewer->renderXR();
    //  // viewer->present();
    //}

    return EXIT_SUCCESS;
  }
  catch( const vsg::Exception& e )
  {
    std::cout << "VSG Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
}
