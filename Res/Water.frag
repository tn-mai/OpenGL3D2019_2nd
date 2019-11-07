/**
* @file Water.frag
*/
#version 430

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inTBN[3];
layout(location=5) in vec3 inRawPosition;

out vec4 fragColor;

uniform sampler2D texColorArray[4];
uniform sampler2D texNormalArray[3];
uniform isamplerBuffer texPointLightIndex; // use texelFetch
uniform isamplerBuffer texSpotLightIndex; // use texelFetch

uniform vec3 cameraPosition;
uniform samplerCube texCubeMap;
uniform float time;

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

const float iorVacuum = 1.0;
const float iorAir = 1.000293;
const float iorWater = 1.333;
const float iorSoil = 2.5;
const float iorRock = 3.5;
const float iorPlant = iorWater;

const float eta = iorAir / iorWater;
const float f = (1.0 - eta) * (1.0 - eta) / ((1.0 + eta) * (1.0 + eta));
float GetFresnelFactor(vec3 v, vec3 n) {
  return f + (1.0 - f) * pow(1.0 - dot(v, n), 5.0);
}
#define FresnelSchlick(F0, dotEH) (F0 + (1.0 - F0) * exp2((-5.55473 * dotEH - 6.98316) * dotEH))

/**
* Terrain fragment shader.
*/
void main()
{
  fragColor = vec4(0.15, 0.20, 0.3, 0.6);

  mat3 matTBN = mat3(normalize(inTBN[0]), normalize(inTBN[1]), normalize(inTBN[2]));
  vec4 uv = inTexCoord.xyxy * vec4(11.0, 11.0, 5.0, 5.0);
  vec4 scroll = vec4(-0.01, -0.01, 0.005, 0.005) * time;
  vec3 normalS = texture(texNormalArray[0], uv.xy + scroll.xy).rgb * 2.0 - 1.0;
  vec3 normalL = texture(texNormalArray[0], uv.zw + scroll.zw).rgb * 2.0 - 1.0;
  vec3 normal = (normalS * 0.5 + normalL) * vec3(0.1, 0.1, 1.0);
  normal = normalize(matTBN * normal);

  vec3 lightColor = ambientLight.color.rgb;

  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power;

  vec3 eyeVector = normalize(-vec3(0, 50, 25));
  lightColor += directionalLight.color.rgb * pow(max(dot(eyeVector, reflect(-directionalLight.direction.xyz, normal)), 0), shininess) * power * normFactor;

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

  vec3 cameraVector = normalize(cameraPosition - inPosition);
  vec3 reflectionVector = 2.0 * max(dot(cameraVector, normal), 0.0) * normal - cameraVector;
  vec3 environmentColor = texture(texCubeMap, reflectionVector).rgb;

  float brightness = 5.0; // 環境マップの明るさ補正値. 発見的に晴れた日中ぽく見える値を選んだ.
  float opacity = 0.6; // 水の不透明度.
  vec3 yuv = mat3(
    0.299,-0.169, 0.500,
    0.587,-0.331,-0.419,
    0.114, 0.500,-0.081) * environmentColor;
  yuv.r *= GetFresnelFactor(cameraVector, normal) * brightness;
  fragColor.a = opacity + yuv.r;

  // 反射光は水中からの出射光に加算されるが、α合成後の明るさは「反射光の明るさ*α」に減じてしまう.
  // 明るさをアルファで除算しておくと「反射光の明るさ*α/α」となるので意図した明るさで表示される.
  // 不透明度が小さすぎる場合に白く飽和してしまうが、水の不透明度をある程度の大きさ以上に設定すれば回避できる.
  yuv.r /= fragColor.a;

  fragColor.rgb += mat3(
    1.000, 1.000, 1.000,
    0.000,-0.344, 1.772,
    1.402,-0.714, 0.000) * yuv;
}
