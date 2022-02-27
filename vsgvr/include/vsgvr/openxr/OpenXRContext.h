#pragma once

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

#include <vsgvr/VRContext.h>

namespace vsgvr
{
  class OpenXRContextImpl;
  class VSG_DECLSPEC OpenXRContext : public vsg::Inherit<vsgvr::VRContext, OpenXRContext>
  {
  public:
    OpenXRContext(vsgvr::VRContext::TrackingOrigin origin = vsgvr::VRContext::TrackingOrigin::Standing);
    virtual ~OpenXRContext();

    void init(vsg::ref_ptr<vsg::Window> renderWindow) override final;

    void update() override final;
    void waitGetPoses() override final;

    std::vector<vsg::dmat4> getProjectionMatrices(float nearZ, float farZ) override final;
    std::vector<vsg::dmat4> getEyeToHeadTransforms() override final;

    uint32_t numberOfHmdImages() const override final;
    bool getRecommendedTargetSize(uint32_t &width, uint32_t &height) override final;

    void submitFrames(const std::vector<VkImage> images, VkDevice device,
                       VkPhysicalDevice physDevice, VkInstance instance,
                       VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                       uint32_t height, VkFormat format, int msaaSamples) override final;

    std::list<std::string> instanceExtensionsRequired(uint32_t vkVersion) const override final;
    std::list<std::string> deviceExtensionsRequired(vsg::ref_ptr<vsg::Instance> instance, vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice) const override final;
  private:
    OpenXRContextImpl* m = nullptr;
  };
}
