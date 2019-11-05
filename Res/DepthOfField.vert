/**
* @file DepthOfField.vert
*/
#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;

layout(location=0) out vec2 outTexCoord;

/**
* DepthOfField Vertex Shader.
*/
void main()
{
  outTexCoord = vTexCoord;
  gl_Position = vec4(vPosition, 1.0);
}
