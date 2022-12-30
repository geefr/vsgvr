#pragma once

#include <vsgvr/xr/Instance.h>
#include <vsgvr/actions/Action.h>
#include <vsgvr/actions/ActionSet.h>
#include <vsgvr/actions/ActionPoseBinding.h>
#include <vsgvr/actions/SpaceBinding.h>

#include <list>
#include <map>

class Game;

class Interaction
{
  public:
    vsg::ref_ptr<vsgvr::ActionSet> actionSet() const { return _actionSet; }
    std::map<std::string, std::list<vsgvr::ActionSet::SuggestedInteractionBinding>> actionsToSuggest() const { return _actionsToSuggest; }

    virtual void frame(vsg::ref_ptr<vsg::Group> scene, Game& game, double deltaT) = 0;
    virtual ~Interaction();
  protected:
    Interaction();
    vsg::ref_ptr<vsgvr::ActionSet> _actionSet;
    std::map<std::string, std::list<vsgvr::ActionSet::SuggestedInteractionBinding>> _actionsToSuggest;
};
