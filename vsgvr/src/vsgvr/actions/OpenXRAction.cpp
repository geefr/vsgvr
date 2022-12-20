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

    OpenXRAction::OpenXRAction(vsg::ref_ptr<OpenXRInstance> instance, OpenXRActionSet *actionSet, XrActionType actionType, std::string name, std::string localisedName, std::vector<std::string> subPaths)
        : _actionType(actionType)
        , _name(name)
        , _localisedName(localisedName)
        , _subPaths(subPaths)
    {
        if (_localisedName.empty())
            _localisedName = name;
        createAction(instance, actionSet);
    }

    OpenXRAction::~OpenXRAction()
    {
        destroyAction();
    }

    void OpenXRAction::createAction(vsg::ref_ptr<OpenXRInstance> instance, OpenXRActionSet *actionSet)
    {
        auto info = XrActionCreateInfo();
        info.type = XR_TYPE_ACTION_CREATE_INFO;
        info.next = nullptr;
        strncpy(info.actionName, _name.c_str(), std::min(static_cast<uint32_t>(_name.size()), static_cast<uint32_t>(XR_MAX_ACTION_NAME_SIZE)));
        info.actionType = _actionType;

        auto subPaths = std::vector<XrPath>();
        for( auto& p : _subPaths ) {
            XrPath xrPath;
            xr_check(xrStringToPath(instance->getInstance(), p.c_str(), &xrPath));
            subPaths.push_back(xrPath);
        }

        info.countSubactionPaths = subPaths.size();
        info.subactionPaths = subPaths.data();
        strncpy(info.localizedActionName, _localisedName.c_str(), std::min(static_cast<uint32_t>(_localisedName.size()), static_cast<uint32_t>(XR_MAX_LOCALIZED_ACTION_NAME_SIZE)));
        xr_check(xrCreateAction(actionSet->getActionSet(), &info, &_action), "Failed to create action: " + _name);

        if( _subPaths.empty() ) {
            _state[{}] = {};
        } else {
            for(auto& p : _subPaths) {
                _state[p] = {};
            }
        }
    }

    void OpenXRAction::syncInputState(vsg::ref_ptr<OpenXRInstance> instance, vsg::ref_ptr<OpenXRSession> session, std::string subPath)
    {
        XrActionStateGetInfo info;
        info.type = XR_TYPE_ACTION_STATE_GET_INFO;
        info.next = nullptr;
        info.action = _action;

        if( subPath.empty() ) {
            // No sub-paths defined on the action or query all subactions
            // TODO: Spec confusing - "XR_NULL_PATH to specify all subaction paths"
            // https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_structs_to_describe_action_and_subaction_paths
            info.subactionPath = XR_NULL_PATH;
        } else {
            xr_check(xrStringToPath(instance->getInstance(), subPath.c_str(), &info.subactionPath));
        }

        switch(_actionType)
        {
            case XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT:
            {
                _state[subPath]._stateBool.type = XR_TYPE_ACTION_STATE_BOOLEAN;
                auto getActionResult =xrGetActionStateBoolean(session->getSession(), &info, &(_state[subPath]._stateBool));
                if( XR_SUCCEEDED(getActionResult) )
                {
                    _state[subPath]._stateValid = true;
                }
                else
                {
                    vsg::error("Failed to read input state for action ", _localisedName, ": ", to_string(getActionResult));
                    _state[subPath]._stateValid = false;
                }
                break;
            }
            case XrActionType::XR_ACTION_TYPE_FLOAT_INPUT:
            {
                _state[subPath]._stateFloat.type = XR_TYPE_ACTION_STATE_FLOAT;
                auto getActionResult =xrGetActionStateFloat(session->getSession(), &info, &(_state[subPath]._stateFloat));
                if( XR_SUCCEEDED(getActionResult) )
                {
                    _state[subPath]._stateValid = true;
                }
                else
                {
                    vsg::error("Failed to read input state for action ", _localisedName, ": ", to_string(getActionResult));
                    _state[subPath]._stateValid = false;
                }
                break;
            }
            case XrActionType::XR_ACTION_TYPE_VECTOR2F_INPUT:
            {
                _state[subPath]._stateVec2f.type = XR_TYPE_ACTION_STATE_VECTOR2F;
                auto getActionResult =xrGetActionStateVector2f(session->getSession(), &info, &(_state[subPath]._stateVec2f));
                if( XR_SUCCEEDED(getActionResult) )
                {
                    _state[subPath]._stateValid = true;
                }
                else
                {
                    vsg::error("Failed to read input state for action ", _localisedName, ": ", to_string(getActionResult));
                    _state[subPath]._stateValid = false;
                }
                break;
            }
            default:
                // Polled in a different manner (TODO: Will be reworked and error handled better, this function probably deleted
                break;
        }
    }

    XrActionStateBoolean OpenXRAction::getStateBool( std::string subPath ) const {
        auto it = _state.find(subPath);
        if( it == _state.end() ) return {};
        return it->second._stateBool;
    }
    XrActionStateFloat OpenXRAction::getStateFloat( std::string subPath ) const {
        auto it = _state.find(subPath);
        if( it == _state.end() ) return {};
        return it->second._stateFloat;
    }
    XrActionStateVector2f OpenXRAction::getStateVec2f( std::string subPath ) const {
        auto it = _state.find(subPath);
        if( it == _state.end() ) return {};
        return it->second._stateVec2f;
    }
    bool OpenXRAction::getStateValid( std::string subPath ) const {
        auto it = _state.find(subPath);
        if( it == _state.end() ) return {};
        return it->second._stateValid;
    }

    void OpenXRAction::destroyAction()
    {
        if( _action != XR_NULL_HANDLE ) {
            xr_check(xrDestroyAction(_action));
        }
    }
}

