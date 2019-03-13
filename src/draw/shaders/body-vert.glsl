#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 2) in vec3 vCol;   // Usually for normals

uniform mat4 mvpMx;

out vec4 vertexColour;

void main() {
  gl_Position = mvpMx * vec4(vPos, 1.0);
  vertexColour = vec4(vCol, 1.0);
}
