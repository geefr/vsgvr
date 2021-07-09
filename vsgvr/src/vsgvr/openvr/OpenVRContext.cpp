#include <vsgvr/openvr/OpenVRContext.h>
#include <vsgvr/VRController.h>
#include <vsgvr/VRDevice.h>

#include <vsg/core/Exception.h>

#include <openvr.h>

#include <memory>
#include <sstream>

namespace vsgvr
{
    class OpenVRContextImpl
    {
    public:
        vr::IVRSystem *ctx = nullptr;
        vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];

        vsg::dmat4 getProjectionMatrix(vr::EVREye eye, float nearZ, float farZ)
        {
            auto projMat = ctx->GetProjectionMatrix(eye, nearZ, farZ);
            auto m = projMat.m;
            return {
                m[0][0],
                m[1][0],
                m[2][0],
                m[3][0],
                m[0][1],
                m[1][1],
                m[2][1],
                m[3][1],
                m[0][2],
                m[1][2],
                m[2][2],
                m[3][2],
                m[0][3],
                m[1][3],
                m[2][3],
                m[3][3],
            };
        }

        vsg::dmat4 getEyeToHeadTransform(vr::EVREye eye)
        {
            auto transform = ctx->GetEyeToHeadTransform(eye);
            auto m = transform.m;
            return {
                m[0][0],
                m[1][0],
                m[2][0],
                0.0f,
                m[0][1],
                m[1][1],
                m[2][1],
                0.0f,
                m[0][2],
                m[1][2],
                m[2][2],
                0.0f,
                m[0][3],
                m[1][3],
                m[2][3],
                1.0f,
            };
        }

        std::string to_string(vr::ETrackedDeviceClass devClass)
        {
            switch (devClass)
            {
            case vr::TrackedDeviceClass_Invalid:
                return "Invalid";
            case vr::TrackedDeviceClass_HMD:
                return "HMD";
            case vr::TrackedDeviceClass_Controller:
                return "Controller";
            case vr::TrackedDeviceClass_GenericTracker:
                return "Generic Tracker";
            case vr::TrackedDeviceClass_TrackingReference:
                return "Base Station";
            case vr::TrackedDeviceClass_DisplayRedirect:
                return "Display Redirect";
            case vr::TrackedDeviceClass_Max:
                return "Max";
            }
            return "Unknown";
        }

        std::string getDeviceString(vr::TrackedDeviceIndex_t dev, vr::TrackedDeviceProperty prop) {
            vr::TrackedPropertyError err;
            auto sLen = ctx->GetStringTrackedDeviceProperty(dev, prop, nullptr, 0, &err);
            if (sLen == 0)
            {
                return "";
            }
            std::unique_ptr<char[]> res(new char[sLen]);
            if (ctx->GetStringTrackedDeviceProperty(dev, prop, res.get(), sLen, &err) != sLen || !res)
            {
                throw vsg::Exception{"Failed to read device string"};
            }
            return std::string(res.get());
        }

        /// Factory method to create device given generic stuff
        vsg::ref_ptr<VRDevice> enumerate(uint32_t id)
        {
            if (!ctx->IsTrackedDeviceConnected(id)) return {};

            // Query the common stuff
            auto devClass = ctx->GetTrackedDeviceClass(id);
            auto name = to_string(devClass);
            auto serial = getDeviceString(id, vr::Prop_SerialNumber_String);
            
            // Setup the child class bits
            switch (devClass)
            {
            case vr::TrackedDeviceClass_Controller:
                return VRController::create(id, name, serial);
            case vr::TrackedDeviceClass_Invalid:
            case vr::TrackedDeviceClass_HMD:
            case vr::TrackedDeviceClass_GenericTracker:
            case vr::TrackedDeviceClass_TrackingReference:
            case vr::TrackedDeviceClass_DisplayRedirect:
            case vr::TrackedDeviceClass_Max:
            default:
                // TODO: Not all types have a class yet, assign any unknowns as generic trackers
                return VRDevice::create(VRDevice::DeviceType::Generic, id, name, serial);
            }
        }

        void setDevicePose(vsg::ref_ptr<vsgvr::VRDevice> dev, vr::TrackedDevicePose_t pose)
        {
            auto p = pose.mDeviceToAbsoluteTracking.m;
            dev->deviceToAbsoluteMatrix = {
                p[0][0], p[1][0], p[2][0], 0.f, p[0][1], p[1][1], p[2][1], 0.f,
                p[0][2], p[1][2], p[2][2], 0.f, p[0][3], p[1][3], p[2][3], 1.f
            };
        }

        std::list<std::string> instanceExtensionsRequired() const
        {
            if (!vr::VRCompositor())
                return {};

            std::list<std::string> extensions;
            uint32_t nBufferSize = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);
            if (nBufferSize > 0)
            {
                // Allocate memory for the space separated list and query for it
                char *pExtensionStr = new char[nBufferSize];
                pExtensionStr[0] = 0;
                vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(pExtensionStr, nBufferSize);

                // Break up the space separated list into entries on the CUtlStringList
                std::string curExtStr;
                uint32_t nIndex = 0;
                while (pExtensionStr[nIndex] != 0 && (nIndex < nBufferSize))
                {
                    if (pExtensionStr[nIndex] == ' ')
                    {
                        extensions.push_back(curExtStr);
                        curExtStr.clear();
                    }
                    else
                    {
                        curExtStr += pExtensionStr[nIndex];
                    }
                    nIndex++;
                }
                if (curExtStr.size() > 0)
                {
                    extensions.push_back(curExtStr);
                }

                delete[] pExtensionStr;
            }

            return extensions;
        }

        std::list<std::string> deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const
        {
            if (!vr::VRCompositor())
                return {};

            std::list<std::string> extensions;
            uint32_t nBufferSize = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalDevice, nullptr, 0);
            if (nBufferSize > 0)
            {
                // Allocate memory for the space separated list and query for it
                char *pExtensionStr = new char[nBufferSize];
                pExtensionStr[0] = 0;
                vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(
                    physicalDevice, pExtensionStr, nBufferSize);

                // Break up the space separated list into entries on the CUtlStringList
                std::string curExtStr;
                uint32_t nIndex = 0;
                while (pExtensionStr[nIndex] != 0 && (nIndex < nBufferSize))
                {
                    if (pExtensionStr[nIndex] == ' ')
                    {
                        extensions.push_back(curExtStr);
                        curExtStr.clear();
                    }
                    else
                    {
                        curExtStr += pExtensionStr[nIndex];
                    }
                    nIndex++;
                }
                if (curExtStr.size() > 0)
                {
                    extensions.push_back(curExtStr);
                }

                delete[] pExtensionStr;
            }

            return extensions;
        }
    };

    OpenVRContext::OpenVRContext(vsgvr::VRContext::TrackingOrigin origin)
        : m(new OpenVRContextImpl())
    {
        if (!vr::VR_IsHmdPresent())
        {
            throw vsg::Exception{"No HMD Detected"};
        }
        if (!vr::VR_IsRuntimeInstalled())
        {
            throw vsg::Exception{"OpenVR runtime not detected"};
        }

        auto err = vr::VRInitError_None;
        m->ctx = vr::VR_Init(&err, vr::EVRApplicationType::VRApplication_Scene);
        if (!m->ctx || err != vr::VRInitError_None)
        {
            std::stringstream str;
            str << "Failed to create OpenVR context: "
                << vr::VR_GetVRInitErrorAsEnglishDescription(err);
            throw vsg::Exception{str.str()};
        }

        for (auto id = vr::k_unTrackedDeviceIndex_Hmd;
             id < vr::k_unMaxTrackedDeviceCount; ++id)
        {
            auto dev = m->enumerate(id);
            if (!dev)
                continue;
            vrDevices[id] = dev;
        }
    }

    OpenVRContext::~OpenVRContext()
    {
        vr::VR_Shutdown();
        delete m;
    }

    void OpenVRContext::update()
    {
        if (!m->ctx)
        {
            return;
        }

        // Pump the event queue
        vr::VREvent_t ev;
        while (m->ctx->PollNextEvent(&ev, sizeof(ev)))
        {
            if (ev.eventType == vr::VREvent_TrackedDeviceActivated ||
                ev.eventType == vr::VREvent_TrackedDeviceUpdated)
            {
                // (re)enumerate the device
                auto dev = m->enumerate(ev.trackedDeviceIndex);
                if (!dev)
                    continue;
                vrDevices[ev.trackedDeviceIndex] = dev;
            }
            else if (ev.eventType == vr::VREvent_TrackedDeviceDeactivated)
            {
                vrDevices.erase(ev.trackedDeviceIndex);
            }
            else
            {
                auto dev = vrDevices.find(ev.trackedDeviceIndex);
                if (dev == vrDevices.end())
                {
                    continue;
                }

                auto controller = dynamic_cast<VRController *>(dev->second.get());
                if (!controller)
                {
                    continue;
                }

                switch (ev.eventType)
                {
                case vr::VREvent_ButtonPress:
                    controller->buttonPress(ev.data.controller.button);
                    break;
                case vr::VREvent_ButtonUnpress:
                    controller->buttonUnPress(ev.data.controller.button);
                    break;
                case vr::VREvent_ButtonTouch:
                    controller->buttonTouch(ev.data.controller.button);
                    break;
                case vr::VREvent_ButtonUntouch:
                    controller->buttonUntouch(ev.data.controller.button);
                    break;
                default:
                    break;
                }
            }
        }
    }

    std::vector<vsg::dmat4> OpenVRContext::getProjectionMatrices(float nearZ, float farZ)
    {
        return {m->getProjectionMatrix(vr::EVREye::Eye_Left, nearZ, farZ),
                m->getProjectionMatrix(vr::EVREye::Eye_Right, nearZ, farZ)};
    }

    std::vector<vsg::dmat4> OpenVRContext::getEyeToHeadTransforms()
    {
        return {m->getEyeToHeadTransform(vr::EVREye::Eye_Left),
                m->getEyeToHeadTransform(vr::EVREye::Eye_Right)};
    }

    void OpenVRContext::waitGetPoses()
    {
        vr::VRCompositor()->WaitGetPoses(m->trackedDevicePoses,
                                         vr::k_unMaxTrackedDeviceCount, nullptr, 0);

        // Update poses of tracked devices
        for (uint32_t id = 0; id < vr::k_unMaxTrackedDeviceCount; ++id)
        {
            auto dev = vrDevices.find(id);
            if (dev == vrDevices.end())
            {
                continue;
            }
            auto &pose = m->trackedDevicePoses[id];
            if (!pose.bDeviceIsConnected)
            {
                continue;
            }

            m->setDevicePose(dev->second, pose);
        }
    }

    uint32_t OpenVRContext::numberOfHmdImages() const
    {
        // OpenVR always has 2 images
        return 2;
    }

    bool OpenVRContext::getRecommendedTargetSize(uint32_t &width, uint32_t &height)
    {
        if (!m->ctx) return false;
        m->ctx->GetRecommendedRenderTargetSize(&width, &height);
        return true;
    }

    void OpenVRContext::submitFrames(const std::vector<VkImage> images, VkDevice device,
                                     VkPhysicalDevice physDevice, VkInstance instance,
                                     VkQueue queue, uint32_t queueFamIndex, uint32_t width,
                                     uint32_t height, VkFormat format, int msaaSamples)
    {
        auto &leftImg = images[0];
        auto &rightImg = images[1];

        // Submit to SteamVR
        vr::VRTextureBounds_t bounds;
        bounds.uMin = 0.0f;
        bounds.uMax = 1.0f;
        bounds.vMin = 0.0f;
        bounds.vMax = 1.0f;

        vr::VRVulkanTextureData_t vulkanData;
        vulkanData.m_nImage = (uint64_t)leftImg;
        vulkanData.m_pDevice = device;
        vulkanData.m_pPhysicalDevice = physDevice;
        vulkanData.m_pInstance = instance;
        vulkanData.m_pQueue = queue;
        vulkanData.m_nQueueFamilyIndex = queueFamIndex;

        vulkanData.m_nWidth = width;
        vulkanData.m_nHeight = height;
        vulkanData.m_nFormat = format;
        vulkanData.m_nSampleCount = msaaSamples;

        vr::Texture_t texture = {&vulkanData, vr::TextureType_Vulkan,
                                 vr::ColorSpace_Auto};
        vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds);

        vulkanData.m_nImage = (uint64_t)rightImg;
        vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds);
    }

    std::list<std::string> OpenVRContext::instanceExtensionsRequired() const { return m->instanceExtensionsRequired(); }
    std::list<std::string> OpenVRContext::deviceExtensionsRequired(VkPhysicalDevice physicalDevice) const { return deviceExtensionsRequired(physicalDevice); };
}
