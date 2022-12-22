#pragma once

#include <vsg/all.h>
#include <vsgvr/app/Viewer.h>
#include <vsgvr/actions/Action.h>
#include <vsgvr/actions/ActionSet.h>
#include <vsgvr/actions/ActionPoseBinding.h>

#include "interaction.h"

#include <memory>

class Game {
public:
  Game(vsg::ref_ptr<vsgvr::Instance> xrInstance, vsg::ref_ptr<vsgvr::Viewer> vr, vsg::ref_ptr<vsg::Viewer> desktopViewer);
  ~Game();

  bool shouldExit = false;

  void frame();

  void setPlayerOriginInWorld(vsg::dvec3 position);
  vsg::dvec3 getPlayerOriginInWorld() { return _playerOriginInWorld; }

  void setPlayerRotationInWorld(vsg::dquat rotation);
  vsg::dquat getPlayerRotationInWorld() { return _playerRotationInWorld; }

  vsg::dmat4 getPlayerTransform();
  
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

  // World space - TODO: For now, moving the world around rather than the OpenXR space
  //                     but should also be able to re-locate the OpenXR session for
  //                     a more natural xr-origin style setup?
  vsg::ref_ptr<vsg::MatrixTransform> _scene;
  vsg::ref_ptr<vsg::Group> _ground;

  vsg::dvec3 _playerOriginInWorld = {0.0, 0.0, 0.0};
  vsg::dquat _playerRotationInWorld = vsg::dquat(0.0, {0.0, 0.0, 0.0});
  bool _spaceChangePending = false;

  std::vector<vsg::ref_ptr<vsg::Camera>> _xrCameras;
  vsg::ref_ptr<vsg::Camera> _desktopCamera;

  // Actions and behaviours
  vsg::ref_ptr<vsgvr::ActionSet> _baseActionSet;
  vsg::ref_ptr<vsgvr::ActionPoseBinding> _leftHandPose;
  vsg::ref_ptr<vsgvr::ActionPoseBinding> _rightHandPose;

  std::map<std::string, std::unique_ptr<Interaction>> _interactions;
};
