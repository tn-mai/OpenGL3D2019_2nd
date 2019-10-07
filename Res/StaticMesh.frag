/**
* @file StaticMesh.frag
*/
#version 430

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inNormal;

out vec4 fragColor;

uniform sampler2D texColor;

uniform int pointLightCount;
uniform int pointLightIndex[8];

uniform int spotLightCount;
uniform int spotLightIndex[8];

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
* StaticMesh fragment shader.
*/
void main()
{
  vec3 normal = normalize(inNormal);

  vec3 lightColor = ambientLight.color.rgb;

  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power;

  for (int i = 0; i < pointLightCount; ++i) {
    int id = pointLightIndex[i];
    vec3 lightVector = pointLight[id].position.xyz - inPosition;
    vec3 lightDir = normalize(lightVector);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
    lightColor += pointLight[id].color.rgb * cosTheta * intensity;
  }

  for (int i = 0; i < spotLightCount; ++i) {
    int id = spotLightIndex[i];
    vec3 lightVector = spotLight[id].posAndInnerCutOff.xyz - inPosition;
    vec3 lightDir = normalize(lightVector);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
    float spotCosTheta = dot(lightDir, -spotLight[id].dirAndCutOff.xyz);
    float cutOff = smoothstep(spotLight[id].dirAndCutOff.w, spotLight[id].posAndInnerCutOff.w, spotCosTheta);
    lightColor += spotLight[id].color.rgb * cosTheta * intensity * cutOff;
  }

  fragColor = texture(texColor, inTexCoord);
  fragColor.rgb *= lightColor;
}