/**
* @file Grass.vert
*/
#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;
layout(location=3) in ivec4 instanceData; // x=�O���b�hX���W, y=�O���b�hY���W, z=����, w=(���g�p)

layout(location=0) out vec3 outPosition;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outShadowPosition;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform mat4 matShadow;
uniform float time;

uniform sampler2D texHeightMap;
uniform sampler2D texGrassHeightMap;
uniform usamplerBuffer texInstanceData;

const ivec2 mapSize = ivec2(201, 201);
const vec2 invMapSize = vec2(1) / vec2(mapSize);
const float heightScale = 50;
const float baseLevel = 0.5;

/**
* �n�ʂ̍������擾����.
*
* @param pos �������擾������W.
*
* @return ���Wpos�̒n�ʂ̍���.
*/
float Height(vec3 pos)
{
  // �擾����ʒu���}�b�v�T�C�Y�𒴂��Ȃ��悤�ɂ���.
  // iposMin: 1x1m�u���b�N�̍������W
  // iposMax: 1x1m�u���b�N�̉E����W
  ivec2 iposMin = ivec2(pos.xz);
  iposMin.y = (mapSize.y - 2) - iposMin.y;
  iposMin = max(ivec2(0), iposMin);
  ivec2 iposMax = min(mapSize - 1, iposMin + ivec2(1));

  // ���㒸�_����̑��΍��W���v�Z.
  vec2 offset = fract(pos.xz);

  // 4�_�̍���������Wpos�̍������v�Z.
  // h0 -- h1
  // |  /  |
  // h2 -- h3
  float h1 = texelFetch(texHeightMap, iposMax, 0).r;
  float h2 = texelFetch(texHeightMap, iposMin, 0).r;
  float height;
  if (offset.x + offset.y < 1.0) {
    float h0 = texelFetch(texHeightMap, ivec2(iposMin.x, iposMax.y), 0).r;
    height = h0 + (h1 - h0) * offset.x + (h2 - h0) * offset.y;
  } else {
    float h3 = texelFetch(texHeightMap, ivec2(iposMax.x, iposMin.y), 0).r;
    height = h3 + (h2 - h3) * (1.0 - offset.x) + (h1 - h3) * (1.0 - offset.y);
  }
  return (height - baseLevel) * heightScale;
}

/**
* Grass vertex shader.
*/
void main()
{
  /*
  ���C���X�^���X�ɕK�v�ȃf�[�^.
  - �O���b�hX, Y���W.
  - ����.
  - Y����]�p��COS, SIN(�����_�����p�����[�^)
  ��]�╗�ɂ��h��̃����_�����́A"�O���b�h���W"����Ƃ��������Ōv�Z������@������.
  */
  uvec4 instanceData = texelFetch(texInstanceData, gl_InstanceID);
  float x = float(instanceData.x);
  float z = float(instanceData.y);

  outTexCoord = vTexCoord;

#if 0
  vec4 pos = vec4(vPosition + vec3(x, 0, z) + vec3(0.5, 0, 0.5), 1);
  vec2 invMapSize = vec2(1) / vec2(mapSize);
  vec2 tcGrassHeightMap = vec2(pos.x, float(mapSize.y) - pos.z) * invMapSize;
  float grassScale = texture(texGrassHeightMap, tcGrassHeightMap).g;
  pos.y *= grassScale;
  pos.y += Height(pos.xyz);
  vec3 normal = vNormal;
#else
  float rad = fract(z * 0.37 + x * 0.09) * 3.1415926 * 2;
  float cosT = cos(rad);
  float sinT = sin(rad);
  vec4 pos = vec4(vPosition, 1);
  pos.x = vPosition.x * cosT + vPosition.z * sinT;
  pos.z = vPosition.x * -sinT + vPosition.z * cosT;
  pos.xz *= 1.25;
  pos.xz += vec2(x, z) + vec2(0.5);
  pos.y = vPosition.y * (texture(texGrassHeightMap, vec2(pos.x, float(mapSize.y) - pos.z) * invMapSize).g + 0.1);
  pos.w = 1;
  pos.x += abs(fract(time * (0.7 + rad * 0.1)) - 0.5) * 0.3 * pos.y;
  pos.y += Height(pos.xyz);

  vec3 normal;
  normal.x = vNormal.x * cosT + vNormal.z * sinT;
  normal.y = vNormal.y;
  normal.z = vNormal.x * -sinT + vNormal.z * cosT;
#endif

  mat3 matNormal = transpose(inverse(mat3(matModel)));
  outNormal = normalize(matNormal * normal);

  pos = matModel * pos;
  outPosition = pos.xyz;
  outShadowPosition = vec3(matShadow * vec4(outPosition, 1.0)) * 0.5 + vec3(0.5, 0.5, 0.5 - 0.0005);
  gl_Position = matMVP * pos;
}