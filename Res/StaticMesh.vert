/**
* @file StaticMesh.vert
*/
#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;

layout(location=0) out vec3 outPosition;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outShadowPosition;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform mat4 matShadow;

/**
* StaticMesh vertex shader.
*/
void main()
{
  mat3 matNormal = transpose(inverse(mat3(matModel)));

  outTexCoord = vTexCoord;
  outNormal = normalize(matNormal * vNormal);
  outPosition = vec3(matModel * vec4(vPosition, 1.0));
  outShadowPosition = vec3(matShadow * vec4(outPosition, 1.0)) * 0.5 + vec3(0.5, 0.5, 0.5 - 0.0005);
  gl_Position = matMVP * (matModel * vec4(vPosition, 1.0));
}