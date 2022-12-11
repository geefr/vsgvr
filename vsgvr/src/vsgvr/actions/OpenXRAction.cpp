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

#include <vsgvr/actions/OpenXRAction.h>
#include <vsgvr/actions/OpenXRActionSet.h>
#include <vsgvr/xr/OpenXRSession.h>

#include <vsg/core/Exception.h>
#include "../xr/OpenXRMacros.cpp"

#include <vsg/io/Logger.h>

#include <iostream>

namespace vsgvr
{

    OpenXRAction::OpenXRAction(OpenXRActionSet *actionSet, XrActionType actionType, std::string name, std::string localisedName)
        : _actionType(actionType), _name(name), _localisedName(localisedName)
    {
        if (_localisedName.empty())
            _localisedName = name;
        createAction(actionSet);
    }

    OpenXRAction::~OpenXRAction()
    {
        destroyAction();
    }

    void OpenXRAction::createAction(OpenXRActionSet *actionSet)
    {
        auto info = XrActionCreateInfo();
        info.type = XR_TYPE_ACTION_CREATE_INFO;
        info.next = nullptr;
        strncpy(info.actionName, _name.c_str(), std::min(static_cast<uint32_t>(_name.size()), static_cast<uint32_t>(XR_MAX_ACTION_NAME_SIZE)));
        info.actionType = _actionType;
        // TODO: Support for subactions (Ability to assign an action to multiple paths e.g. both controllers)
        info.countSubactionPaths = 0;
        info.subactionPaths = nullptr;
        strncpy(info.localizedActionName, _localisedName.c_str(), std::min(static_cast<uint32_t>(_localisedName.size()), static_cast<uint32_t>(XR_MAX_LOCALIZED_ACTION_NAME_SIZE)));
        xr_check(xrCreateAction(actionSet->getActionSet(), &info, &_action), "Failed to create action: " + _name);
    }

    void OpenXRAction::syncInputState(vsg::ref_ptr<OpenXRSession> session)
    {
        XrActionStateGetInfo info;
        info.type = XR_TYPE_ACTION_STATE_GET_INFO;
        info.next = nullptr;
        info.action = _action;
        // TODO: Subaction support
        info.subactionPath = XR_NULL_PATH;

        switch(_actionType)
        {
            case XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT:
            {
                _stateBool.type = XR_TYPE_ACTION_STATE_BOOLEAN;
                auto getActionResult =xrGetActionStateBoolean(session->getSession(), &info, &_stateBool);
                if( XR_SUCCEEDED(getActionResult) )
                {
                    _stateValid = true;
                }
                else
                {
                    vsg::error("Failed to read input state for action ", _localisedName, ": ", to_string(getActionResult));
                    _stateValid = false;
                }
                break;
            }
            case XrActionType::XR_ACTION_TYPE_FLOAT_INPUT:
            {
                _stateBool.type = XR_TYPE_ACTION_STATE_FLOAT;
                auto getActionResult =xrGetActionStateFloat(session->getSession(), &info, &_stateFloat);
                if( XR_SUCCEEDED(getActionResult) )
                {
                    _stateValid = true;
                }
                else
                {
                    vsg::error("Failed to read input state for action ", _localisedName, ": ", to_string(getActionResult));
                    _stateValid = false;
                }
                break;
            }
            case XrActionType::XR_ACTION_TYPE_VECTOR2F_INPUT:
            {
                _stateBool.type = XR_TYPE_ACTION_STATE_VECTOR2F;
                auto getActionResult =xrGetActionStateVector2f(session->getSession(), &info, &_stateVec2f);
                if( XR_SUCCEEDED(getActionResult) )
                {
                    _stateValid = true;
                }
                else
                {
                    vsg::error("Failed to read input state for action ", _localisedName, ": ", to_string(getActionResult));
                    _stateValid = false;
                }
                break;
            }
            default:
                // Polled in a different manner (TODO: Will be reworked and error handled better, this function probably deleted
                break;
        }
    }

    void OpenXRAction::destroyAction()
    {
        xr_check(xrDestroyAction(_action));
    }
}

