
#include "interaction_example.h"
#include "game.h"

#include <vsg/all.h>

Interaction_example::Interaction_example(vsg::ref_ptr<vsgvr::OpenXRInstance> xrInstance,
  vsg::ref_ptr<vsg::MatrixTransform> leftController,
  vsg::ref_ptr<vsg::MatrixTransform> rightController)
  : _leftController(leftController)
  , _rightController(rightController)
{
  _actionSet = vsgvr::OpenXRActionSet::create(xrInstance, "example_interaction", "Example Interaction");
  _action = vsgvr::OpenXRAction::create(xrInstance, _actionSet, XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT,
    "controller_spin", "Spin Controller", std::vector<std::string>{"/user/hand/left", "/user/hand/right"});
  _actionSet->actions = { _action };

  _actionsToSuggest["/interaction_profiles/khr/simple_controller"] = {
    {_action, "/user/hand/left/input/select/click"},
    {_action, "/user/hand/right/input/select/click"},
  };
  _actionsToSuggest["/interaction_profiles/oculus/touch_controller"] = {
    {_action, "/user/hand/left/input/trigger/click"},
    {_action, "/user/hand/right/input/trigger/click"},
  };
}

Interaction_example::~Interaction_example() {}

void Interaction_example::frame(vsg::ref_ptr<vsg::Group> scene, Game& game)
{
  // Quick input test - When triggers pressed (select action binding) make controllers spin
  // Note coordinate space of controllers - Z is forward
  if (_action->getStateValid("/user/hand/left"))
  {
    auto lState = _action->getStateBool("/user/hand/left");
    if (lState.isActive && lState.currentState)
    {
      _leftRot -= 0.1;
    }
  }
  if (_action->getStateValid("/user/hand/right"))
  {
    auto rState = _action->getStateBool("/user/hand/right");
    if (rState.isActive && rState.currentState)
    {
      _rightRot += 0.1;
    }
  }
  _leftController->matrix = _leftController->matrix * vsg::rotate(_leftRot, { 0.0, 0.0, 1.0 });
  _rightController->matrix = _rightController->matrix * vsg::rotate(_rightRot, { 0.0, 0.0, 1.0 });
}
