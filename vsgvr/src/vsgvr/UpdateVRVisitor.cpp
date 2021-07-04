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

#include <vsgvr/UpdateVRVisitor.h>

// TODO: Remove use of glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>

#include <vsg/nodes/MatrixTransform.h>

namespace vsgvr {
UpdateVRVisitor::UpdateVRVisitor(vsg::ref_ptr<vsgvr::VRContext> context) : ctx(context) {}

void UpdateVRVisitor::apply(vsg::Group &o) {
  std::string val;
  if (o.getValue(vsgvr::TAG_DEVICE_ID, val)) {
    if (auto txf = o.cast<vsg::MatrixTransform>()) {
      auto &devices = ctx->devices();
      auto it = std::find_if(devices.begin(), devices.end(), [&val](auto &d) {
        return d.second->serial == val;
      });

      txf->matrix = it->second->deviceToAbsoluteMatrix;
    }
  }

  o.traverse(*this);
}
} // namespace vsgvr
