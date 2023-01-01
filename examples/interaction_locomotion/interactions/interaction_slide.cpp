
#include "interaction_slide.h"
#include "game.h"

#include <vsg/all.h>

#include <iostream>

Interaction_slide::Interaction_slide(
  vsg::ref_ptr<vsgvr::Instance> xrInstance,
  vsg::ref_ptr<vsgvr::SpaceBinding> headPose,
  vsg::ref_ptr<vsgvr::ActionPoseBinding> leftHandPose, 
  vsg::ref_ptr<vsg::Node> ground)
    : _headPose(headPose)
    , _leftHandPose(leftHandPose)
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
  bool updateOrigin = false;

  // The player's position in vr space, at 'ground' level
  // In most cases this will simply be playerPosOrigin.z = 0,
  // though an intersection to the scene would be more appropriate in many cases
  auto playerPosUser = _headPose->getTransform() * vsg::dvec3{ 0.0, 0.0, 0.0 };
  playerPosUser.z = 0.0;
  
  // Player's position in the scene - Strafing is performed in scene space
  auto newPositionScene = game.userOrigin()->userToScene() * playerPosUser;

  if (_rotateAction->getStateValid())
  {
    auto state = _rotateAction->getStateFloat();
    if (state.isActive && fabs(state.currentState) > deadZone)
    {
      _rotation += state.currentState * rotateSensitivity * deltaT * -1.0;
      updateOrigin = true;
    }
  }

  if (_leftHandPose->getTransformValid() && _strafeXAction->getStateValid() && _strafeYAction->getStateValid())
  {
    // Direction vectors in scene space, based on left controller
    // (User may point controller to modify strafe direction)
    // Alternatively the head's pose could be used to determine forward,
    // but that would prevent strafing sideways while looking sideways
    //
    // Here all strafing is performed in the xy plane, ignoring height differences
    // of the ground.
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
        newPositionScene += d;
        updateOrigin = true;
      }
    }
  }

  if( updateOrigin )
  {
    // Update the vsg scene's transform to strafe player as needed
    game.userOrigin()->setUserInScene(
      playerPosUser,
      newPositionScene,
      vsg::dquat(_rotation, { 0.0, 0.0, 1.0 }),
      { 1.0, 1.0, 1.0 }
    );
  }
}
