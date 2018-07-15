#version 430

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D t_color;
uniform sampler2D t_brightness;

void main()
{
    //Constants, these could be made uniform
    const float GAMMA = 2.2f;
    const float EXPOSURE = 1.0f;
    //Additive blend
    vec3 color = texture(t_color, texCoords).rgb + texture(t_brightness, texCoords).rgb;      
    //Tone mapping (via exposure tone mapping)
    color = vec3(1.0) - exp(-color * EXPOSURE);
    //Gamma Correction
    color = pow(color, vec3(1.0 / GAMMA));
    //Output
    fragColor = vec4(color, 1.0);
}