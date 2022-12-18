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
#include <vsgvr/xr/OpenXRInstance.h>

#include <vsg/core/Exception.h>
#include "../xr/OpenXRMacros.cpp"

#include <iostream>

namespace vsgvr
{
    OpenXRActionSet::OpenXRActionSet(OpenXRInstance *instance, std::string name, std::string localisedName, uint32_t priority)
        : _name(name), _localisedName(localisedName), _priority(priority)
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


    bool OpenXRActionSet::suggestInteractionBindings(OpenXRInstance* instance, std::string interactionProfile, std::list<SuggestedInteractionBinding> bindings)
    {
      XrPath xrProfilePath;
      auto res = xrStringToPath(instance->getInstance(), interactionProfile.c_str(), &xrProfilePath);
      if (!XR_SUCCEEDED(res))
      {
        std::cerr << "OpenXRActionSet::suggestInteractionBindings: xrStringToPath(" << interactionProfile << "): " << to_string(res) << std::endl;
        return false;
      }

      std::vector<XrActionSuggestedBinding> actionBindings;
      for( auto& b : bindings )
      {  
        XrPath xrPath;
        xr_check(xrStringToPath(instance->getInstance(), b.path.c_str(), &xrPath), "suggestInteractionBindings: Invalid path provided");
        actionBindings.push_back(XrActionSuggestedBinding{b.action->getAction(), xrPath});
      }

      auto interactionProfileBinding = XrInteractionProfileSuggestedBinding();
      interactionProfileBinding.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
      interactionProfileBinding.next = nullptr;
      interactionProfileBinding.interactionProfile = xrProfilePath;
      interactionProfileBinding.countSuggestedBindings = actionBindings.size();
      interactionProfileBinding.suggestedBindings = actionBindings.data();
      auto result = xrSuggestInteractionProfileBindings(instance->getInstance(), &interactionProfileBinding);
      if (XR_SUCCEEDED(result))
      {
        return true;
      }
      else
      {
        std::cerr << "OpenXRActionPoseBinding::suggestInteractionBinding: Failed to suggest interaction bindings (" << interactionProfile << "): " << to_string(result) << std::endl;
        return false;
      }
    }
}

