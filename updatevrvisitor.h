#pragma once

#include <vsg/all.h>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>

const std::string OpenVRIDTag = "OpenVR ID";
const std::string LeftControllerID = "Left Controller";
const std::string RightControllerID = "Right Controller";

class UpdateVRVisitor : public vsg::Visitor
{
public:
  UpdateVRVisitor() = delete;
  UpdateVRVisitor(vrhelp::Env *vr, vrhelp::Controller *left, vrhelp::Controller *right, vrhelp::Device *hmd)
      : mVr(vr), mLeft(left), mRight(right), mHmd(hmd) {}

  virtual void apply(vsg::Group &o) override
  {
    // TODO: Need to convert controller positions into vsg world space
    std::string v;
    if (mLeft)
    {
      if (o.getValue(OpenVRIDTag, v) && v == LeftControllerID)
      {
        if (auto txf = o.cast<vsg::MatrixTransform>())
        {
          auto mat = mLeft->deviceToAbsoluteMatrix();
          vsg::dmat4 vmat(glm::value_ptr(mat));
          txf->setMatrix(vmat);
        }
      }
    }
    if (mRight)
    {
      if (o.getValue(OpenVRIDTag, v) && v == RightControllerID)
      {
        if (auto txf = o.cast<vsg::MatrixTransform>())
        {
          auto mat = mRight->deviceToAbsoluteMatrix();
          vsg::dmat4 vmat(glm::value_ptr(mat));
          txf->setMatrix(vmat);
        }
      }
    }

    o.traverse(*this);
  }

private:
  vrhelp::Env *mVr = nullptr;
  vrhelp::Controller *mLeft = nullptr;
  vrhelp::Controller *mRight = nullptr;
  vrhelp::Device *mHmd = nullptr;
};