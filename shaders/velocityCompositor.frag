#version 430

uniform sampler2D _colorTex;
uniform sampler2D _velocityTex;
uniform sampler2D _depthTex;

out vec3 fragColor;

void main()
{
	fragColor = vec3(1,0,0);//texelFetch(_velocityTex, ivec2(gl_FragCoord.xy),0).rgb;
}