/**
* @file SkeletalMesh.frag
*/
#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;
layout(location=4) in vec3 inShadowPosition;

out vec4 fragColor;

uniform sampler2D texColor;
uniform sampler2DShadow texShadow;

/**
* Fragment shader for SkeletalMesh.
*/
void main()
{
  fragColor = inColor * texture(texColor, inTexCoord);
  if (fragColor.a < 0.25) {
    discard;
  }

  float shadow = texture(texShadow, inShadowPosition) * 0.75 + 0.25;
  vec3 ambientColor = vec3(0.1, 0.1, 0.2);
  vec3 vLight = normalize(vec3(1, -1.5, -1));
  float power = max(dot(normalize(inNormal), -vLight), 0.0);
  fragColor.rgb *= vec3(1.0, 0.95, 0.91) * power * shadow + ambientColor;
}