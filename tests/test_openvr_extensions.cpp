#include <vsgvr/openvr/OpenVRContext.h>

#include <chrono>
#include <memory>
#include <iostream>
#include <vsg/all.h>

int main(void) {
  vsgvr::OpenVRContext vr(vsgvr::VRContext::TrackingOrigin::Standing);
  auto instanceExtensions = vr.instanceExtensionsRequired();
  std::cout << "Instance Extensions Required:\n";
  for (auto &ext : instanceExtensions) {
    std::cout << "\t" << ext << "\n";
  }

  auto windowTraits = vsg::WindowTraits::create();
  auto window = vsg::Window::create(windowTraits);
  if (!window) {
    std::cout << "Failed to create window\n";
    return EXIT_FAILURE;
  }

  auto physDev = window->getOrCreatePhysicalDevice();
  if (!physDev) {
    std::cout << "Failed to create physical device\n";
    return EXIT_FAILURE;
  }

  auto deviceExtensions = vr.deviceExtensionsRequired(*physDev);
  std::cout << "Device Extensions Required:\n";
  for (auto &ext : deviceExtensions) {
    std::cout << "\t" << ext << "\n";
  }

  // Create a window for real, with all the required extensions
  for (auto &ext : instanceExtensions)
    windowTraits->instanceExtensionNames.push_back(ext.c_str());
  for (auto &ext : deviceExtensions)
    windowTraits->deviceExtensionNames.push_back(ext.c_str());
  window = vsg::Window::create(windowTraits);

  std::this_thread::sleep_for(std::chrono::seconds(5));
  return EXIT_SUCCESS;
}
