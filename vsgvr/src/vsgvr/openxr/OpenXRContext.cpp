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

#include <vsgvr/openxr/OpenXRContext.h>

// Higher level wrappers around openxr api
#include "OpenXRContextFunctions.cpp"

namespace vsgvr
{
  class OpenXRContextImpl
  {
    public:
      OpenXRContextImpl()
      {

      }
  };

  OpenXRContext::OpenXRContext(vsgvr::VRContext::TrackingOrigin origin)
    : m(new OpenXRContextImpl())
  {

  }

  OpenXRContext::~OpenXRContext()
  {

  }

  void OpenXRContext::update()
  {

  }

  void OpenXRContext::waitGetPoses()
  {
    // TODO: Doesn't apply to XR?
  }

  std::vector<vsg::dmat4> OpenXRContext::getProjectionMatrices(float nearZ, float farZ)
  {
    // TODO
  }

  std::vector<vsg::dmat4> OpenXRContext::getEyeToHeadTransforms()
  {
    // TODO
  }

  uint32_t OpenXRContext::numberOfHmdImages() const
  {
    // TODO: Doesn't apply to XR?
  }

  bool OpenXRContext::getRecommendedTargetSize(uint32_t &width, uint32_t &height)
  {
    // TODO: Doesn't apply to XR?
  }

  void OpenXRContext::submitFrames(const std::vector<VkImage> images, VkDevice device,
                      VkPhysicalDevice physDevice, VkInstance instance,
                      VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                      uint32_t height, VkFormat format, int msaaSamples)
  {
    // TODO: Doesn't apply to XR?
  }

  std::list<std::string> OpenXRContext::instanceExtensionsRequired() const
  {
    // TODO: Doesn't apply to XR?
  }

  std::list<std::string> OpenXRContext::deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const
  {
    // TODO: Doesn't apply to XR?
  }
}
