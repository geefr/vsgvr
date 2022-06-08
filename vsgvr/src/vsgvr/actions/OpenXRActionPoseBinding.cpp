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
#include <vsgvr/OpenXRSession.h>

#include <vsg/core/Exception.h>
#include "../OpenXRMacros.cpp"

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

    bool OpenXRActionPoseBinding::suggestInteractionBinding(OpenXRInstance* instance, std::string interactionProfile, std::string posePath)
    {
        // Suggest bindings for the pose
        XrPath xrProfilePath;
        xr_check(xrStringToPath(instance->getInstance(), interactionProfile.c_str(), &xrProfilePath));
        XrPath xrPosePath;
        xr_check(xrStringToPath(instance->getInstance(), posePath.c_str(), &xrPosePath));

        auto actionBindings = XrActionSuggestedBinding();
        actionBindings.action = _action;
        actionBindings.binding = xrPosePath;

        auto interactionProfileBinding = XrInteractionProfileSuggestedBinding();
        interactionProfileBinding.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        interactionProfileBinding.next = nullptr;
        interactionProfileBinding.interactionProfile = xrProfilePath;
        interactionProfileBinding.countSuggestedBindings = 1;
        interactionProfileBinding.suggestedBindings = &actionBindings;
        auto result = xrSuggestInteractionProfileBindings(instance->getInstance(), &interactionProfileBinding);
        if( XR_SUCCEEDED(result) )
        {
            _anySuggestionSucceeded = true;
            return true;
        }
        else
        {
            std::cerr << "OpenXRActionPoseBinding::suggestInteractionBinding: Failed to suggest interaction binding (" << interactionProfile << " : " << posePath << "): " << to_string(result) << std::endl;
            return false;
        }
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
        xr_check(xrDestroySpace(_space));
        _space = nullptr;
    }
}
