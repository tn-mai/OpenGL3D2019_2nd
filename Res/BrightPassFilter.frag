/**
* @file BrightPassFilter.frag
*/
#version 430

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D texColor;

void main()
{
  fragColor = texture(texColor, inTexCoord);
  fragColor.rgb = mat3(
    0.299,-0.169, 0.500,
    0.587,-0.331,-0.419,
    0.114, 0.500,-0.081) * fragColor.rgb;
  fragColor.r = fragColor.r - 1.0;
  fragColor.rgb = mat3(
    1.000, 1.000, 1.000,
    0.000,-0.344, 1.772,
    1.402,-0.714, 0.000) * fragColor.rgb;
  fragColor.rgb = max(fragColor.rgb, 0.0);
}