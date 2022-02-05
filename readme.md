# VSG OpenVR prototype

Latest State: vsgvr library created, functionality encapsulated in a few functions & VRViewer class.
Hardcoded to OpenVR for now, but structure should allow for OpenXR/other backends.

example_vr.cpp should be similar enough to a desktop 'hello world' - As long as openvr is available before it's started it should 'just work'

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

Issue                        | Status
-----------------------------|-------
Segfault on exit             | Only on Linux?
Lighting isn't correct       | likely due to axes swap in view matrix - Shaders include hardcoded light source, which isn't transformed by the view matrix..
HMD Tracking is laggy        | Recent improvements, but not perfect. Tracking slightly delayed, should switch to explicit frame timings or otherwise fight waitGetPoses..

## Setup

If you don't have a headset see here - Force a null driver to allow basic display:
https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings

vr folder contains some basic openvr wrappers, pulled from one of my other projects. It'll need rework to say the least, should probably be rewritten completely.

Building requires:
* cmake > 3.14
* vulkan sdk/vulkan scene graph
* OpenVR sdk (Included as a submodule in deps)

```
cmake -DCMAKE_PREFIX_PATH="VulkanSceneGraph/lib/cmake/vsg/;VulkanSceneGraph/lib/cmake/vsg_glslang"  -DOPENVR_ROOT="/path/to/OpenVR" /path/to/vsg_vr_test
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
