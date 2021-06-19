#pragma once

/*
Copyright(c) 2021 Gareth Francis

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vsg/nodes/Group.h>

#include <memory>

// TODO: Generic interface for vr backends
#include "../../../vr/env.h"

namespace vsgvr {
    // TODO: Placeholder types - Will need to migrate vrhelp classes in, make generic to account for other backends
    using Context = vrhelp::Env;
    using Device = vrhelp::Device;
    using Controller = vrhelp::Controller;

    const std::string TAG_DEVICE_NAME = "vsgvr::device::name";
    const std::string TAG_DEVICE_ID = "vsgvr::device::id";

    /// Connect to the VR runtime and initialise a context
    std::shared_ptr<vsgvr::Context> initVR();

    /// Add a MatrixTransform to the scene for each tracked device
    void createDeviceNodes(std::shared_ptr<vsgvr::Context> ctx, vsg::ref_ptr<vsg::Group> parentNode, vsg::ref_ptr<vsg::Node> controllerModel = {});

} // namespace vsg
