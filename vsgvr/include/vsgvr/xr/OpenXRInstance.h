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
#include <vsg/core/ref_ptr.h>

#include <vsgvr/xr/OpenXRCommon.h>
#include <vsgvr/xr/OpenXRTraits.h>
#include <vsgvr/actions/OpenXRActionSet.h>

namespace vsgvr {
    class VSGVR_DECLSPEC OpenXRInstance : public vsg::Inherit<vsg::Object, OpenXRInstance>
    {
        public:
            OpenXRInstance() = delete;
            OpenXRInstance(vsg::ref_ptr<OpenXrTraits> xrTraits);
            ~OpenXRInstance();

            XrInstance getInstance() const { return _instance; }
            XrInstanceProperties getInstanceProperties() const { return _instanceProperties; }
            XrSystemId getSystem() const { return _system; }

            // TODO: Greater control over view configuration - At the moment traits
            // are specified, and if invalid init fails. Between system init and
            // session init there should be a point where view config can be chosen
            // such as choosing whether multisampling should be on, the size of
            // view images and such (based on _viewConfigurationViews)
            // TODO: Choices in traits should be a preference list?
            void onEventInstanceLossPending(const XrEventDataInstanceLossPending& event);

        private:
            void createInstance();
            void destroyInstance();
            void createSystem();
            // TODO: Support/validation of xr layers - Debug/validation, view type extensions
            void validateTraits();

            vsg::ref_ptr<OpenXrTraits> _xrTraits;

            XrInstance _instance = 0;
            XrInstanceProperties _instanceProperties;

            XrSystemId _system = 0;
            XrSystemProperties _systemProperties;
    };
}

EVSG_type_name(vsgvr::OpenXRInstance);
