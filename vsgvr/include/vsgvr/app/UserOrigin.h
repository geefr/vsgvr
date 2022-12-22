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

#pragma once

#include <vsg/nodes/Transform.h>

namespace vsgvr {

  /// A node which allows the vsgvr origin to be set within the scene
  /// via position, rotation, and scale.
  /// 
  /// This node functions in the same way as vsg::MatrixTransform, but is intended
  /// to transform the whole scene to position the user as needed, within the scene,
  /// as world space is defined by the OpenXR / vsgvr session (The user's play area).
  /// 
  /// Also provided are the following functions, to provide transforms between
  /// the XR world space and vsg world space.
  /// * userToScene
  /// * sceneToUser
  class VSGVR_DECLSPEC UserOrigin : public vsg::Inherit<vsg::Transform, UserOrigin>
  {
  public:
    UserOrigin();
    explicit UserOrigin(vsg::dvec3 in_position, vsg::dquat in_orientation, vsg::dvec3 in_scale = {1.0, 1.0, 1.0});

    vsg::dmat4 userToScene() const;
    vsg::dmat4 sceneToUser() const;

    vsg::dmat4 transform(const vsg::dmat4& mv) const override { return mv * sceneToUser(); }

    /// Origin position within the vsg scene space
    vsg::dvec3 position;
    /// Origin orientation within the vsg scene space
    vsg::dquat orientation;
    /// Origin scale within the vsg scene space
    vsg::dvec3 scale = {1.0, 1.0, 1.0};
  };
}

EVSG_type_name(vsgvr::UserOrigin);
