# VSG OpenVR Integration

(prototype) VR support for [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph)

Latest State: vsgvr library created, functionality encapsulated in a few functions & VRViewer class.
Hardcoded to OpenVR for now, but structure should allow for OpenXR/other backends.

[![Demo Video](http://img.youtube.com/vi/ZA7syEMAIMo/0.jpg)](http://www.youtube.com/watch?v=ZA7syEMAIMo "vsgvr Demo Video")

example_vr.cpp should be similar enough to a desktop 'hello world' - As long as openvr is available before it's started it should 'just work'.

Rough TODO:
* Lots of cleanups - code style isn't great
* May need an HMDWindow class, to split out some functionality currently hardcoded into VRViewer
* Binding new devices into the scene graph at runtime - At the moment all devices must be on and tracking when the example starts
* Map vr input handling to vsg event system

Thing                        | Status
-----------------------------|--------
Linux Build                  | Building
Windows Build                | Building
Code                         | Messy, but in roughly the right structure
OpenVR Input                 | Present, but input not bound to vsg events (see VRController::buttonPressed)
Controller tracking          | Working
Controller models in scene   | Working
HMD tracking                 | Working
HMD Presentation             | Working
Desktop view                 | Broken - blank display?

Issue                        | Status
-----------------------------|-------
Lighting isn't correct       | Fixed, if the latest vsgXchange/lights_experimental work is used
Desktop mirror window looks a little different | Will need rework, the desktop view is very basic at the moment


## Setup

If you don't have a headset see here - Force a null driver to allow basic display output:
https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings

Building requires:
* cmake > 3.14
* vulkan sdk
* VulkanSceneGraph
* OpenVR sdk (Included as a git submodule)
* (For model creation) vsgXchange -> As of 2022/02/05 the lights_experimental branch is required

```
git submodule update --init
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="VulkanSceneGraph/lib/cmake/vsg/;VulkanSceneGraph/lib/cmake/vsg_glslang" ../
make
```

# Models

Models created in Blender
* Controller 'top' is at 0,0 (or just below)
* Controller 'up' is [0,0,-1] in blender space

Export from blender to gltf:
* Include custom properties
* Include punctual lights
* +Y up

Convert to vsg via `vsgconv model.glb model.vsgt`
* Ensure vsgXchange is built with assimp support
* Ensure a recent build is used for correct lighting (fd35cc2 or newer)#
