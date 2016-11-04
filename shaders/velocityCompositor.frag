#version 430

uniform sampler2D _colorTex;
uniform sampler2D _velocityTex;
uniform sampler2D _depthTex;

out vec3 fragColor;

void main()
{
	vec3 color = texelFetch(_colorTex,ivec2(gl_FragCoord.xy),0).rgb;
	vec3 velocity = texelFetch(_velocityTex,ivec2(gl_FragCoord.xy),0).rgb;
	if(ivec3(velocity.xyz*255)==ivec3(127))
	{
		fragColor = color;
	}
	else
		fragColor = color*0.5+velocity*0.5;
}