/**
* @file GaussianBlur9.frag
*/
#version 430

layout(location=0) in vec2 inTexCoord;
out vec4 fragColor;

uniform sampler2D texColor;
uniform vec2 blurDirection;

const float sigma = 1.0;
const float norm = 1.0 / (sqrt(2 * 3.14159265359) * sigma);
const int count = int(sigma * 3.0);

void main()
{
#if 0
  fragColor = texture2D(texColor, inTexCoord);
  for (int i = 1; i < count; ++i) {
    float coeff = exp(-float(i) * float(i) / (2.0 * sigma * sigma));
    fragColor += texture2D(texColor, inTexCoord - float(i) * blurDirection) * coeff;
    fragColor += texture2D(texColor, inTexCoord + float(i) * blurDirection) * coeff;
  }
  fragColor *= norm * 1.0;
#elif 1
  fragColor = texture2D(texColor, inTexCoord);
  float total = 1.0;
  for (int i = 1; i < 4; ++i) {
    float coeff = exp(-float(i) * float(i) * 0.5);
    fragColor += texture2D(texColor, inTexCoord - float(i) * blurDirection) * coeff;
    fragColor += texture2D(texColor, inTexCoord + float(i) * blurDirection) * coeff;
	total += coeff * 2.0;
  }
  fragColor /= total;
#else
  vec2 off1 = vec2(1.3846153846) * blurDirection;
  vec2 off2 = vec2(3.2307692308) * blurDirection;
  fragColor = texture2D(texColor, inTexCoord) * 0.2270270270;
  fragColor += texture2D(texColor, inTexCoord + off1) * 0.3162162162;
  fragColor += texture2D(texColor, inTexCoord - off1) * 0.3162162162;
  fragColor += texture2D(texColor, inTexCoord + off2) * 0.0702702703;
  fragColor += texture2D(texColor, inTexCoord - off2) * 0.0702702703;
#endif
}
