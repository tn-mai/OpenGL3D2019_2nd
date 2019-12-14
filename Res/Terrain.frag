/**
* @file Terrain.frag
*/
#version 430

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inTBN[3];
layout(location=5) in vec3 inRawPosition;
layout(location=6) in vec3 inShadowPosition;

out vec4 fragColor;

uniform sampler2D texColorArray[4];
uniform sampler2D texNormalArray[3];
uniform sampler2DShadow texShadow;
uniform isamplerBuffer texPointLightIndex; // use texelFetch
uniform isamplerBuffer texSpotLightIndex; // use texelFetch

const ivec2 mapSize = ivec2(201, 201);

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

const float shininess = 2;
const float normFactor = (shininess + 2) * (1.0 / (2.0 * 3.1415926));

/**
* Terrain fragment shader.
*/
void main()
{
  vec4 ratio = texture(texColorArray[0], inTexCoord);
  float baseRatio = max(0, 1.0 - ratio.r - ratio.g);
  vec2 uv = inTexCoord * 10;
  fragColor.rgb = texture(texColorArray[1], uv).rgb * baseRatio;
  fragColor.rgb += texture(texColorArray[2], uv).rgb * ratio.r;
  fragColor.rgb += texture(texColorArray[3], uv).rgb * ratio.g;
  fragColor.a = 1;

  mat3 matTBN = mat3(normalize(inTBN[0]), normalize(inTBN[1]), normalize(inTBN[2]));
  vec3 normal = (texture(texNormalArray[0], uv).rgb * 2.0 - 1.0) * baseRatio;
  normal += (texture(texNormalArray[1], uv).rgb * 2.0 - 1.0) * ratio.r;
  normal += (texture(texNormalArray[2], uv).rgb * 2.0 - 1.0) * ratio.g;
  normal = normalize(matTBN * normal);

  vec3 lightColor = ambientLight.color.rgb;

  float shadow = texture(texShadow, inShadowPosition) * 0.75 + 0.25;
  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power * shadow;
  vec3 eyeVector = normalize(-vec3(0, 50, 25));
  lightColor += directionalLight.color.rgb * pow(max(dot(eyeVector, reflect(-directionalLight.direction.xyz, normal)), 0), shininess) * power * normFactor * shadow;

  int offset = int(inRawPosition.z) * mapSize.x + int(inRawPosition.x);
  ivec4 pointLightIndex = texelFetch(texPointLightIndex, offset);
  for (int i = 0; i < 4; ++i) {
    int id = pointLightIndex[i];
    if (id >= 0) {
      vec3 lightVector = pointLight[id].position.xyz - inPosition;
      vec3 lightDir = normalize(lightVector);
      float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
      float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
      lightColor += pointLight[id].color.rgb * cosTheta * intensity;
    }
  }

  ivec4 spotLightIndex = texelFetch(texSpotLightIndex, offset);
  for (int i = 0; i < 4; ++i) {
    int id = spotLightIndex[i];
    if (id >= 0) {
      vec3 lightVector = spotLight[id].posAndInnerCutOff.xyz - inPosition;
      vec3 lightDir = normalize(lightVector);
      float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
      float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
      float spotCosTheta = dot(lightDir, -spotLight[id].dirAndCutOff.xyz);
      float cutOff = smoothstep(spotLight[id].dirAndCutOff.w, spotLight[id].posAndInnerCutOff.w, spotCosTheta);
      lightColor += spotLight[id].color.rgb * cosTheta * intensity * cutOff;
    }
  }

  fragColor.rgb *= lightColor;
}