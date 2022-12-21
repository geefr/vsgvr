#pragma once

#include "interaction.h"

class Interaction_teleport : public Interaction
{
  public:
    Interaction_teleport() = delete;
    Interaction_teleport(vsg::ref_ptr<vsgvr::OpenXRInstance> xrInstance, 
      vsg::ref_ptr<vsgvr::OpenXRActionPoseBinding> leftHandPose,
      vsg::ref_ptr<vsg::Switch> teleportTarget,
      vsg::ref_ptr<vsg::Group> ground);

    void frame(vsg::ref_ptr<vsg::Group> scene, Game& game) final override;
    virtual ~Interaction_teleport();
  protected:

    vsg::ref_ptr<vsgvr::OpenXRAction> _teleportAction;
    vsg::ref_ptr<vsgvr::OpenXRAction> _rotateAction;

    vsg::dvec3 _teleportPosition = {0.0, 0.0, 0.0};
    vsg::ref_ptr<vsg::Switch> _teleportTarget;
    vsg::ref_ptr<vsgvr::OpenXRActionPoseBinding> _leftHandPose;

    bool _teleportButtonDown = false;
    bool _teleportTargetValid = false;
    int _rotateActionState = 0; // -1 is rot left, 1 is rot right
    double _playerRotation = 0.0;

    vsg::ref_ptr<vsg::Group> _ground;
};
