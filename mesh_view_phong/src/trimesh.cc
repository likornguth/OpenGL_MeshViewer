#include "trimesh.h"
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>
#include "utils/gldrawdata.h"
#include "utils/glshader.h"





namespace olio {

using namespace std;
namespace fs=boost::filesystem;


TriMesh::TriMesh(const std::string &name) :
  OMTriMesh{}
{
  gl_buffers_dirty_ = true;
  name_ = name.size() ? name : "TriMesh";
}





// fill in vec3r bmin and bmax with the bottom left close corner 
// and top right far corner coordinates
void
TriMesh::GetBoundingBox(Vec3r &bmin, Vec3r &bmax)
{
    // iterate over every vertex
    for (auto vit =  vertices_begin(); vit != vertices_end(); ++vit){
      Vec3r point = this->point(*vit);
      bmin[0] = std::min(bmin[0], point[0]);
      bmin[1] = std::min(bmin[1], point[1]);
      bmin[2] = std::min(bmin[2], point[2]);
      bmax[0] = std::max(bmax[0], point[0]);
      bmax[1] = std::max(bmax[1], point[1]);
      bmax[2] = std::max(bmax[2], point[2]);
    }
}



TriMesh::~TriMesh()
{
  DeleteGLBuffers();
}

void
TriMesh::DeleteGLBuffers()
{
  // delete vbos
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  if (positions_normals_vbo_) {
    glDeleteBuffers(1, &positions_normals_vbo_);
    positions_normals_vbo_ = 0;
  }

  // delete ebos
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  if (faces_ebo_) {
    glDeleteBuffers(1, &faces_ebo_);
    faces_ebo_ = 0;
  }
}

bool 
TriMesh::Load(const fs::path &filepath)
{
  OpenMesh::IO::Options opts = OpenMesh::IO::Options::VertexTexCoord |
    OpenMesh::IO::Options::VertexNormal |
    OpenMesh::IO::Options::FaceNormal;

  // save name
  filepath_ = filepath;

  // request vertex texture coordinates
  request_vertex_texcoords2D();
  if (!has_vertex_texcoords2D()) {
    spdlog::error("request to allocate vertex texture coordinates failed.");
    return false;
  }

  // request vertex and face normals
  request_vertex_normals();
  if(!has_vertex_normals()) {
    spdlog::error("request to allocate vertex normals failed.");
    return false;
  }
  request_face_normals();
  if(!has_face_normals()) {
    spdlog::error("request to allocate face normals failed.");
    return false;
  }

  // read in mesh
  auto filename = filepath_.string();
  spdlog::info ("loading {}...", filename);
  if (!OpenMesh::IO::read_mesh(*this, filename, opts)) {
    spdlog::error("could not load mesh from {}", filename);
    return false;
  }
  spdlog::info("done. (vertices: {}, edges: {}, faces: {})",
               n_vertices(), n_edges(), n_faces());

  // delete vertex texcoord2ds if file did not have them
  if (opts.check(OpenMesh::IO::Options::VertexTexCoord))
    spdlog::info("mesh has texture coordinates");
  else
    release_vertex_texcoords2D();

  // check if the mesh file contained vertex (or face normals) normals
  // if not, compute normals
  if(!opts.check(OpenMesh::IO::Options::FaceNormal))
    update_face_normals();
  if(!opts.check(OpenMesh::IO::Options::VertexNormal))
    update_vertex_normals();
  if (has_halfedge_normals())
    update_halfedge_normals();

  // dirty bound
  // bound_dirty_ = true;
  return true;

}

bool TriMesh::ComputeFaceNormals()
{
  auto FaceNormal = [&](TriMesh::FaceHandle fh) {
    auto heh = halfedge_handle(fh);
    Vec3r p0 = point(from_vertex_handle(heh));
    Vec3r p1 = point(to_vertex_handle(heh));
    auto vh2 = to_vertex_handle(next_halfedge_handle(heh));
    Vec3r p2 = point(vh2);
    Vec3r face_normal = (p1 - p0).cross(p2 - p0);
    return face_normal.normalized();
  };

  if (!has_face_normals())
    return false;
  for (auto fit = faces_begin(); fit != faces_end(); ++fit)
    set_normal(*fit, FaceNormal(*fit));
  return true;
  
}

bool TriMesh::ComputeVertexNormals()
{
  auto VertexNormal = [&](TriMesh::VertexHandle vh) {
     Vec3r vertex_normal{0, 0, 0};
     for (auto vfit = vf_iter(vh); vfit.is_valid(); ++vfit)
       vertex_normal += normal(*vfit);
     vertex_normal.normalize();
     return vertex_normal;
  };

  if (!has_vertex_normals() || !has_face_normals())
    return false;
  for (auto vit = vertices_begin(); vit != vertices_end(); ++vit)
    set_normal(*vit, VertexNormal(*vit));
  return true;

}


void
TriMesh::UpdateGLBuffers(bool force_update)
{
  if (!gl_buffers_dirty_ && !force_update)
    return;

  // delete existing VBOs
  DeleteGLBuffers();

  vector<Vec3r> positions, normals;
  vector<int> face_indices;
//   BuildWatertightSphere(center_, radius_, grid_nx_, grid_ny_, positions, normals,
//                         face_indices);
    Load(filepath_);

    // fill positions, normals, and face_indices arrrays
    for ( auto vit =  vertices_begin(); vit != vertices_end(); ++vit) {
        // store Vec3r of coordinate position in positions
        positions.push_back(point(*vit));
        // store Vec3r of vertex normal in normals
        normals.push_back(normal(*vit));
    }
    for (auto fit = faces_begin(); fit != faces_end(); ++fit) {
        //face_indices.push_back(fit->idx());
        for(auto fv_it = fv_iter(*fit); fv_it.is_valid(); ++fv_it){
          face_indices.push_back(fv_it->idx());
        }
    }

  // interleave positions and normals
  vector<GLfloat> positions_normals;
  for (size_t i = 0; i < positions.size(); ++i) {
    // position
    positions_normals.push_back(static_cast<GLfloat>(positions[i][0]));
    positions_normals.push_back(static_cast<GLfloat>(positions[i][1]));
    positions_normals.push_back(static_cast<GLfloat>(positions[i][2]));
    // normal
    positions_normals.push_back(static_cast<GLfloat>(normals[i][0]));
    positions_normals.push_back(static_cast<GLfloat>(normals[i][1]));
    positions_normals.push_back(static_cast<GLfloat>(normals[i][2]));
  }
  vertex_count_ = positions.size();
  face_indices_count_ = face_indices.size();

  // create VBO for positions and normals
  glGenBuffers(1, &positions_normals_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, positions_normals_vbo_);
  glBufferData(GL_ARRAY_BUFFER, positions_normals.size() * sizeof(GLfloat),
               &positions_normals[0], GL_STATIC_DRAW);

  // create elements array
  vector<GLuint> faces;
  faces.reserve(face_indices.size());
  for (auto index : face_indices)
    faces.push_back(static_cast<GLuint>(index));

  // create EBO for faces
  glGenBuffers(1, &faces_ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLuint),
               &faces[0], GL_STATIC_DRAW);

  gl_buffers_dirty_ = false;
}


void
TriMesh::DrawGL(const GLDrawData &draw_data)
{
  // check we have a valid material and shader
  auto material = draw_data.GetMaterial();
  if (!material)
    return;
  auto shader = material->GetGLShader();
  if (!shader || !shader->Use())
    return;

  if (gl_buffers_dirty_ || !positions_normals_vbo_)
    UpdateGLBuffers(false);

  if (!vertex_count_ || !face_indices_count_ ||
      !positions_normals_vbo_ || !faces_ebo_)
    return;

  // enable depth test
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // set up uniforms: MVP matrices, lights, material
  shader->SetupUniforms(draw_data);

  // enable positions attribute and set pointer
  glBindBuffer(GL_ARRAY_BUFFER, positions_normals_vbo_);
  auto positions_attr_index = glGetAttribLocation(shader->GetProgramID(), "position");
  glVertexAttribPointer(positions_attr_index, 3, GL_FLOAT, GL_FALSE,
                        6 * sizeof(GLfloat), (void*)(0));
  glEnableVertexAttribArray(positions_attr_index);

  // enable normals attribute and set pointer
  auto normals_attr_index = glGetAttribLocation(shader->GetProgramID(), "normal");
  glVertexAttribPointer(normals_attr_index, 3, GL_FLOAT, GL_FALSE,
                        6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(normals_attr_index);

  // draw mesh
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ebo_);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(face_indices_count_),
                 GL_UNSIGNED_INT, nullptr);

  // check for gl errors
  CheckOpenGLError();
}



}  // namespace olio