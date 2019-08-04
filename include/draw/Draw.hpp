#ifndef _MPIGRAV_DRAW_INCLUDED
#define _MPIGRAV_DRAW_INCLUDED

#include <GLT/Mesh.hpp>
#include <GLT/GL/Shader.hpp>

#include "Body.hpp"


// Shader paths
#define _MPIGRAV_BODY_VERT_SHADER_PATH "shaders/body-vert.glsl"
#define _MPIGRAV_BODY_FRAG_SHADER_PATH "shaders/body-frag.glsl"


// Shader construction
GLT::ShaderProgram BuildBodyShader(void);


// Generate a mesh from a list of bodies
GLT::Mesh MakeMeshFromBodyList(std::vector<Body> const& bodies);


#endif // _MPIGRAV_DRAW_INCLUDED
