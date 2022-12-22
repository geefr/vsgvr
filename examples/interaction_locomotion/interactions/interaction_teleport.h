#pragma once

#include "interaction.h"

/// Basic user-teleport navigation, on left controller
/// * A button, Trackpad click, or trigger pull on controller -> Teleport to aimed target
/// * Thumstick on controller -> rotate in place
class Interaction_teleport : public Interaction
{
  public:
    Interaction_teleport() = delete;
    Interaction_teleport(vsg::ref_ptr<vsgvr::Instance> xrInstance, 
      vsg::ref_ptr<vsgvr::ActionPoseBinding> leftHandPose,
      vsg::ref_ptr<vsg::Switch> teleportTarget,
      vsg::ref_ptr<vsg::Group> ground);

    void frame(vsg::ref_ptr<vsg::Group> scene, Game& game, double deltaT) final override;
    virtual ~Interaction_teleport();
  protected:

    vsg::ref_ptr<vsgvr::Action> _teleportAction;
    vsg::ref_ptr<vsgvr::Action> _rotateAction;

    vsg::dvec3 _teleportPosition = {0.0, 0.0, 0.0};
    vsg::ref_ptr<vsg::Switch> _teleportTarget;
    vsg::ref_ptr<vsgvr::ActionPoseBinding> _leftHandPose;

    bool _teleportButtonDown = false;
    bool _teleportTargetValid = false;
    int _rotateActionState = 0; // -1 is rot left, 1 is rot right
    double _playerRotation = 0.0;

    vsg::ref_ptr<vsg::Group> _ground;
};
