#pragma once

#include <vsg/viewer/ProjectionMatrix.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/quat.h>

#include <vsgvr/openxr/OpenXR.h>
#include <cmath>

namespace vsgvr
{
    class VSG_DECLSPEC OpenXRProjectionMatrix : public vsg::Inherit<vsg::ProjectionMatrix, OpenXRProjectionMatrix>
    {
    public:
        OpenXRProjectionMatrix(vsg::dmat4 matrix)
        : mat(matrix)
        {}

        OpenXRProjectionMatrix(const XrFovf& pose, double near, double far)
        {
            // https://gitlab.freedesktop.org/monado/demos/xrgears/-/blob/master/src/main.cpp _create_projection_from_fov
            // See also https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/main/src/common/xr_linear.h

            // TODO: Added to resolve inside-out objects, should be reviewed properly
            // TODO: Pretty sure this is related to vsg being reverse-depth by default. Should be fixed to work for both.
            std::swap(near, far);

            auto tanL = std::tan(pose.angleLeft);
            auto tanD = std::tan(pose.angleDown);
            auto tanU = std::tan(pose.angleUp);
            auto tanR = std::tan(pose.angleRight);

            auto tanX = tanR - tanL;
            auto tanY = tanU - tanD;

            mat = vsg::dmat4(
              2 / tanX, 0, 0, 0,
              0, 2 / tanY, 0, 0,
              (tanR + tanL) / tanX, (tanU + tanD) / tanY, -far / (far - near), -1,
              0, 0, -(far * near) / (far - near), 0
            );
        }

        vsg::dmat4 transform() const override { return mat; }
        vsg::dmat4 mat;
    };
}
