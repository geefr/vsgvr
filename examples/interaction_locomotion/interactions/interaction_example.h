#pragma once

#include "interaction.h"

class Interaction_example : public Interaction
{
  public:
    Interaction_example() = delete;
    Interaction_example(vsg::ref_ptr<vsgvr::OpenXRInstance> xrInstance,
      vsg::ref_ptr<vsg::MatrixTransform> leftController, 
      vsg::ref_ptr<vsg::MatrixTransform> rightController);

    void frame(vsg::ref_ptr<vsg::Group> scene, Game& game) final override;
    virtual ~Interaction_example();
  protected:

    vsg::ref_ptr<vsg::MatrixTransform> _leftController;
    vsg::ref_ptr<vsg::MatrixTransform> _rightController;

    vsg::ref_ptr<vsgvr::OpenXRAction> _action;

    double _leftRot = 0.0;
    double _rightRot = 0.0;
};
