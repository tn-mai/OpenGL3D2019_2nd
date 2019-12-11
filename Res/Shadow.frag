/**
* @file Shadow.frag
*/
#version 430

layout(location = 0) out float fragDepth;

uniform sampler2D texColor;

void main()
{
  fragDepth = gl_FragCoord.z;
}
