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

#include <vsg/app/ProjectionMatrix.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/quat.h>

#include <vsgvr/xr/OpenXRCommon.h>
#include <cmath>

namespace vsgvr
{
    class VSGVR_DECLSPEC OpenXRProjectionMatrix : public vsg::Inherit<vsg::ProjectionMatrix, OpenXRProjectionMatrix>
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

EVSG_type_name(vsgvr::OpenXRProjectionMatrix);
