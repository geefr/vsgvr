#pragma once

#include <vsg/core/Inherit.h>

#include <vsgvr/openxr/OpenXR.h>
#include <vsgvr/openxr/OpenXRTraits.h>

namespace vsgvr {
    class OpenXRInstance;
    class OpenXRSession;
    class VSG_DECLSPEC OpenXREventHandler : public vsg::Inherit<vsg::Object, OpenXREventHandler>
    {
        public:
            OpenXREventHandler();
            ~OpenXREventHandler();

            void pollEvents(OpenXRInstance* instance, OpenXRSession* session);
    };
}
