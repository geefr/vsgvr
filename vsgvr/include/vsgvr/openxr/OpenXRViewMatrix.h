#pragma once

#include <vsg/viewer/ViewMatrix.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/quat.h>

#include <vsgvr/openxr/OpenXR.h>

namespace vsgvr {
  class VSG_DECLSPEC OpenXRViewMatrix : public vsg::Inherit<vsg::ViewMatrix, OpenXRViewMatrix>
  {
  public:
    OpenXRViewMatrix(const vsg::dmat4& m) :
      matrix(m)
    {}

    OpenXRViewMatrix(const XrPosef& pose)
    {
      // https://gitlab.freedesktop.org/monado/demos/xrgears/-/blob/master/src/main.cpp _create_view_from_pose
      // Convert pose to matrix, then invert into the view matrix
      // TODO: Axis mapping here was done by experimentation, and based on OpenVR setup. This needs review later.
      // See also https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/main/src/common/xr_linear.h
      auto q = vsg::dquat(
        pose.orientation.x,
        pose.orientation.y * -1.0,
        pose.orientation.z,
        pose.orientation.w * -1.0
      );
      auto rotateMat = vsg::rotate(q);

      auto p = vsg::dvec3(pose.position.x, pose.position.y * -1.0, pose.position.z);
      auto translateMat = vsg::translate(p);
      matrix = vsg::inverse(translateMat * rotateMat );

      vsg::dmat4 viewAxesMat(
        1, 0, 0, 0,
        0, -1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      );
      auto worldRotateMat = vsg::rotate(-vsg::PI / 2.0, 1.0, 0.0, 0.0);
      matrix = matrix * viewAxesMat * worldRotateMat;
    }

    vsg::dmat4 transform() const override { return matrix; }
    vsg::dmat4 matrix;
  };
}
