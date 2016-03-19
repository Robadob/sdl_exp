 #version 430

in vec2 texcoordsV;

out vec3 fragColor;

uniform sampler2D _texture;

void main(){
  _coords = gl_TexCoords[0].xy;
//vec3(0.2,0.6,0.8);//
  fragColor = texture( _texture, texcoordsV ).rgb;
}