#include <vsg/all.h>

#include <vsgvr/vsgvr.h>
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
    
    auto controllerNodeLeft = vsg::read_cast<vsg::Node>("controller.vsgt");
    auto controllerNodeRight = vsg::read_cast<vsg::Node>("controller2.vsgt");

    // TODO: Instantiate devices and nodes in scene
    // - This needs a better interface for controller poses - Ideally common between vr/xr apis, but xr does actions/spaces differently
    // vsgvr::createDeviceNodes(vr, vsg_scene, controllerNodeLeft, controllerNodeRight);

    // The connection to OpenXR, equivalent to a vsg::Viewer
    auto vr = vsgvr::OpenXRInstance::create(xrTraits, vkTraits);

    // add close handler to respond the close window button and pressing escape
    // viewer->addEventHandler(vsg::CloseHandler::create(viewer));

    /*
    // add the CommandGraph to render the scene
    auto commandGraphs = vr->createCommandGraphsForView(vsg_scene);
    vr->assignRecordAndSubmitTaskAndPresentation(commandGraphs);

    // compile all Vulkan objects and transfer image, vertex and primitive data to GPU
    vr->compile();
    */

    // Render loop
    for(;;)
    {
      auto pol = vr->pollEvents();
      if( pol == vsgvr::OpenXRInstance::PollEventsResult::Exit)
      {
        break;
      }

      if( pol == vsgvr::OpenXRInstance::PollEventsResult::NotRunning)
      {
        continue;
      }
      else if (pol == vsgvr::OpenXRInstance::PollEventsResult::RuntimeIdle)
      {
        // Reduce power usage, wait for XR to wake
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }

      // The session is running, and a frame must be processed
      auto frameState = vr->advanceToNextFrame();

      if (pol == vsgvr::OpenXRInstance::PollEventsResult::RunningDontRender &&
          frameState.shouldRender == false)
      {
        // TODO: Used && above, as the SteamVR nullHMD seems to enter synchronised,
        // but also ask for render, without transitioning to Visible or Focused.
        // Not sure if that's inline with spec but xrWaitFrame does ask for render
        // all the same.

        // Application is awaiting session synchronisation, or otherwise should
        // not render anything. Frames must still be acquired and released.
      }
      else // if (pol == vsgvr::OpenXRInstance::PollEventsResult::RunningDoRender)
      {
        // HACK HACK HACK
        // add the CommandGraph to render the scene
        auto commandGraphs = vr->createCommandGraphsForView(vsg_scene);
        vr->assignRecordAndSubmitTaskAndPresentation(commandGraphs);

        // compile all Vulkan objects and transfer image, vertex and primitive data to GPU
        vr->compile();
        // HACK HACK HACK

        vr->update();
        vr->recordAndSubmit();
      }

      // (Viewer::present())
      vr->releaseFrame(frameState);
    }

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
