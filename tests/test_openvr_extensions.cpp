#include "vr/env.h"

#include <vsg/all.h>
#include <fmt/core.h>
#include <memory>
#include <chrono>

int main(void) {
    vrhelp::Env vr(vr::ETrackingUniverseOrigin::TrackingUniverseStanding);
    auto instanceExtensions = vr.instanceExtensionsRequired();
    fmt::print("Instance Extensions Required:\n");
    for( auto& ext : instanceExtensions ) {
        fmt::print("\t{}\n", ext);
    }

    auto windowTraits = vsg::WindowTraits::create();
    auto window = vsg::Window::create(windowTraits);
    if (!window)
    {
        fmt::print("Failed to create window\n");
        return EXIT_FAILURE;
    }

    auto physDev = window->getOrCreatePhysicalDevice();
    if( !physDev ) {
        fmt::print("Failed to create physical device\n");
        return EXIT_FAILURE;
    }

    auto deviceExtensions = vr.deviceExtensionsRequired(*physDev);
    fmt::print("Device Extensions Required:\n");
    for( auto& ext : deviceExtensions ) {
        fmt::print("\t{}\n", ext);
    }

    // Create a window for real, with all the required extensions
    for( auto& ext : instanceExtensions ) windowTraits->instanceExtensionNames.push_back(ext.c_str());
    for( auto& ext : deviceExtensions ) windowTraits->deviceExtensionNames.push_back(ext.c_str());
    window = vsg::Window::create(windowTraits);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    return EXIT_SUCCESS;
}
