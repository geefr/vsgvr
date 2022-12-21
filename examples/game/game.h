#pragma once

#include <vsg/all.h>
#include <vsgvr/app/OpenXRViewer.h>
#include <vsgvr/actions/OpenXRAction.h>
#include <vsgvr/actions/OpenXRActionSet.h>
#include <vsgvr/actions/OpenXRActionPoseBinding.h>

#include "interaction.h"

#include <memory>

class Game {
public:
  Game(vsg::ref_ptr<vsgvr::OpenXRInstance> xrInstance, vsg::ref_ptr<vsgvr::OpenXRViewer> vr, vsg::ref_ptr<vsg::Viewer> desktopViewer);
  ~Game();

  bool shouldExit = false;

  void frame();

  void setPlayerOriginInWorld(vsg::dvec3 position);
  vsg::dvec3 getPlayerOriginInWorld() { return _playerOriginInWorld; }
  
private:
  void loadScene();
  void initVR();
  void initActions();

  vsg::ref_ptr<vsgvr::OpenXRInstance> _xrInstance;
  vsg::ref_ptr<vsgvr::OpenXRViewer> _vr;
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
  std::vector<vsg::ref_ptr<vsg::Camera>> _xrCameras;
  vsg::ref_ptr<vsg::Camera> _desktopCamera;

  // Actions and behaviours
  vsg::ref_ptr<vsgvr::OpenXRActionSet> _baseActionSet;
  vsg::ref_ptr<vsgvr::OpenXRActionPoseBinding> _leftHandPose;
  vsg::ref_ptr<vsgvr::OpenXRActionPoseBinding> _rightHandPose;

  std::map<std::string, std::unique_ptr<Interaction>> _interactions;
};
