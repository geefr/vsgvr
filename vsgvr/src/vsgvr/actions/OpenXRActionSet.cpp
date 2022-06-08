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

#include <vsgvr/actions/OpenXRActionSet.h>
#include <vsgvr/OpenXRInstance.h>

#include <vsg/core/Exception.h>
#include "../OpenXRMacros.cpp"

namespace vsgvr
{
    OpenXRActionSet::OpenXRActionSet(OpenXRInstance *instance, std::string name, uint32_t priority, std::string localisedName)
        : _name(name), _priority(priority), _localisedName(localisedName)
    {
        if (_localisedName.empty())
            _localisedName = name;
        createActionSet(instance);
    }

    OpenXRActionSet::~OpenXRActionSet()
    {
        destroyActionSet();
    }

    void OpenXRActionSet::createActionSet(OpenXRInstance *instance)
    {
        auto info = XrActionSetCreateInfo();
        info.type = XR_TYPE_ACTION_SET_CREATE_INFO;
        info.next = nullptr;
        strncpy(info.actionSetName, _name.c_str(), std::min(static_cast<uint32_t>(_name.size()), static_cast<uint32_t>(XR_MAX_ACTION_NAME_SIZE)));
        strncpy(info.localizedActionSetName, _localisedName.c_str(), std::min(static_cast<uint32_t>(_localisedName.size()), static_cast<uint32_t>(XR_MAX_ACTION_NAME_SIZE)));
        info.priority = _priority;
        xr_check(xrCreateActionSet(instance->getInstance(), &info, &_actionSet), "Failed to create action set: " + _name);
    }

    void OpenXRActionSet::destroyActionSet()
    {
        xr_check(xrDestroyActionSet(_actionSet));
    }
}
