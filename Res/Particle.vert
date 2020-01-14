/**
* @file Particle.vert
*/
#version 430

layout(location=0) in vec3 vCenter;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec2 vOffset;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;

uniform mat4x4 matMVP;
uniform mat3 matInverseViewRotation;

/**
* Sprite vertex shader.
*/
void main()
{
  outColor = vColor;
  outTexCoord = vTexCoord;
  vec3 offset = matInverseViewRotation * vec3(vOffset, 0.0);
  gl_Position = matMVP * vec4(vCenter + offset, 1.0);
}