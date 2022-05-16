
#include <vsgvr/openxr/OpenXRTraits.h>

namespace vsgvr
{
  OpenXrTraits::OpenXrTraits() {}

  void OpenXrTraits::setApplicationVersion(uint32_t maj, uint32_t min, uint32_t patch)
  {
    applicationVersion = XR_MAKE_VERSION(maj, min, patch);
  }
  void OpenXrTraits::setEngineVersion(uint32_t maj, uint32_t min, uint32_t patch)
  {
    engineVersion = XR_MAKE_VERSION(maj, min, patch);
  }
}
