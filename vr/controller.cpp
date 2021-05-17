
#include "controller.h"

namespace vrhelp {
  Controller::Controller(vr::IVRSystem* context, uint32_t id, vr::ETrackedDeviceClass devClass)
  : Device(context,id,devClass)
  {
    mRole = context->GetInt32TrackedDeviceProperty(mID, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32);
  }

  Controller::~Controller() {}
}
