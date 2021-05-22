#pragma once

#include "device.h"

#include <map>

#include "openvr.h"

namespace vrhelp {
	class Controller : public Device
	{
	public:
    Controller(vr::IVRSystem* context, uint32_t id, vr::ETrackedDeviceClass devClass);
		virtual ~Controller();

    void buttonPress(uint32_t button);
    void buttonUnPress(uint32_t button);
    void buttonTouch(uint32_t button);
    void buttonUntouch(uint32_t button);

    int32_t mRole = 0;
    std::map<uint32_t, bool> mButtonPressed;
    std::map<uint32_t, bool> mButtonTouched;
	};

  inline void Controller::buttonPress(uint32_t button) { mButtonPressed[button] = true;}
  inline void Controller::buttonUnPress(uint32_t button) { mButtonPressed[button] = false; }
  inline void Controller::buttonTouch(uint32_t button) { mButtonTouched[button] = true; }
  inline void Controller::buttonUntouch(uint32_t button) { mButtonTouched[button] = false; }
}
