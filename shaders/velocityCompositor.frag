#version 430

uniform sampler2DMS _colorTex;
uniform sampler2DMS _velocityTex;
uniform sampler2DMS _depthTex;

out vec3 fragColor;

void main()
{
	vec3 color = 
	(
	texelFetch(_colorTex,ivec2(gl_FragCoord.xy),0).rgb+
	texelFetch(_colorTex,ivec2(gl_FragCoord.xy),1).rgb+
	texelFetch(_colorTex,ivec2(gl_FragCoord.xy),2).rgb+
	texelFetch(_colorTex,ivec2(gl_FragCoord.xy),3).rgb
	)/4.0f;
	vec3 velocity = 
	(
	texelFetch(_velocityTex,ivec2(gl_FragCoord.xy),0).rgb+
	texelFetch(_velocityTex,ivec2(gl_FragCoord.xy),1).rgb+
	texelFetch(_velocityTex,ivec2(gl_FragCoord.xy),2).rgb+
	texelFetch(_velocityTex,ivec2(gl_FragCoord.xy),3).rgb
	)/4.0f;
	if(ivec3(velocity.xyz*255)==ivec3(127))
	{
		fragColor = color;
	}
	else
		fragColor = color*0.5+velocity*0.5;
}