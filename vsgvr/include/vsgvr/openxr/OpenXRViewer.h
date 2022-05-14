#pragma once

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

#include <vsg/viewer/Viewer.h>

#include <vsgvr/VR.h>
#include <vsgvr/openxr/OpenXRWindow.h>
#include <vsgvr/openxr/OpenXRContext.h>

namespace vsgvr {
class VSG_DECLSPEC OpenXRViewer final : public vsg::Inherit<vsg::Viewer, OpenXRViewer> {
public:
  OpenXRViewer(vsg::ref_ptr<vsgvr::OpenXRContext> ctx,
           vsg::ref_ptr<vsg::WindowTraits> windowTraits);
  OpenXRViewer(const OpenXRViewer &) = delete;
  Viewer &operator=(const OpenXRViewer &) = delete;

  // TODO: Adding desktop window not supported
  void addWindow(vsg::ref_ptr<vsg::Window> window) override;

  bool advanceToNextFrame() override;

  // TODO: Implement event handlers
  void handleEvents() override;

  void compile(vsg::ref_ptr<vsg::ResourceHints> hints = {}) override;

  bool acquireNextFrame() override;

  void update() override;
  void present() override;

  std::vector<vsg::ref_ptr<vsg::CommandGraph>> createCommandGraphsForView(vsg::ref_ptr<vsg::Node> vsg_scene);

private:
  // void createDesktopWindow(vsg::ref_ptr<vsg::WindowTraits> windowTraits);
  // void createHmdWindow(vsg::ref_ptr<vsg::WindowTraits> windowTraits);

  vsg::ref_ptr<vsgvr::OpenXRContext> m_ctx;

  // vsg::ref_ptr<vsg::Window> m_desktopWindow;
  vsg::ref_ptr<vsgvr::OpenXRWindow> m_hmdWindow;
};
} // namespace vsgvr
