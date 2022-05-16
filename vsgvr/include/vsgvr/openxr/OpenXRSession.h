#pragma once

#include <vsg/core/Inherit.h>
#include <vsgvr/openxr/OpenXR.h>

namespace vsgvr {
    class VSG_DECLSPEC OpenXRSession : public vsg::Inherit<vsg::Object, OpenXRSession>
    {

      // Similar to the other objects, but for the session.

      // This one may be a little difficult, as the session has a lifecycle - Depending on event updates the session may be discarded, recreated, resized, etc..

    };
}
