
#include "interaction_teleport.h"
#include "game.h"

#include <vsg/all.h>

#include <iostream>

Interaction_teleport::Interaction_teleport(vsg::ref_ptr<vsgvr::Instance> xrInstance,
  vsg::ref_ptr<vsgvr::ActionPoseBinding> leftHandPose,
  vsg::ref_ptr<vsg::Switch> teleportTarget,
  vsg::ref_ptr<vsg::Group> ground)
  : _leftHandPose(leftHandPose)
  , _teleportTarget(teleportTarget)
  , _ground(ground)
{
  _actionSet = vsgvr::ActionSet::create(xrInstance, "teleport", "Teleport");

  // Pose bindings - One for each hand
  _teleportAction = vsgvr::Action::create(xrInstance, _actionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT, "teleport", "Teleport");
  _rotateAction = vsgvr::Action::create(xrInstance, _actionSet, XrActionType::XR_ACTION_TYPE_FLOAT_INPUT, "rotate", "Rotate");

  _actionSet->actions = {
    _teleportAction,
    _rotateAction,
  };

  _actionsToSuggest["/interaction_profiles/khr/simple_controller"] = {
    {_teleportAction, "/user/hand/left/input/select/click"},
  };
  _actionsToSuggest["/interaction_profiles/oculus/touch_controller"] = {
    {_teleportAction, "/user/hand/left/input/x/click"},
    {_rotateAction, "/user/hand/left/input/thumbstick/x"}
  };
}

Interaction_teleport::~Interaction_teleport() {}

void Interaction_teleport::frame(vsg::ref_ptr<vsg::Group> scene, Game& game)
{
  if (_teleportAction->getStateValid())
  {
    auto state = _teleportAction->getStateBool();
    if (state.isActive && state.currentState && !_teleportButtonDown)
    {
      _teleportButtonDown = true;
    }
  }
  double rotThresh = 0.25;
  if (_rotateAction->getStateValid())
  {
    auto state = _rotateAction->getStateFloat();
    if (state.isActive && _rotateActionState == 0)
    {
      if( state.currentState < -rotThresh)  _rotateActionState = -1;
      else if( state.currentState > rotThresh) _rotateActionState = 1;

      if (_rotateActionState != 0)
      {
        _playerRotation += (- 15.0 * _rotateActionState);
        auto rot = vsg::dquat(vsg::radians(_playerRotation), {0.0, 0.0, 1.0});
        game.setPlayerRotationInWorld(rot);
      }
    }
  }
    
  if (_teleportButtonDown && _leftHandPose->getTransformValid())
  {
    // Raycast from controller aim to world, colliding with anything named "ground"
    // https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-standard-pose-identifiers
    // In XR the 'world' space is fixed to our physical room, with the game world sliding around beneath us
    vsg::dvec3 intersectStart = game.getPlayerTransform() * (_leftHandPose->getTransform() * vsg::dvec3{0.0, 0.0, 0.0});
    vsg::dvec3 intersectEnd = game.getPlayerTransform() * (_leftHandPose->getTransform() * vsg::dvec3{0.0, 0.0, -100.0});

    auto intersector = vsg::LineSegmentIntersector::create(intersectStart, intersectEnd);
    // TODO: Intersect whole scene, or sub-scene matched on tags? For now we can't be anywhere but the ground plane
    _ground->accept(*intersector);
    if (!intersector->intersections.empty())
    {
      _teleportTargetValid = true;
      _teleportTarget->setAllChildren(true);

      _teleportPosition = intersector->intersections.front()->worldIntersection;

      for (auto& child : _teleportTarget->children)
      {
        if (auto m = child.node->cast<vsg::MatrixTransform>())
        {
          // TODO: Double check this rotate - Due to the wonky axis mapping / Need some helpers for vsgworld_to_vsgvrworld?
          // TODO: Or, put markers within the vsg world space, with just pose bindings and such being special -> Forcing everyone to have an
          //       extra 'world' node at the top of their scene graph?

          // Doing the same transform as the 'world' eventually gets in game - TODO: Seriously this super-space stuff should be in vsgvr and hidden
          m->matrix = vsg::inverse(game.getPlayerTransform()) * vsg::translate(_teleportPosition) * vsg::rotate(vsg::radians(90.0), {1.0, 0.0, 0.0});
        }
      }
    }
    else
    {
      _teleportTargetValid = false;
      _teleportTarget->setAllChildren(false);
    }
  }

  if (_teleportAction->getStateValid())
  {
    auto state = _teleportAction->getStateBool();
    if (state.isActive && state.currentState == false && _teleportButtonDown)
    {
      if (_teleportTargetValid)
      {
        game.setPlayerOriginInWorld(_teleportPosition);
      }
      _teleportButtonDown = false;
      _teleportTargetValid = false;
      _teleportTarget->setAllChildren(false);
    }
  }
  if (_rotateAction->getStateValid())
  {
    auto state = _rotateAction->getStateFloat();
    if (state.isActive && _rotateActionState != 0)
    {
      if( fabs(state.currentState) < rotThresh) _rotateActionState = 0;
    }
  }
}
