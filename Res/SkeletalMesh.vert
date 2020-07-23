/**
* @file SkeletalMesh.vert
*/
#version 410

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;
layout(location=3) in vec4 vWeights;
layout(location=4) in vec4 vJoints;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outPosition;
layout(location=4) out vec3 outShadowPosition;

// global
uniform mat4x4 matMVP;
uniform mat4 matShadow;

// per mesh
layout(std140) uniform MeshMatrixUniformData
{
  vec4 color;
  mat3x4 matModel[8]; // it must transpose.
  mat3x4 matBones[256]; // it must transpose.
} vd;

// per primitive
uniform vec4 materialColor;
uniform int meshIndex;

// Lights.

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
* Vertex shader for SkeletalMesh.
*/
void main()
{
  outColor = materialColor * vd.color;
  outTexCoord = vTexCoord;
  mat3x4 matSkinTmp =
    vd.matBones[int(vJoints.x)] * vWeights.x +
    vd.matBones[int(vJoints.y)] * vWeights.y +
    vd.matBones[int(vJoints.z)] * vWeights.z +
    vd.matBones[int(vJoints.w)] * vWeights.w;
  mat4 matSkin = mat4(transpose(matSkinTmp));
  matSkin[3][3] = dot(vWeights, vec4(1)); // ウェイトが正規化されていない場合の対策([3][3]が1.0になるとは限らない).
  mat4 matModel = mat4(transpose(vd.matModel[0/*meshIndex*/])) * matSkin;
  mat3 matNormal = transpose(inverse(mat3(matModel)));
  outNormal = matNormal * vNormal;
  outPosition = vec3(matModel * vec4(vPosition, 1.0));

  // shadow bias(normal offset).
  const float meterPerShadowMapPixel = 100.0 / 4096.0;
  float cosTheta = 1.0 - clamp(dot(outNormal, -directionalLight.direction.xyz), 0.0, 1.0);
  vec3 normalBias= outNormal * (2.0 * meterPerShadowMapPixel * cosTheta);

  // shadow bias(z offset).
  float zBias = meterPerShadowMapPixel * 0.05;

  outShadowPosition = vec3(matShadow * matModel * vec4(vPosition + normalBias, 1.0)) * 0.5 + vec3(0.5);
  outShadowPosition.z -= zBias;

  gl_Position = matMVP * matModel * vec4(vPosition, 1.0);
}