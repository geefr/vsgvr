# VSG OpenVR prototype

Thing                        | Status
-----------------------------|--------
Linux Build                  | Building
Windows Build                | Building
Code                         | Messy
OpenVR Input                 | Present, through 'vrhelp' namespace. Needs rework eventually.
Controller tracking          | Working
Controller models in scene   | Working
HMD tracking                 | Working
HMD Presentation             | Working, tested with null steamvr driver & HTC Vive on Linux

Issue                        | Status
-----------------------------|-------
Segfault on exit             | Only on my laptop (quadro M2000), using null steamvr headset?
Lighting isn't correct       | likely due to axes swap in view matrix - Shaders include hardcoded light source, which isn't transformed by the view matrix..
HMD Tracking is laggy        | Recent improvements, but not perfect. Tracking slightly delayed, should switch to explicit frame timings or otherwise fight waitGetPoses..

## Setup

If you don't have a headset see here - Force a null driver to allow basic display:
https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings

vr folder contains some basic openvr wrappers, pulled from one of my other projects. It'll need rework to say the least, should probably be rewritten completely.

Building requires:
* cmake > 3.14
* vulkan sdk/vulkan scene graph
* OpenVR sdk
* glm (Fetched automatically)
* fmtlib (Fetched automatically)

```
cmake -DCMAKE_PREFIX_PATH="VulkanSceneGraph/lib/cmake/vsg/;VulkanSceneGraph/lib/cmake/vsg_glslang"  -DOPENVR_ROOT="/path/to/OpenVR" /path/to/vsg_vr_test
make
```
