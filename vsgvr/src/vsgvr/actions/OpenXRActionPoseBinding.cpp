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

#include <vsgvr/actions/OpenXRActionPoseBinding.h>
#include <vsgvr/xr/OpenXRSession.h>

#include <vsg/core/Exception.h>
#include "../xr/OpenXRMacros.cpp"

#include <vsg/maths/mat4.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/transform.h>

#include <iostream>

namespace vsgvr
{
    OpenXRActionPoseBinding::OpenXRActionPoseBinding(OpenXRActionSet* actionSet, std::string name, std::string localisedName )
      : Inherit(actionSet, XR_ACTION_TYPE_POSE_INPUT, name, localisedName )
    {}

    OpenXRActionPoseBinding::~OpenXRActionPoseBinding()
    {
        destroyActionSpace();
    }

    void OpenXRActionPoseBinding::createActionSpace(OpenXRSession* session)
    {
        if( _space ) throw vsg::Exception({"Space already created: " + _name});

        // Create the action space
        { 
            auto info = XrActionSpaceCreateInfo();
            info.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
            info.next = nullptr;
            info.action = _action;
            info.subactionPath = XR_NULL_PATH;
            info.poseInActionSpace.orientation = XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f};
            info.poseInActionSpace.position = XrVector3f{0.0f, 0.0f, 0.0f};
            xr_check(xrCreateActionSpace(session->getSession(), &info, &_space), "Failed to create action space: " + _name);
        }
    }

    void OpenXRActionPoseBinding::destroyActionSpace()
    {
        if (_space) {
            xr_check(xrDestroySpace(_space));
            _space = 0;
        }
    }

    void OpenXRActionPoseBinding::setSpaceLocation(XrSpaceLocation location)
    {
      if ((location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) && 
          (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT))
      {
        _transformValid = true;

        auto q = vsg::dquat(
          location.pose.orientation.x,
          location.pose.orientation.y,
          location.pose.orientation.z,
          location.pose.orientation.w
        );
        auto rotateMat = vsg::rotate(q);

        auto p = vsg::dvec3(location.pose.position.x, location.pose.position.y, location.pose.position.z);
        auto translateMat = vsg::translate(p);
        _transform = translateMat * rotateMat;

        // std::cout << _name << ": " << p.x << "," << p.y << "," << p.z << std::endl;
      }
      else
      {
        _transformValid = false;
      }
    }
}
