#version 430

uniform sampler2D _colorTex;
uniform sampler2D _velocityTex;
uniform sampler2D _depthTex;

out vec3 fragColor;

void main()
{
	fragColor = texelFetch(_velocityTex, gl_FragCoord.xy,0);
}