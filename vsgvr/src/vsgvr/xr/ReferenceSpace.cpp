/*
Copyright(c) 2022 Gareth Francis

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vsgvr/xr/ReferenceSpace.h>
#include <vsgvr/xr/Session.h>

#include <vsg/core/Exception.h>
#include "Macros.cpp"

namespace vsgvr {

  ReferenceSpace::ReferenceSpace(XrSession session, XrReferenceSpaceType referenceSpaceType)
  {
    createSpace(session, referenceSpaceType, XrPosef{
      XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},
      XrVector3f{0.0f, 0.0f, 0.0f}
    });
  }
  ReferenceSpace::ReferenceSpace(XrSession session, XrReferenceSpaceType referenceSpaceType, XrPosef poseInReferenceSpace)
  {
    createSpace(session, referenceSpaceType, poseInReferenceSpace);
  }
  ReferenceSpace::~ReferenceSpace()
  {
    destroySpace();
  }

  XrSpaceLocation ReferenceSpace::locate(XrSpace baseSpace, XrTime time)
  {
    XrSpaceLocation s;
    s.type = XR_TYPE_SPACE_LOCATION;
    s.next = nullptr;
    xr_check(xrLocateSpace(_space, baseSpace, time, &s));
    return s;
  }

  void ReferenceSpace::createSpace(XrSession session, XrReferenceSpaceType referenceSpaceType, XrPosef poseInReferenceSpace)
  {
    XrReferenceSpaceCreateInfo info;
    info.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    info.next = nullptr;
    info.referenceSpaceType = referenceSpaceType;
    info.poseInReferenceSpace = poseInReferenceSpace;
    xr_check(xrCreateReferenceSpace(session, &info, &_space));
  }

  void ReferenceSpace::destroySpace()
  {
    if (_space != XR_NULL_HANDLE)
    {
      xr_check(xrDestroySpace(_space));
      _space = XR_NULL_HANDLE;
    }
  }
}
