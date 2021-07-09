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

#include <vsgvr/VRDevice.h>
#include <vsgvr/VRController.h>
#include <vsg/core/ref_ptr.h>

#include <list>
#include <string>

namespace vsgvr
{
  class VSG_DECLSPEC VRContext : public vsg::Inherit<vsg::Object, VRContext>
  {
  public:
    using TrackedDevices = std::map<uint32_t, vsg::ref_ptr<vsgvr::VRDevice>>;

    enum class TrackingOrigin
    {
      Uncalibrated,
      Seated,
      Standing,
    };
    virtual ~VRContext();

    virtual void update() = 0;
    virtual void waitGetPoses() = 0;

    const TrackedDevices& devices() const;

    vsg::ref_ptr<VRController> leftHand() const;
    vsg::ref_ptr<VRController> rightHand() const;
    vsg::ref_ptr<VRDevice> hmd() const;

    virtual std::vector<vsg::dmat4> getProjectionMatrices(float nearZ, float farZ) = 0;
    virtual std::vector<vsg::dmat4> getEyeToHeadTransforms() = 0;

    virtual uint32_t numberOfHmdImages() const = 0;
    virtual bool getRecommendedTargetSize(uint32_t &width, uint32_t &height) =  0;

    virtual void submitFrames(const std::vector<VkImage> images, VkDevice device,
                       VkPhysicalDevice physDevice, VkInstance instance,
                       VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                       uint32_t height, VkFormat format, int msaaSamples) = 0;

    virtual std::list<std::string> instanceExtensionsRequired() const = 0;
    virtual std::list<std::string> deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const = 0;

  protected:
    VRContext();
    VRContext(TrackingOrigin origin);

    void addDevice(uint32_t id, vsg::ref_ptr<vsgvr::VRDevice> device);
    void removeDevice(uint32_t id);
    vsg::ref_ptr<vsgvr::VRDevice> getDevice(uint32_t id) const;

    TrackingOrigin originType = TrackingOrigin::Standing;
    TrackedDevices vrDevices;
    vsg::ref_ptr<VRController> vrLeftHand;
    vsg::ref_ptr<VRController> vrRightHand;
    vsg::ref_ptr<VRDevice> vrHmd;
  };
}
