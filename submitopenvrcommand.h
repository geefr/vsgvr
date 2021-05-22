#pragma once

#include <vsg/all.h>
#include "vr/env.h"

auto hmdImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

class SubmitOpenVRCommand : public vsg::Command
{
public:
  SubmitOpenVRCommand(vrhelp::Env *vr, vsg::ref_ptr<vsg::Window> window, uint32_t width, uint32_t height, 
                      vsg::ref_ptr<vsg::Image> leftImage, vsg::ref_ptr<vsg::Image> rightImage, vsg::ref_ptr<vsg::CommandGraph> commandGraph)
      : mVr(vr)
      , mWindow(window)
      , mWidth(width)
      , mHeight(height)
      , mLeftImage(leftImage)
      , mRightImage(rightImage)
      , mCommandGraph(commandGraph)
  {
  }

  void read(vsg::Input &input) override {}
  void write(vsg::Output &output) const override {}

  void compile(vsg::Context &context) override {}

  void record(vsg::CommandBuffer &commandBuffer) const override
  {
    // // Submit to SteamVR
    // vr::VRTextureBounds_t bounds;
    // bounds.uMin = 0.0f;
    // bounds.uMax = 1.0f;
    // bounds.vMin = 0.0f;
    // bounds.vMax = 1.0f;

    // auto lImg = mLeftImage->vk(mWindow->getDevice()->deviceID);
    // auto rImg = mRightImage->vk(mWindow->getDevice()->deviceID);
    
    // // TODO: Multisampling support
    // mVr->submitFrames(
    //   lImg,
    //   rImg,
    //   *mWindow->getDevice(),
    //   *mWindow->getPhysicalDevice(),
    //   *mWindow->getInstance(),
    //   *mWindow->getDevice()->getQueue(mCommandGraph->queueFamily),
    //   mCommandGraph->queueFamily,
    //   mWidth,
    //   mHeight,
    //   hmdImageFormat,
    //   VK_SAMPLE_COUNT_1_BIT
    // );
  }

protected:
  virtual ~SubmitOpenVRCommand() {}
  vrhelp::Env *mVr = nullptr;
  vsg::ref_ptr<vsg::Window> mWindow;
  uint32_t mWidth;
  uint32_t mHeight;
  vsg::ref_ptr<vsg::Image> mLeftImage;
  vsg::ref_ptr<vsg::Image> mRightImage;
  vsg::ref_ptr<vsg::CommandGraph> mCommandGraph;
};