/**
* @file Terrain.vert
*/
#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;

layout(location=0) out vec3 outPosition;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outTBN[3];
layout(location=5) out vec3 outRawPosition;

uniform mat4 matMVP;
uniform mat4 matModel;

/**
* Terrain vertex shader.
*/
void main()
{
  mat3 matNormal = transpose(inverse(mat3(matModel)));
  vec3 n = matNormal * vNormal;
  vec3 b = matNormal * vec3(0.0, 0.0, -1.0);
  vec3 t = normalize(cross(b, n));
  b = normalize(cross(t, n));
  outTBN[0] = t;	
  outTBN[1] = b;
  outTBN[2] = n;

  outTexCoord = vTexCoord;

  outPosition = vec3(matModel * vec4(vPosition, 1.0));
  outRawPosition = vPosition;
  gl_Position = matMVP * (matModel * vec4(vPosition, 1.0));
}
