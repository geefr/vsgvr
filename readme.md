# VSG OpenXR Integration

VR/XR integration layer for [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph) via OpenXR.

## Status

Generally things are a work in progress, but functionality is suitable for simple scenes:
* vsgvr works with the main VR platforms, and the codebase is somewhat portable
* Basic rendering and presentation is functional, presented to application through an OpenXRViewer class, similar to the desktop vsg::Viewer.
* Pose bindings are available, to allow models to be bound to controller positions or similar.
* The basic `example_vr.cpp` application is similar in usage to the desktop vsg Viewer application, and some test models are provided.

[![Demo Video](http://img.youtube.com/vi/ZA7syEMAIMo/0.jpg)](http://www.youtube.com/watch?v=ZA7syEMAIMo "vsgvr Demo Video")

### Supported Hardware

In theory, any OpenXR compatible hardware or systems are supported. Some slightly different behaviour has been noted between OpenXR runtimes however.

The following configurations have been tested:
* Rendering to a virtual HMD in Monado, on Linux
* Rendering to SteamVR (HTC Vive), on Linux, and Windows
* Rendering to SteamVR + Oculus Link (Quest 2) on Windows
* Rendering to Oculus OpenXR runtime + Oculus Link (Quest 2) on Windows
* Rendering to Oculus Quest 2, native build for Android

The following should work:
* All other desktop OpenXR runtimes, which use the standard OpenXR loader

The following might work / support is planned:
* Monado running on an Android phone / tablet


Feature                      | Status
-----------------------------|--------
Linux Build                  | Done
Windows Build                | Done
Android Build (Oculus)       | Done
Android Build (Generic)      | Done, untested
Code quality / API           | Messy, but in roughly the right structure for now
OpenXR Rendering             | Working, could do with a cleanup and better vsg integration at some point
OpenXR Input                 | Working
Controller tracking          | Working
Controller models in scene   | Working
HMD tracking                 | Working
Desktop view                 | Working

### OpenXR Spec and Extensions

As a minimum OpenXR requires the following OpenXR extensions:
* XR\_KHR\_vulkan\_enable
* On Android: XR\_KHR\_android\_create\_instance

In addition the Vulkan runtime must satisfy the requirements of OpenXR. These requirements will vary but generally relate to shared memory extensions and similar.

As features are implemented vsgvr will use more extensions if available, to support common features such as additional composition layers. If specific extensions are not present, or not requested by the application these features will be inoperable.

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

TODO: Currently vsgvr can be built for Android against the generic OpenXR loader - In theory this means it will work against Monado running on a phone/tablet.
This is however unverified, if you know how to install Monado on a phone, or otherwise have a functional OpenXR runtime on your phone please get in touch.

## Compilation

Required:
* cmake > 3.14
* vulkan sdk
* VulkanSceneGraph
* The OpenXR loader - From a variety of sources
  * OPENXR\_GENERIC - The generic OpenXR loader, included as a git submodule at deps/openxr
  * OPENXR\_OCULUS\_MOBILE - The Oculus mobile SDK, available from https://developer.oculus.com/downloads/package/oculus-openxr-mobile-sdk/

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
* Should face 'forward' as normal

Export from blender to gltf:
* Include custom properties
* Include punctual lights
* +Y up

Convert to vsg via `vsgconv model.glb model.vsgt`
* Ensure vsgXchange is built with assimp support

## Development Tips

Validation layers from the OpenXR SDK
```
set XR_API_LAYER_PATH="C:/dev/OpenXR-SDK-Source/build/src/api_layers/"
set XR_ENABLE_API_LAYERS=XR_APILAYER_LUNARG_core_validation
```

