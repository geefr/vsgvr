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

#include <vsgvr/xr/Common.h>
#include <vsgvr/xr/Traits.h>
#include <vsgvr/actions/ActionSet.h>

namespace vsgvr {
  class VSGVR_DECLSPEC Instance : public vsg::Inherit<vsg::Object, Instance>
  {
  public:
    Instance() = delete;
    /// Create an OpenXR instance, and configure the overall XrSystem
    /// Once initialised applications should configure Instance::traits
    /// as needed, in order to continue to initialisation of a Viewer / Session
    /// 
    /// An exception will be thrown if the formFactor is unsupported. In this case applications
    /// may attempt to re-create the Instance with an alternate form factor.
    Instance(XrFormFactor formFactor, vsg::ref_ptr<vsgvr::Traits> xrTraits);
    ~Instance();

    std::vector<XrViewConfigurationType> getSupportedViewConfigurationTypes();
    std::vector<XrEnvironmentBlendMode> getSupportedEnvironmentBlendModes(XrViewConfigurationType viewConfiguration);

    /// Check whether instance-level features are supported by the OpenXR runtime
    bool checkViewConfigurationSupported(XrViewConfigurationType viewConfiguration);
    bool checkEnvironmentBlendModeSupported(XrViewConfigurationType viewConfiguration, XrEnvironmentBlendMode environmentBlendMode);

    vsg::ref_ptr<vsgvr::Traits> traits;

    XrInstance getInstance() const { return _instance; }
    XrInstanceProperties getInstanceProperties() const { return _instanceProperties; }
    XrSystemId getSystem() const { return _system; }
    XrSystemProperties getSystemProperties() const { return _systemProperties; }

    void onEventInstanceLossPending(const XrEventDataInstanceLossPending& event);

  private:
    void createInstance();
    void destroyInstance();
    void createSystem();

    XrFormFactor _formFactor;

    XrInstance _instance = XR_NULL_HANDLE;
    XrInstanceProperties _instanceProperties;

    XrSystemId _system = XR_NULL_SYSTEM_ID;
    XrSystemProperties _systemProperties;
  };
}

EVSG_type_name(vsgvr::Instance);
