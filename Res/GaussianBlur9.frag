/**
* @file GaussianBlur9.frag
*/
#version 430

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D texColor;
uniform vec2 blurDirection;

void main()
{
  vec2 off1 = vec2(1.3846153846) * blurDirection;
  vec2 off2 = vec2(3.2307692308) * blurDirection;
  fragColor = texture2D(texColor, inTexCoord) * 0.2270270270;
  fragColor += texture2D(texColor, inTexCoord + off1) * 0.3162162162;
  fragColor += texture2D(texColor, inTexCoord - off1) * 0.3162162162;
  fragColor += texture2D(texColor, inTexCoord + off2) * 0.0702702703;
  fragColor += texture2D(texColor, inTexCoord - off2) * 0.0702702703;
}
