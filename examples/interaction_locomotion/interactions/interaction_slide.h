#pragma once

#include "interaction.h"

/// Basic user-slide navigation
/// * Thumstick on left controller -> strafe, relative to controller aim
/// * Thumbstick on right controller -> rotate
class Interaction_slide : public Interaction
{
  public:
    Interaction_slide() = delete;
    Interaction_slide(vsg::ref_ptr<vsgvr::Instance> xrInstance,
      vsg::ref_ptr<vsgvr::SpaceBinding> headPose,
      vsg::ref_ptr<vsgvr::ActionPoseBinding> leftHandPose, 
      vsg::ref_ptr<vsg::Node> ground);

    void frame(vsg::ref_ptr<vsg::Group> scene, Game& game, double deltaT) final override;
    virtual ~Interaction_slide();

    double strafeSensitivity = 1.8; // m/s
    double rotateSensitivity = 1.4; // rad/s
    double deadZone = 0.1;
  protected:

    vsg::ref_ptr<vsgvr::Action> _strafeXAction;
    vsg::ref_ptr<vsgvr::Action> _strafeYAction;
    vsg::ref_ptr<vsgvr::Action> _rotateAction;
    vsg::ref_ptr<vsgvr::SpaceBinding> _headPose;
    vsg::ref_ptr<vsgvr::ActionPoseBinding> _leftHandPose;

    vsg::ref_ptr<vsg::Node> _ground;
    double _rotation = 0.0;
};
