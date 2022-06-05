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
      // TODO: Validate this. Based on https://gitlab.freedesktop.org/monado/demos/xrgears/-/blob/master/src/main.cpp
      // TODO: Would it be better to pass this through as-is, and modify the base reference space?
      auto q = vsg::dquat(
        pose.orientation.w * -1.0,
        pose.orientation.x,
        pose.orientation.y * -1.0,
        pose.orientation.z
      );
      auto rotateMat = vsg::rotate(q);

      auto p = vsg::dvec3(pose.position.x, pose.position.y, pose.position.z);
      auto translateMat = vsg::translate(p);
      matrix = vsg::inverse(translateMat * rotateMat);
    }

    vsg::dmat4 transform() const override { return matrix; }
    vsg::dmat4 matrix;
  };
}
