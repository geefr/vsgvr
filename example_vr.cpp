#include <vsg/all.h>

#include <vsgvr/VRViewer.h>
#include <vsgvr/openvr/OpenVRContext.h>

#include <iostream>

int main(int argc, char **argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // realted options to use when reading and writing files.
    auto options = vsg::Options::create();
    arguments.read(options);

   // add relative data path based on binary dir
    vsg::Path dataPath = vsg::filePath(vsg::executableFilePath());
    dataPath.append(VSGVR_REL_DATADIR);
    options->paths.push_back(dataPath);
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
    auto vr = vsgvr::OpenVRContext::create();
    auto controllerNodeLeft = vsg::read_cast<vsg::Node>("controller.vsgt", options);
    auto controllerNodeRight = vsg::read_cast<vsg::Node>("controller2.vsgt", options);
    vsgvr::createDeviceNodes(vr, vsg_scene, controllerNodeLeft, controllerNodeRight);

    // Create the VR Viewer
    // This viewer creates its own desktop window internally along
    // with cameras, event handlers, etc
    // The provided window traits are a template - The viewer will configure
    // any additional settings as needed (such as disabling v-sync)
    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "VSGVR Example";
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
    if (arguments.read({"--window", "-w"}, windowTraits->width,
                      windowTraits->height)) {
      windowTraits->fullscreen = false;
    }

    auto viewer = vsgvr::VRViewer::create(vr, windowTraits);

    // add close handler to respond the close window button and pressing escape
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));

    // add the CommandGraph to render the scene
    auto commandGraphs = viewer->createCommandGraphsForView(vsg_scene);
    viewer->assignRecordAndSubmitTaskAndPresentation(commandGraphs);

    // compile all Vulkan objects and transfer image, vertex and primitive data to
    // GPU
    viewer->compile();

    // Render loop
    while (viewer->advanceToNextFrame()) {
      viewer->handleEvents();
      viewer->update();
      viewer->recordAndSubmit();
      viewer->present();
    }

    return EXIT_SUCCESS;
  }
  catch( const vsg::Exception& e )
  {
    std::cout << "VSG Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
}
