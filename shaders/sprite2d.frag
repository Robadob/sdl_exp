#version 430

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
void main()
{
	//Render depth as HSV, giving us better contrast
	float depthColor = texture(_texture, vec2(texCoords.x,texCoords.y)).r;
	depthColor = clamp(depthColor,0.0f,1.0f);
	fragColor = vec4(hsv2rgb(vec3(depthColor,1,1)),1);
}