#version 430

in vec3 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;
uniform vec4 _col;
uniform vec4 _backCol;
//Blends round corners onto the tex
//Without passing window dims, we cant workout which corners are touching top & right bounds
//Hence disabled
float roundCorner()
{
    int rad = 8;
    int radsq = rad*rad;
    int radm1sq = radsq-((rad-1)*(rad-1));
    //Get dimensions of the texture
    ivec2 texDim = textureSize(_texture, 0);
    //Calc how close we are to each bound of tex
    int left = int(texCoords.x*texDim.x);
    int bottom = int(texCoords.y*texDim.y);
    int right = texDim.x - left;
    int top = texDim.y - bottom;
    //No idea, why, but these incrments are required
    left++;
    bottom++;
    if(right<=rad&&bottom<=rad)
    {
        int b = rad-right;
        int r = rad-bottom;
        int tot = b*b+r*r;//Length^2
        return ((radsq-tot)/radm1sq);//(desired length^2-length^2)/(desired length^2-(desired length-1)^2)
    }
    else if(right<=rad&&top<=rad)
    {
        int b = rad-right;
        int r = rad-top;
        int tot = b*b+r*r;//Length^2
        return ((radsq-tot)/radm1sq);//(desired length^2-length^2)/(desired length^2-(desired length-1)^2)
    }
    else if(left<=rad&&bottom<=rad)
    {
        int b = rad-left;
        int r = rad-bottom;
        int tot = b*b+r*r;//Length^2
        return ((radsq-tot)/radm1sq);//(desired length^2-length^2)/(desired length^2-(desired length-1)^2)
    }
    else if(left<=rad&&top<=rad)
    {
        int b = rad-left;
        int r = rad-top;
        int tot = b*b+r*r;//Length^2
        return ((radsq-tot)/radm1sq);//(desired length^2-length^2)/(desired length^2-(desired length-1)^2)
    }
    else
        return 1.0f;
}
void main()
{
  float tex = texture(_texture, vec2(texCoords.x,-texCoords.y)).r;
  //Interpolate the forecolour according to the tex
  vec4 foreground = _col*tex;
  //Interpolate the background according to inverse tex
  vec4 background = _backCol*(1-tex);
  //Blend the colors according to their own alpha channels
  vec3 blend = (foreground.xyz*foreground.w) + (background.xyz*background.w);
  //Output the blended rgb values with a summed alpha channel
  fragColor = vec4(blend, (foreground.w+background.w));
}