#include "draw/Draw.hpp"

#include <string>


// Output all the things
GLT::ShaderProgram BuildBodyShader(void) {
  GLT::Shader vs(GL_VERTEX_SHADER, _MPIGRAV_BODY_VERT_SHADER_PATH);
  GLT::Shader fs(GL_FRAGMENT_SHADER, _MPIGRAV_BODY_FRAG_SHADER_PATH);
  return GLT::ShaderProgram({vs, fs});
}


// Generate mesh from list of bodies
GLT::Mesh MakeMeshFromBodyList(std::vector<Body> const& bodies) {
  std::vector<GLT::vertex_t> v(bodies.size());
  for(unsigned i = 0; i < bodies.size(); i++) {
    v[i].position.x = bodies[i].r.x;      //
    v[i].position.y = bodies[i].r.y;      // Position
    v[i].position.z = bodies[i].r.z;      //
    v[i].normal = glm::vec3(1, 1, 1);     // Colour (just white for now)
  }
  return GLT::Mesh(v);
}


// Override the mesh draw routine
void GLT::Mesh::Draw(Camera& camera, ShaderProgram& shader, glm::mat4& m) {
  glm::mat4 mvp = camera.GetProjMat() * camera.GetViewMat() * m;
  shader.GetUniform("mvpMx").SetFMat4(mvp);
  this->vertexBuffer.Bind();
  glDrawElements(
    GL_POINTS,
    this->vertexBuffer.GetIndexBufferLength(),
    GL_UNSIGNED_INT, 0);
  this->vertexBuffer.Unbind();
}
