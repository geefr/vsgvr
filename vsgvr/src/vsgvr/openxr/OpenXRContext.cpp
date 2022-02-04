#include <vsgvr/openxr/OpenXRContext.h>
#include <vsgvr/VRController.h>
#include <vsgvr/VRDevice.h>

#include <vsg/core/Exception.h>

#include <memory>
#include <sstream>

namespace vsgvr
{
    class OpenXRContextImpl
    {
    public:

    };

    OpenXRContext::OpenXRContext(vsgvr::VRContext::TrackingOrigin origin)
        : m(new OpenXRContextImpl())
    {
    }

    OpenXRContext::~OpenXRContext()
    {
    }

    void OpenXRContext::update()
    {
       
    }

    std::vector<vsg::dmat4> OpenXRContext::getProjectionMatrices(float nearZ, float farZ)
    {
        return {{},{}};
    }

    std::vector<vsg::dmat4> OpenXRContext::getEyeToHeadTransforms()
    {
        return {{},{}};
    }

    void OpenXRContext::waitGetPoses()
    {
      
    }

    uint32_t OpenXRContext::numberOfHmdImages() const
    {
        return 0;
    }

    bool OpenXRContext::getRecommendedTargetSize(uint32_t &width, uint32_t &height)
    {
        return false;
    }

    void OpenXRContext::submitFrames(const std::vector<VkImage> images, VkDevice device,
                                     VkPhysicalDevice physDevice, VkInstance instance,
                                     VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                                     uint32_t height, VkFormat format, int msaaSamples)
    {
        
    }

    std::list<std::string> OpenXRContext::instanceExtensionsRequired() const { return {}; }
    std::list<std::string> OpenXRContext::deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const { return {}; };
}
