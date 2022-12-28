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
#include <vsg/maths/transform.h>

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
    explicit UserOrigin(vsg::dmat4 in_matrix);
    explicit UserOrigin(vsg::dvec3 in_position, vsg::dquat in_orientation, vsg::dvec3 in_scale = {1.0, 1.0, 1.0});

    vsg::dmat4 userToScene() const { return matrix; }
    vsg::dmat4 sceneToUser() const { return vsg::inverse(matrix); }

    vsg::dmat4 transform(const vsg::dmat4& mv) const override { return mv * sceneToUser(); }

    /// Set the matrix to position the origin within the vsg scene space.
    /// 
    /// Note: The user is not usually at the origin, @see setUserInScene.
    /// 
    /// @param position The position to set the origin to, within the scene's space
    /// @param orientation The orientation of the origin, within the scene's space
    /// @param scale The scale of the origin, within the scene's space
    void setOriginInScene( vsg::dvec3 position, vsg::dquat orientation, vsg::dvec3 scale);

    /// Set the matrix to position th euser within the vsg scene space.
    ///
    /// This is similar to setOriginInScene, however accounts for the position of the player
    /// within the vr space - The origin will be offset by playerFloorPosition.
    /// 
    /// playerFloorPosition is typically calculated based on the user's head
    /// ```c++
    /// // Enable tracking of the headset - View space in OpenXR
    /// headPose = vsgvr::SpaceBinding::create(_xrInstance, XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_VIEW);
    /// viewer->spaceBindings.push_back(_headPose);
    /// 
    /// // The position on the ground, beneath the player
    /// // This assumes the user is standing on a flat plane, and that z == 0 in vr space
    /// // aligns with z == 0 in scene space (An intersection in scene space may be preferred)
    /// auto playerGroundPosition = _headPose->getTransform() * vsg::dvec3{ 0.0, 0.0, 0.0 };
    /// playerPosUser.z = 0.0;
    /// ```
    /// 
    /// @param playerGroundPosition A position on the floor, directly beneath the player, in the vr space
    /// @param position The position to set the origin to, within the scene's space
    /// @param orientation The orientation of the origin, within the scene's space
    /// @param scale The scale of the origin, within the scene's space
    void setUserInScene( vsg::dvec3 playerGroundPosition, vsg::dvec3 position, vsg::dquat orientation, vsg::dvec3 scale);

    vsg::dmat4 matrix;
  };
}

EVSG_type_name(vsgvr::UserOrigin);
