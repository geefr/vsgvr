
#include "interaction_teleport.h"
#include "game.h"

#include <vsg/all.h>

#include <iostream>

Interaction_teleport::Interaction_teleport(vsg::ref_ptr<vsgvr::Instance> xrInstance,
  vsg::ref_ptr<vsgvr::SpaceBinding> headPose,
  vsg::ref_ptr<vsgvr::ActionPoseBinding> leftHandPose,
  vsg::ref_ptr<vsg::Switch> teleportTarget,
  vsg::ref_ptr<vsg::Group> ground)
  : _headPose(headPose)
  , _leftHandPose(leftHandPose)
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
  _actionsToSuggest["/interaction_profiles/htc/vive_controller"] = {
    {_teleportAction, "/user/hand/left/input/trackpad/click"},
    {_rotateAction, "/user/hand/left/input/trackpad/x"}
  };
  _actionsToSuggest["/interaction_profiles/oculus/touch_controller"] = {
    {_teleportAction, "/user/hand/left/input/x/click"},
    {_rotateAction, "/user/hand/left/input/thumbstick/x"}
  };
}

Interaction_teleport::~Interaction_teleport() {}

void Interaction_teleport::frame(vsg::ref_ptr<vsg::Group> scene, Game& game, double deltaT)
{
  auto applyRotation = false;
  auto applyTeleport = false;

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
        applyRotation = true;
      }
    }
  }

  if (_teleportButtonDown && _leftHandPose->getTransformValid())
  {
    // Raycast from controller aim to world, colliding with anything named "ground"
    // https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-standard-pose-identifiers
    // In XR the 'world' space is fixed to our physical room, with the game world sliding around beneath us
    // so this is simply an intersection to the target point, and later converting it through to the scene
    // space to update the vsgvr::UserOrigin
    vsg::dvec3 intersectStart = _leftHandPose->getTransform() * vsg::dvec3{0.0, 0.0, 0.0};
    vsg::dvec3 intersectEnd = _leftHandPose->getTransform() * vsg::dvec3{0.0, 0.0, -100.0};

    auto intersector = vsg::LineSegmentIntersector::create(intersectStart, intersectEnd);
    // TODO: Intersect whole scene, or sub-scene matched on tags? For now we can't be anywhere but the ground plane
    // TODO: Should sort intersections as well - For now intersections.back() seems to always get the ground plane,
    //       and not other geometry within the world.
    _ground->accept(*intersector);
    if (!intersector->intersections.empty())
    {
      _teleportTargetValid = true;
      _teleportTarget->setAllChildren(true);

      _teleportPosition = intersector->intersections.back()->worldIntersection;

      for (auto& child : _teleportTarget->children)
      {
        if (auto m = child.node->cast<vsg::MatrixTransform>())
        {
          // TODO: The rotate here shouldn't be needed - we're overriding the root transform in the model however
          m->matrix = vsg::translate(_teleportPosition) * vsg::rotate(vsg::radians(90.0), {1.0, 0.0, 0.0});
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
        // Teleport position within the child scene
        // Here the origin is being moved, by the difference between the teleport target, and where the user currently is (left hand)
        applyTeleport = true;
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

  if( applyRotation || applyTeleport )
  {
    // The player's position in vr space, at 'ground' level
    auto playerPosOrigin = _headPose->getTransform() * vsg::dvec3{ 0.0, 0.0, 0.0 };
    playerPosOrigin.z = _teleportPosition.z;

    vsg::dvec3 newPlayerPosScene;
    if (!applyTeleport)
    {
      // The player hasn't moved
      newPlayerPosScene = game.userOrigin()->userToScene() * playerPosOrigin;
    }
    else
    {
      // The player has moved - Center on the teleport target
      newPlayerPosScene = game.userOrigin()->userToScene() * _teleportPosition;
    }

    // Set the transform from vr origin to position/rotation within scene
    // The initial translate is important as the user is rarely positioned 
    // at the origin of their vr space.
    game.userOrigin()->matrix =
      vsg::translate(newPlayerPosScene) *
      vsg::rotate(vsg::radians(_playerRotation), { 0.0, 0.0, 1.0 }) *
      vsg::scale(1.0, 1.0, 1.0) *
      vsg::translate(-playerPosOrigin);
  }
}
