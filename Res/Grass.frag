/**
* @file Grass.frag
*/
#version 430

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inShadowPosition;

out vec4 fragColor;

uniform sampler2D texColor;
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

/**
* Grass fragment shader.
*/
void main()
{
  fragColor = texture(texColor, inTexCoord);
//  if (fragColor.a < 0.25) {
//    discard;
//  }

  vec3 normal = normalize(inNormal);
#if 0
  vec3 fdx = dFdx(gl_FragCoord.xyz);
  vec3 fdy = dFdy(gl_FragCoord.xyz);
  vec3 faceNormal = normalize(cross(fdx,fdy));
  if (dot (normal, faceNormal) < 0.0) {
    normal = -normal;
  }
#elif 0
  if (gl_FrontFacing == false) {
    normal = -normal;
  }
#endif

#if 0
  fragColor.rgb *= texture(texShadow, inShadowPosition) * 0.75 + 0.25;
#else
  vec3 lightColor = ambientLight.color.rgb;
  float shadow = texture(texShadow, inShadowPosition) * 0.75 + 0.25;
  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power * shadow;

  // ワールド座標から添え字を計算.
  int offset = int(inPosition.z) * mapSize.x + int(inPosition.x);

  // 添え字offsetを使って、ポイントライトのインデックスバッファからライト番号を取得.
  ivec4 pointLightIndex = texelFetch(texPointLightIndex, offset);
  for (int i = 0; i < 4; ++i) {
    int id = pointLightIndex[i];
    if (id < 0) { // 番号が0未満の場合、それ以上ライト番号は入っていない.
      break;
    }
    vec3 lightVector = pointLight[id].position.xyz - inPosition;
    vec3 lightDir = normalize(lightVector);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
    lightColor += pointLight[id].color.rgb * cosTheta * intensity;
  }

  // 添え字offsetを使って、スポットライトのインデックスバッファからライト番号を取得.
  ivec4 spotLightIndex = texelFetch(texSpotLightIndex, offset);
  for (int i = 0; i < 4; ++i) {
    int id = spotLightIndex[i];
    if (id < 0) { // 番号が0未満の場合、それ以上ライト番号は入っていない.
      break;
    }
    vec3 lightVector = spotLight[id].posAndInnerCutOff.xyz - inPosition;
    vec3 lightDir = normalize(lightVector);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
    float spotCosTheta = dot(lightDir, -spotLight[id].dirAndCutOff.xyz);
    float cutOff = smoothstep(spotLight[id].dirAndCutOff.w, spotLight[id].posAndInnerCutOff.w, spotCosTheta);
    lightColor += spotLight[id].color.rgb * cosTheta * intensity * cutOff;
  }

  fragColor.rgb *= lightColor;
#endif
}