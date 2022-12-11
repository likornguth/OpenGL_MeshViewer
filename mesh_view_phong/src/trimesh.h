//! \file   TriMesh.h
//! \brief  TriMesh class
//! \author Lindsay Kornguth, 2022

#pragma once
// From implementation of sphere class
#include <string>
#include <memory>
#include "types.h"
#include "utils/utils.h"
#include "utils/material.h"

// OpenMesh::TriMesh_ArrayKernelT
#include <boost/filesystem.hpp>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Geometry/EigenVectorT.hh>


namespace olio {
// class GLDrawData
class GLDrawData;
// use Eigen instead of OpenMesh's default structures for Point and
// Normal

struct EigenMeshTraits : OpenMesh::DefaultTraits {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using Point  = Vec3r;
  using Normal = Vec3r;
  using TexCoord2D = Vec2r;
};
using OMTriMesh = OpenMesh::TriMesh_ArrayKernelT<EigenMeshTraits>;

class TriMesh : public OMTriMesh {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    using Ptr = std::shared_ptr<TriMesh>;

    explicit TriMesh(const std::string &name=std::string());

    // TriMesh(const TriMesh &) = delete;
    // TriMesh(TriMesh &&) = delete;
    // TriMesh& operator=(const TriMesh &) = delete;
    // TriMesh& operator=(TriMesh &&) = delete;
    ~TriMesh();

    // TriMesh member functions
    bool Load(const boost::filesystem::path &filepath);
    void GetBoundingBox(Vec3r &bmin, Vec3r &bmax);
    bool ComputeFaceNormals();
    bool ComputeVertexNormals();

    void SetFilePath(const boost::filesystem::path &filepath)
        {filepath_ = filepath;}

    boost::filesystem::path GetFilePath() const {return filepath_;}

    // opengl
    void DeleteGLBuffers();
    void UpdateGLBuffers(bool force_update=false);
    void DrawGL(const GLDrawData &draw_data);
protected:
  boost::filesystem::path filepath_;
  std:: string name_;
  size_t vertex_count_ = 0;
  size_t face_indices_count_ = 0;
    // opengl
  bool gl_buffers_dirty_ = false;
  GLuint positions_normals_vbo_{0};
  GLuint faces_ebo_{0};
  
};

}  // namespace olio