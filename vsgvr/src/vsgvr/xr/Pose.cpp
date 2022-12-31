
#include <vsgvr/xr/Pose.h>

namespace vsgvr {

  Pose::Pose(XrPosef pose)
    : _pose(pose)
  {}

  Pose::Pose(vsg::dvec3 position, vsg::dquat orientation)
  {
    setPose(position, orientation);
  }

  Pose::Pose(vsg::vec3 position, vsg::quat orientation)
  {
    setPose(position, orientation);
  }

  Pose::~Pose() {}

  void Pose::setPose(XrPosef pose)
  {
    _pose = pose;
  }

  void Pose::setPose(vsg::dvec3 position, vsg::dquat orientation)
  {
    vsg::dmat4 vsgToXr(
      1.0, 0.0, 0.0, 0.0,
      0.0, 0.0, -1.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 1.0
    );
    vsg::vec3 xrp = static_cast<vsg::vec3>(vsgToXr * position);

    // TODO: I'm a little unsure of why w needed to be inverted here - It may be compensating bugs in ViewMatrix or ProjectionMatrix
    // IMPORTANT: OpenXR is required to raise an error if quaternion length is >1% away from unit
    auto xro = static_cast<vsg::quat>(vsg::dquat(
      orientation.x,
      orientation.z,
      -orientation.y,
      -orientation.w
    ));
    xro = vsg::normalize(xro);

    _pose = XrPosef{
      XrQuaternionf{xro.x, xro.y, xro.z, xro.w},
      XrVector3f{xrp.x, xrp.y, xrp.z}
    };
  }

  void Pose::setPose(vsg::vec3 position, vsg::quat orientation)
  {
    setPose(static_cast<vsg::dvec3>(position), static_cast<vsg::dquat>(orientation));
  }
}
