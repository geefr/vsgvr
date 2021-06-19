
/*
Copyright(c) 2021 Gareth Francis

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vsgvr/VR.h>

#include <vsg/nodes/MatrixTransform.h>

namespace vsgvr
{
    std::shared_ptr<vsgvr::Context> initVR() 
    {
        return std::make_shared<vsgvr::Context>(vr::ETrackingUniverseOrigin::TrackingUniverseStanding);
    }

    void createDeviceNodes(std::shared_ptr<vsgvr::Context> ctx, vsg::ref_ptr<vsg::Group> parentNode, vsg::ref_ptr<vsg::Node> controllerModel)
    {
        auto& devices = ctx->devices();
        for( auto& device : devices )
        {
            vsg::ref_ptr<vsg::Group> deviceNode = vsg::MatrixTransform::create();
            deviceNode->setValue(vsgvr::TAG_DEVICE_NAME, device.second->mName);
            deviceNode->setValue(vsgvr::TAG_DEVICE_ID, device.second->mSerial);
            parentNode->addChild(deviceNode);

            if( auto controller = dynamic_cast<vsgvr::Controller*>(device.second.get()) )
            {
                if( controllerModel )
                {
                    deviceNode->addChild(controllerModel);
                }
            }
        }
    }
} // namespace vsg
