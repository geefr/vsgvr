#pragma once

#include <vsgvr/xr/OpenXRInstance.h>
#include <vsgvr/actions/OpenXRAction.h>
#include <vsgvr/actions/OpenXRActionSet.h>
#include <vsgvr/actions/OpenXRActionPoseBinding.h>

#include <list>
#include <map>

class Game;

class Interaction
{
  public:
    vsg::ref_ptr<vsgvr::OpenXRActionSet> actionSet() const { return _actionSet; }
    std::map<std::string, std::list<vsgvr::OpenXRActionSet::SuggestedInteractionBinding>> actionsToSuggest() const { return _actionsToSuggest; }

    virtual void frame(vsg::ref_ptr<vsg::Group> scene, Game& game) = 0;
    virtual ~Interaction();
  protected:
    Interaction();
    vsg::ref_ptr<vsgvr::OpenXRActionSet> _actionSet;
    std::map<std::string, std::list<vsgvr::OpenXRActionSet::SuggestedInteractionBinding>> _actionsToSuggest;
};
