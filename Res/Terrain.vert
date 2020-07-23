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
layout(location=6) out vec3 outShadowPosition;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform mat4 matShadow;

struct AmbientLight
{
  vec4 color;
};

struct DirectionalLight
{
  vec4 color;
  vec4 direction;
};

struct PointLight
{
  vec4 color;
  vec4 position;
};

struct SpotLight
{
  vec4 color;
  vec4 dirAndCutOff;
  vec4 posAndInnerCutOff;
};

layout(std140) uniform LightUniformBlock
{
  AmbientLight ambientLight;
  DirectionalLight directionalLight;
  PointLight pointLight[100];
  SpotLight spotLight[100];
};
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

  // shadow bias(normal offset).
  const float meterPerShadowMapPixel = 100.0 / 4096.0;
  float cosTheta = 1.0 - clamp(dot(n, -directionalLight.direction.xyz), 0.0, 1.0);
  vec3 normalBias= n * (2.0 * meterPerShadowMapPixel * cosTheta);

  // shadow bias(z offset).
  float zBias = meterPerShadowMapPixel * 0.01;

  outShadowPosition = vec3(matShadow * vec4(outPosition + normalBias, 1.0)) * 0.5 + 0.5;
  outShadowPosition.z -= zBias;

  outRawPosition = vPosition;
  gl_Position = matMVP * (matModel * vec4(vPosition, 1.0));
}
