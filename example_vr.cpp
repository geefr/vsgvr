#include <vsg/all.h>

#include <vsgvr/OpenXRInstance.h>

#include <iostream>

int main(int argc, char **argv) {
  try
  {
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO
    // related options to use when reading and writing files.
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
    // TODO: At the moment traits must be configured up front, exceptions will be thrown if these can't be satisfied
    auto xrTraits = vsgvr::OpenXrTraits();
    xrTraits.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    xrTraits.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    auto vkTraits = vsgvr::OpenXrVulkanTraits();

    vkTraits.vulkanDebugLayer = arguments.read({ "--debug", "-d" });
    vkTraits.vulkanApiDumpLayer = arguments.read({ "--api", "-a" });
    
    auto controllerNodeLeft = vsg::read_cast<vsg::Node>("controller.vsgt");
    auto controllerNodeRight = vsg::read_cast<vsg::Node>("controller2.vsgt");

    // TODO: Instantiate devices and nodes in scene
    // TODO: If controllers are off when program starts they won't be added later
    // - This needs a better interface for controller poses - Ideally common between vr/xr apis, but xr does actions/spaces differently
    // vsgvr::createDeviceNodes(vr, vsg_scene, controllerNodeLeft, controllerNodeRight);

    // The connection to OpenXR, equivalent to a vsg::Viewer
    auto vr = vsgvr::OpenXRInstance::create(xrTraits, vkTraits);

    // add the CommandGraph to render the scene
    auto commandGraphs = vr->createCommandGraphsForView(vsg_scene, true);
    vr->assignRecordAndSubmitTaskAndPresentation(commandGraphs);

    // compile all Vulkan objects and transfer image, vertex and primitive data to GPU
    vr->compile();
    
    // Render loop
    for(;;)
    {
      auto pol = vr->pollEvents();
      if( pol == vsgvr::OpenXRInstance::PollEventsResult::Exit)
      {
        // User exited through VR overlay / XR runtime
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
      if (vr->advanceToNextFrame())
      {
        if (pol == vsgvr::OpenXRInstance::PollEventsResult::RunningDontRender)
        {
          // XR Runtime requested that rendering is not performed (not visible to user)
        }
        else
        {
          // Render a frame
          vr->update();
          vr->recordAndSubmit();
        }

        // End the frame, and present to user
        vr->releaseFrame();
      }
    }

    return EXIT_SUCCESS;
  }
  catch( const vsg::Exception& e )
  {
    std::cout << "VSG Exception: " << e.message << std::endl;
    return EXIT_FAILURE;
  }
}
