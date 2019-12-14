/**
* @file ShadowWithTexture.frag
*/
#version 430

layout(location = 0) out float fragDepth;

void main()
{
  fragDepth = gl_FragCoord.z;
}
