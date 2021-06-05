# VSG OpenVR prototype

Thing                        | Status
-----------------------------|--------
Linux Build                  | Building
Windows Build                | Building
Code                         | Messy
OpenVR Input                 | Present, through 'vrhelp' namespace. Needs rework eventually.
Controller tracking          | Working
Controller models in scene   | Working
HMD tracking                 | Working, but errors in projection/view matrices
HMD Presentation             | Working, tested with null steamvr driver & HTC Vive on Linux
Correct Matrices for HMD     | View/Projection matrices may be correct..

Issue                        | Status
-----------------------------|-------
Segfault on exit             | Only on my laptop (quadro M2000), using null steamvr headset?
Model loading on windows     | vsgconv to C++ source fails to compile, due to > 16KB string literals
Lighting isn't correct       | likely projection matrix wrong?
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
