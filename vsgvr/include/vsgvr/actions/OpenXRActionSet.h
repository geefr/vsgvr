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

#include <vsgvr/xr/OpenXRCommon.h>
#include <vsgvr/actions/OpenXRAction.h>
#include <vsg/core/Inherit.h>

#include <set>

namespace vsgvr {
    class OpenXRInstance;
    class VSGVR_DECLSPEC OpenXRActionSet : public vsg::Inherit<vsg::Object, OpenXRActionSet>
    {
        public:
            OpenXRActionSet(OpenXRInstance* instance, std::string name, std::string localisedName, uint32_t priority = 0);
            ~OpenXRActionSet();

            XrActionSet getActionSet() const { return _actionSet; }
            std::string getName() const { return _name; }
            std::string getLocalisedName() const { return _localisedName; }
            uint32_t getPriority() const { return _priority; }

            // Storage for actions in the set.
            // Any actions within the set will be synchronised by the viewer
            // each frame.
            std::vector<vsg::ref_ptr<OpenXRAction>> actions;

            struct SuggestedInteractionBinding
            {
              OpenXRAction* action;
              std::string path;
            };
            static bool suggestInteractionBindings(OpenXRInstance* instance, std::string interactionProfile, std::list<SuggestedInteractionBinding> bindings);

        private:
            void createActionSet(OpenXRInstance* instance);
            void destroyActionSet();

            std::string _name;
            std::string _localisedName;
            uint32_t _priority;

            XrActionSet _actionSet;

            static std::set<std::string> suggestInteractionBindingsCalledFor;
    };
}

EVSG_type_name(vsgvr::OpenXRActionSet);
