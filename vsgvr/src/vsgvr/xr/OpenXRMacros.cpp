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
  PFN_xrVoidFunction xr_pfn_noexcept(XrInstance instance, std::string name) {
    PFN_xrVoidFunction fn = nullptr;
    auto res = xrGetInstanceProcAddr(instance, name.c_str(), &fn);
    if( XR_SUCCEEDED(res) ) return fn;
    else return nullptr;
  }
}

