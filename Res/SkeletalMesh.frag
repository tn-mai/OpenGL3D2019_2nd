/**
* @file SkeletalMesh.frag
*/
#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;

out vec4 fragColor;

uniform sampler2D texColor;

/**
* Fragment shader for SkeletalMesh.
*/
void main()
{
  fragColor = inColor * texture(texColor, inTexCoord);
  if (fragColor.a < 0.1) {
    discard;
  }
  vec3 ambientColor = vec3(0.1, 0.1, 0.2);
  vec3 vLight = normalize(vec3(1, -1.5, -1));
  float power = max(dot(normalize(inNormal), -vLight), 0.0);
  fragColor.rgb = fragColor.rgb * power + ambientColor;
}