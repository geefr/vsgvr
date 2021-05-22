#pragma once

#include <vsg/viewer/ProjectionMatrix.h>
#include <vsg/viewer/ViewMatrix.h>

/*
 * Matrix classes that allow explicitly setting a matrix
 * TODO: These might not be needed now - Were based on an old vsg version
 */
class RawProjectionMatrix : public vsg::ProjectionMatrix
{
public:
  RawProjectionMatrix(vsg::dmat4 mat) : mMat(mat) {}
  void get(vsg::mat4 &matrix) const override { matrix = mMat; }
  void get(vsg::dmat4 &matrix) const override { matrix = mMat; }

private:
  vsg::dmat4 mMat;
};

class RawViewMatrix : public vsg::ViewMatrix
{
public:
  RawViewMatrix(vsg::dmat4 mat) : mMat(mat) {}
  void get(vsg::mat4 &matrix) const override { matrix = mMat; }
  void get(vsg::dmat4 &matrix) const override { matrix = mMat; }

private:
  vsg::dmat4 mMat;
};