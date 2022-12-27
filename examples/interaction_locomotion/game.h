#pragma once

#include <vsg/all.h>
#include <vsgvr/app/Viewer.h>
#include <vsgvr/app/UserOrigin.h>
#include <vsgvr/actions/Action.h>
#include <vsgvr/actions/ActionSet.h>
#include <vsgvr/actions/ActionPoseBinding.h>
#include <vsgvr/actions/SpaceBinding.h>

#include "interaction.h"

#include <memory>

class Game {
public:
  Game(vsg::ref_ptr<vsgvr::Instance> xrInstance, vsg::ref_ptr<vsgvr::Viewer> vr, vsg::ref_ptr<vsg::Viewer> desktopViewer);
  ~Game();

  bool shouldExit = false;

  void frame();

  vsg::ref_ptr<vsgvr::UserOrigin> userOrigin() const { return _userOrigin; }

private:
  void loadScene();
  void initVR();
  void initActions();

  vsg::ref_ptr<vsgvr::Instance> _xrInstance;
  vsg::ref_ptr<vsgvr::Viewer> _vr;
  vsg::ref_ptr<vsg::Viewer> _desktopViewer;

  // The user / OpenXR root space - Contains elements such as controllers
  vsg::ref_ptr<vsg::Group> _sceneRoot;
  vsg::ref_ptr<vsg::MatrixTransform> _controllerLeft;
  vsg::ref_ptr<vsg::MatrixTransform> _controllerRight;
  vsg::ref_ptr<vsg::Switch> _teleportMarker;
    
  // A transform allowing the player to move within the normal vsg scene
  vsg::ref_ptr<vsgvr::UserOrigin> _userOrigin;
  vsg::ref_ptr<vsg::Group> _ground;

  std::vector<vsg::ref_ptr<vsg::Camera>> _xrCameras;
  vsg::ref_ptr<vsg::Camera> _desktopCamera;

  // Actions and behaviours
  vsg::ref_ptr<vsgvr::ActionSet> _baseActionSet;
  vsg::ref_ptr<vsgvr::ActionPoseBinding> _leftHandPose;
  vsg::ref_ptr<vsgvr::ActionPoseBinding> _rightHandPose;
  vsg::ref_ptr<vsgvr::SpaceBinding> _headPose;

  std::map<std::string, std::unique_ptr<Interaction>> _interactions;

  vsg::time_point _lastFrameTime;
};
