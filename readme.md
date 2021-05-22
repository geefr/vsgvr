# VSG OpenVR prototype

## Status


Thing | Status
------------|--------
Linux Build | Building
Windows Build | Untested, should build
Code  | Messy
OpenVR Input | Present, through 'vrhelp' namespace. Needs rework eventually.
Controller tracking | Working
Controller models in scene | Working
HMD tracking | Partially Working
HMD Presentation | Partially Working - Limited render test to null steamvr driver :)
Correct Matrices for HMD | No - Desktop view upside down

Bugs:
* Segfault on exit

## Setup

If you don't have a headset see here - Force a null driver to allow basic display:
https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings

vr folder contains some basic openvr wrappers, pulled from one of my other projects. It'll need rework to say the least, should probably be rewritten completely.

Building requires:
* vulkan sdk/vulkan scene graph
* OpenVR sdk
* libglm-dev

```
cmake -DCMAKE_PREFIX_PATH="VulkanSceneGraph/lib/cmake/vsg/;VulkanSceneGraph/lib/cmake/vsg_glslang"  -DOPENVR_ROOT="/path/to/OpenVR" /path/to/vsg_vr_test
make
```
