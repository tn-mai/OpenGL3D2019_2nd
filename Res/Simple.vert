/**
* @file Simple.vert
*/
#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;

layout(location=0) out vec2 outTexCoord;

uniform mat4 matMVP;
uniform mat4 matModel;

/**
* Simple Vertex Shader.
*/
void main()
{
  outTexCoord = vTexCoord;
  gl_Position = matMVP * matModel * vec4(vPosition, 1.0);
}
