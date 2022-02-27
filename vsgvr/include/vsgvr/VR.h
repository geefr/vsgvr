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

#include <vsgvr/VRContext.h>

#include <vsg/nodes/Group.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageInfo.h>

#include <memory>

namespace vsgvr
{

  const std::string TAG_DEVICE_NAME = "vsgvr::device::name";
  const std::string TAG_DEVICE_ID = "vsgvr::device::id";

  struct HMDImage
  {
    vsg::ref_ptr<vsg::ImageInfo> colourImageInfo;
    vsg::ref_ptr<vsg::Image> colourImage;
    vsg::ref_ptr<vsg::ImageInfo> depthImageInfo;
    vsg::ref_ptr<vsg::Image> depthImage;
    uint32_t width;
    uint32_t height;
  };

  /// Add a MatrixTransform to the scene for each tracked device
  void createDeviceNodes(vsg::ref_ptr<vsgvr::VRContext> ctx,
                         vsg::ref_ptr<vsg::Group> parentNode,
                         vsg::ref_ptr<vsg::Node> controllerModelLeft = {},
                         vsg::ref_ptr<vsg::Node> controllerModelRight = {});

} // namespace vsgvr
