# VSG OpenXR Integration

(prototype) VR support for [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph) via OpenXR.

## Status

Basic rendering and presentation is functional, presented to application through an OpenXRViewer class, similar to the desktop vsg::Viewer.

Pose bindings are available, to allow models to be bound to controller positions or similar.

The basic `example_vr.cpp` application is similar to the desktop vsg Viewer application, and some test models are provided.

[![Demo Video](http://img.youtube.com/vi/ZA7syEMAIMo/0.jpg)](http://www.youtube.com/watch?v=ZA7syEMAIMo "vsgvr Demo Video")

### Supported Hardware

In theory, any OpenXR compatible hardware or systems are supported.

The following configurations have been tested:
* Rendering to a virtual HMD in Monado, on Linux
* Rendering to SteamVR (HTC Vive), on Linux, and Windows

Feature                      | Status
-----------------------------|--------
Linux Build                  | Done
Windows Build                | Done
Android Build                | Builds, but untested, no example application
Code quality / API           | Messy, but in roughly the right structure for now
OpenXR Rendering             | Working, could do with a cleanup and better vsg integration at some point
OpenXR Input                 | Not Implemented Yet
Controller tracking          | Working
Controller models in scene   | Working
HMD tracking                 | Working
Desktop view                 | Working


## Setup

If you don't have a VR headset there's a couple of options.

### SteamVR Mock HMD

SteamVR can be configured to simulate a HMD - This allows testing of rendering, though doesn't provide controller or headset tracking.

See here - Force a null driver to allow basic display output:
https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings

### Monado

Monado can run on any Linux box pretty much, just install through your package manager and run the following.

This should display a preview of the headset, along with a few controls to simulate headset motion.

```sh
export QWERTY_ENABLE=1
export OXR_DEBUG_GUI=1
export XRT_COMPOSITOR_FORCE_XCB=1
rm /tmp/monado_comp_ipc
monado-service
```

### Android Phone / Tablet

TODO: Once the Android build is functional, it should be possible to use most phones/tablets as an XR display.
These don't directly match how a VR setup works, but should provide basic hardware for rotation/positional tracking (ARCore).


## Compilation

Required:
* cmake > 3.14
* vulkan sdk
* VulkanSceneGraph
* The OpenXR loader - Included as a git submodule in deps/openxr
* (For model creation) vsgXchange

```sh
# Ensure submodules are available
git submodule update --init

# The usual CMake build
# Instead of CMAKE_PREFIX_PATH, you may set CMAKE_INSTALL_PREFIX to the same as your VulkanSceneGraph project to locate VSG
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="VulkanSceneGraph/lib/cmake/vsg/;VulkanSceneGraph/lib/cmake/vsg_glslang" ../
make
```

## Models

Models created in Blender
* Controller 'top' is at 0,0 (or just below)
* Controller 'up' is [0,0,-1] in blender space

Export from blender to gltf:
* Include custom properties
* Include punctual lights
* +Y up

Convert to vsg via `vsgconv model.glb model.vsgt`
* Ensure vsgXchange is built with assimp support (For assimp itself I used vcpkg)
* Ensure a recent build is used for correct lighting (fd35cc2 or newer)#

## Development Tips


Validation layers from the OpenXR SDK
```
set XR_API_LAYER_PATH="C:/dev/OpenXR-SDK-Source/build/src/api_layers/"
set XR_ENABLE_API_LAYERS=XR_APILAYER_LUNARG_core_validation
```
