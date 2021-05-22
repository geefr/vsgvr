#pragma once

#include "openvr.h"
#include "glm/glm.hpp"

#include <cstdint>
#include <memory>

namespace vrhelp {
	class Device
	{
	public:
    Device(vr::IVRSystem* context, uint32_t id, vr::ETrackedDeviceClass devClass);
		virtual ~Device();

    /// Factory method to create device given generic stuff
    static std::unique_ptr<Device> enumerate(vr::IVRSystem* context, uint32_t id);

    /// Device space -> Absolute (world space) matrix, glm layout, -Ve Z forward
    glm::dmat4x4 deviceToAbsoluteMatrix() const;
    /// Device space -> Absolute (world space) matrix, glm layout, +Ve Z up
    glm::dmat4x4 deviceToAbsoluteMatrixZup() const;

    /// Position of device in world space
    glm::vec3 positionAbsolute() const;

    /// Forward vector
    glm::vec3 directionForward() const;
    glm::vec3 directionBackward() const;
    glm::vec3 directionLeft() const;
    glm::vec3 directionRight() const;
    glm::vec3 directionUp() const;
    glm::vec3 directionDown() const;

		uint32_t mID = 0;
		vr::ETrackedDeviceClass mDevClass;
		std::string mName;
		std::string mSerial;
    vr::TrackedDevicePose_t mPose;

		static std::string getDeviceString(vr::IVRSystem* context, vr::TrackedDeviceIndex_t dev,
			vr::TrackedDeviceProperty prop);
	};
}
