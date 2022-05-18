
namespace {
  const char* to_string(XrResult result) {
    switch (result) {
#define RESULT_CASE(V, _) \
      case V: return #V;
      XR_LIST_ENUM_XrResult(RESULT_CASE);
    default: return "Unknown";
    }
  }
  const char* to_string(XrSessionState result) {
    switch (result) {
#define RESULT_CASE(V, _) \
      case V: return #V;
      XR_LIST_ENUM_XrSessionState(RESULT_CASE);
    default: return "Unknown";
    }
  }
  void xr_check(XrResult result, std::string msg = "") {
    if (XR_SUCCEEDED(result)) return;
    vsg::Exception e;
    e.message = msg + " (" + to_string(result) + ")";
    throw e;
  }
  PFN_xrVoidFunction xr_pfn(XrInstance instance, std::string name) {
    PFN_xrVoidFunction fn = nullptr;
    xr_check(xrGetInstanceProcAddr(instance, name.c_str(), &fn),
      "Failed to look up function: " + name);
    return fn;
  }
}
