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

#include <vsg/core/Inherit.h>

#include <vsgvr/xr/OpenXRCommon.h>
#include <vsgvr/actions/OpenXRAction.h>

namespace vsgvr {
    class OpenXRSession;

    /**
     * An action of type XR_INPUT_ACTION_TYPE_POSE, and an associated action space
     * The space will only be valid while a session is running
     */
    class VSG_DECLSPEC OpenXRActionPoseBinding : public vsg::Inherit<OpenXRAction, OpenXRActionPoseBinding>
    {
        public:
            OpenXRActionPoseBinding(OpenXRActionSet* actionSet, std::string name, std::string localisedName);
            virtual ~OpenXRActionPoseBinding();

            XrSpace getActionSpace() const { return _space; }

            bool getTransformValid() const { return _transformValid; }
            vsg::mat4 getTransform() const { return _transform; }

            void createActionSpace(OpenXRSession* session);
            void destroyActionSpace();

            void setSpaceLocation(XrSpaceLocation location);
        private:
            XrSpace _space = nullptr;

            bool _transformValid = false;
            vsg::mat4 _transform;
    };
}
