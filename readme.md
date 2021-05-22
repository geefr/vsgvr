# VSG OpenVR prototype

Thing                        | Status
-----------------------------|--------
Linux Build                  | Building
Windows Build                | Untested, should build
Code                         | Messy
OpenVR Input                 | Present, through 'vrhelp' namespace. Needs rework eventually.
Controller tracking          | Working
Controller models in scene   | Working
HMD tracking                 | Working, but errors in projection/view matrices
HMD Presentation             | Working, tested with null steamvr driver & HTC Vive on Linux
Correct Matrices for HMD     | Definitely not

Bugs:
* Segfault on exit
* Lighting isn't correct - likely projection matrix wrong?
* Models all need to be y-up z-forward to work, axis mapping for vr matrices probably wrong
* Models are missing materials - Could use some help on blender -> vsg export..
* HMD Tracking is laggy - At least 1 frame delayed, need to tweak the current setup or switch to explicit frame timings as recommended

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
