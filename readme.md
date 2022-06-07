# VSG OpenXR Integration

(prototype) VR support for [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph)

Latest State: vsgvr library created, functionality encapsulated in a few functions & VRViewer class.
Functionality under OpenXR has reached parity with previous openvr version, so OpenXR will be the backend going forward.
(openvr version should be available in a branch if you want it)

[![Demo Video](http://img.youtube.com/vi/ZA7syEMAIMo/0.jpg)](http://www.youtube.com/watch?v=ZA7syEMAIMo "vsgvr Demo Video")

example_vr.cpp should be similar enough to a desktop 'hello world'.

Thing                        | Status
-----------------------------|--------
Linux Build                  | Building
Windows Build                | Building
Code                         | Messy, but in roughly the right structure
OpenXR Presentation          | Present, will need cleanup but should be functional for now
OpenXR Input                 | Not Implemented Yet
Controller tracking          | Not Implemented Yet
Controller models in scene   | Not Implemented Yet
HMD tracking                 | Working
Desktop view                 | Not Implemented Yet

Issue                        | Status
-----------------------------|-------


## Setup

If you don't have a headset see here - Force a null driver to allow basic display output:
https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings

Building requires:
* cmake > 3.14
* vulkan sdk
* VulkanSceneGraph
* OpenXR loader (Included in deps/openxr)
* (For model creation) vsgXchange

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
* Ensure vsgXchange is built with assimp support (For assimp itself I used vcpkg)
* Ensure a recent build is used for correct lighting (fd35cc2 or newer)#
