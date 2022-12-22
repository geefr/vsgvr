
#include "interaction_slide.h"
#include "game.h"

#include <vsg/all.h>

#include <iostream>

Interaction_slide::Interaction_slide(
  vsg::ref_ptr<vsgvr::Instance> xrInstance, 
  vsg::ref_ptr<vsgvr::ActionPoseBinding> leftHandPose, 
  vsg::ref_ptr<vsg::Group> ground)
    : _leftHandPose(leftHandPose)
    , _ground(ground)
{
  _actionSet = vsgvr::ActionSet::create(xrInstance, "slide", "Slide");

  // Pose bindings - One for each hand
  _strafeXAction = vsgvr::Action::create(xrInstance, _actionSet, XrActionType::XR_ACTION_TYPE_FLOAT_INPUT, "strafe_x", "Strafe X");
  _strafeYAction = vsgvr::Action::create(xrInstance, _actionSet, XrActionType::XR_ACTION_TYPE_FLOAT_INPUT, "strafe_y", "Strafe Y");
  _rotateAction = vsgvr::Action::create(xrInstance, _actionSet, XrActionType::XR_ACTION_TYPE_FLOAT_INPUT, "rotate", "Rotate");

  _actionSet->actions = {
    _strafeXAction,
    _strafeYAction,
    _rotateAction,
  };

  // Doesn't have a thumstick, nothing we can do (except map to controller aim + trigger)
  // _actionsToSuggest["/interaction_profiles/khr/simple_controller"] = {};

  _actionsToSuggest["/interaction_profiles/htc/vive_controller"] = {
    {_strafeXAction, "/user/hand/left/input/trackpad/x"},
    {_strafeYAction, "/user/hand/left/input/trackpad/y"},
    {_rotateAction, "/user/hand/right/input/trackpad/x"}
  };
  _actionsToSuggest["/interaction_profiles/oculus/touch_controller"] = {
    {_strafeXAction, "/user/hand/left/input/thumbstick/x"},
    {_strafeYAction, "/user/hand/left/input/thumbstick/y"},
    {_rotateAction, "/user/hand/right/input/thumbstick/x"}
  };
}

Interaction_slide::~Interaction_slide() {}

void Interaction_slide::frame(vsg::ref_ptr<vsg::Group> scene, Game& game, double deltaT)
{
  if (_rotateAction->getStateValid())
  {
    auto state = _rotateAction->getStateFloat();
    if (state.isActive && fabs(state.currentState) > deadZone)
    {
      _rotation += state.currentState * rotateSensitivity * deltaT * -1.0;
      game.userOrigin()->orientation = vsg::dquat(_rotation, { 0.0, 0.0, 1.0 });
    }
  }

  if (_leftHandPose->getTransformValid() && _strafeXAction->getStateValid() && _strafeYAction->getStateValid())
  {
    auto lt = game.userOrigin()->userToScene() * _leftHandPose->getTransform();
    auto lForward = (lt * vsg::dvec3{0.0, 0.0, -1.0}) - (lt * vsg::dvec3{0.0, 0.0, 0.0});
    lForward.z = 0;
    lForward = vsg::normalize(lForward);
    auto lRight = (lt * vsg::dvec3{ 1.0, 0.0, 0.0 }) - (lt * vsg::dvec3{ 0.0, 0.0, 0.0 });
    lRight.z = 0;
    lRight = vsg::normalize(lRight);

    auto xState = _strafeXAction->getStateFloat();
    auto yState = _strafeYAction->getStateFloat();
    if (xState.isActive && yState.isActive)
    {
      vsg::dvec3 d = (lRight * static_cast<double>(xState.currentState) ) + (lForward * static_cast<double>(yState.currentState) );
      if (vsg::length(d) > deadZone)
      {
        d *= strafeSensitivity * deltaT;
        game.userOrigin()->position += d;
      }
    }
  }
}
