#pragma once

/*
Copyright(c) 2021 Gareth Francis

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

#include <vsgvr/TrackedDevice.h>

namespace vsgvr
{
  class VRContext : public vsg::Inherit<vsg::Object, VRContext>
  {
  public:
    using TrackedDevices = std::vector<vsg::ref_ptr<vsgvr::TrackedDevice>>;

    enum class TrackingOrigin
    {
      Uncalibrated,
      Seated,
      Standing,
    };

    VRContext(TrackingOrigin origin = vsgvr::TrackingOrigin::Standing);
    virtual ~VRContext();

    virtual void update() = 0;
    virtual void waitGetPoses() = 0;

    virtual const TrackedDevices& devices() const = 0;

    virtual vsg::ref_ptr<TrackedDevice> leftHand() const = 0;
    virtual vsg::ref_ptr<TrackedDevice> rightHand() const = 0;
    virtual vsg::ref_ptr<TrackedDevice> hmd() const = 0;

    virtual uint32_t numberOfHmdImages() const = 0;
    virtual void getRecommendedTargetSize(uint32_t &width, uint32_t &height) =  0;

    virtual void submitFrames(std::vector<vsgvr::HMDImage> images) = 0;

  protected:
    vsgvr::TrackingOrigin originType = vsgvr::TrackingOrigin::Standing;
    TrackedDevices devices;
  };
}
