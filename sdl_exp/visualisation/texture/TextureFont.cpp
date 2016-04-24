#include "TextureFont.h"


TextureFont::TextureFont(unsigned int width, unsigned int height)
    : width(width)
    , height(height)
{
    tex = (unsigned char**)malloc(sizeof(char*)*height);
    tex[0] = (unsigned char*)malloc(sizeof(char)*width*height);
    memset(tex[0], 0, sizeof(char)*width*height);
    for (int i = 1; i < height;i++)
    {
        tex[i] = tex[i*width];
    }
}
TextureFont::~TextureFont()
{
    free(tex[0]);
    free(tex);
}
void TextureFont::paintGlyph(FT_GlyphSlot glyph, unsigned int penX, unsigned int penY)
{
    for (int y = 0; y<glyph->bitmap.rows; y++)
    {
        //src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph->bitmap.buffer + y*glyph->bitmap.pitch;
        //dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        //copy entire row
        for (int x = 0; x<glyph->bitmap.pitch; x++)
        {
            dst_ptr[x] = src_ptr[x];
        }
    }
}