#ifndef __TextureFont_h__
#define __TextureFont_h__
#include "ft2build.h"
#include FT_FREETYPE_H

#include "Texture.h"
/*
*/
class TextureFont
{
public:
    TextureFont(unsigned int width, unsigned int height);
    ~TextureFont();
    void paintGlyph(FT_GlyphSlot glyph, unsigned int penX, unsigned int penY);
private:
    unsigned char **tex;
    unsigned int width;
    unsigned int height;
};

#endif //ifndef __Texture_h__