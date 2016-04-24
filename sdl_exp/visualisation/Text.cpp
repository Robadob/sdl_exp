#include "Text.h"
#include <vector>

//http://www.freetype.org/freetype2/docs/tutorial/step1.html
//http://www.freetype.org/freetype2/docs/tutorial/step2.html
Text::Text(char *string, char const *fontFile, unsigned int faceIndex)
    : library()
    , font()
    , string(string)
    , fontHeight(20)
    , wrapDistance(800)
{
    if (!fontFile)
        fontFile = FONT_ARIAL;
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst initialising FreeType: %i\n", error);
        return;
    }
    error = FT_New_Face(library,
        fontFile,
        faceIndex,
        &font);
    if (error == FT_Err_Unknown_File_Format)
    {
        fprintf(stderr, "The font file %s is of an unsupport format, default to Arial\n", fontFile);
        fontFile = FONT_ARIAL;
        error = FT_New_Face(library,
            fontFile,
            0,
            &font);
    }
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst loading font file %s: %i\n", fontFile, error);
        return;
    }
    error = FT_Set_Pixel_Sizes(
        font,   /* handle to face object */
        0,      /* pixel_width           */
        fontHeight);/* pixel_height          */
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst setting font size: %i\n", error);
        return;
    }
    printf("Font %s was loaded successfully, the file contains %i faces.\n", fontFile, font->num_faces);
    recomputeTex();
}
Text::~Text()
{
    if (font)
        FT_Done_Face(font);
    if (library)
        FT_Done_FreeType(library);
}
void Text::setFontHeight(unsigned int pixels)
{
    if (!font)
    {
        fprintf(stderr, "Unable to set font height, font has not been loaded.\n");
        return;
    }
    fontHeight = pixels;
    /*//Untested fixed size font handling
    int closest = 0;
    int best_index = -1;
    for (int i = 0; i < font->num_fixed_sizes; i++)
    {
        if (font->available_sizes[i].height <= pixels && font->available_sizes[i].height > closest)
        {
            best_index = i;
            closest = font->available_sizes[i].height;
        }
    }
    */
    FT_Error error = FT_Set_Pixel_Sizes(
        font,   /* handle to face object */
        0,      /* pixel_width           */
        fontHeight);/* pixel_height          */
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst setting font size: %i\n",  error);
        return;
    }
    recomputeTex();
}
void Text::recomputeTex()
{
    FT_Error error;
    //Measure tex width and height
    unsigned int texWidth = 0;
    unsigned int texHeight = fontHeight;
    unsigned int lastSpace = 0;
    unsigned int currentLineWidth = 0;
    std::vector<int> lineStarts = {0};
    //Iterate chars, measuring space required and wrapping lines so we can allocate tex
    for (unsigned int i = 0; i < stringLen; i++)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(font, string[i]);
        //Defaults to unicode charmap
        error = FT_Load_Char(font, string[i], FT_LOAD_RENDER);
        if (error)
        {
            fprintf(stderr, "An unexpected error occured whilst loading glyph '%c': %i\n", string[i], error);
            continue;
        }
        //Remember last space so we know for wrapping
        if (string[i] == ' ')
        {
            lastSpace = currentLineWidth;
        }
        //handle carraige return and newline
        if (string[i] == '\r')
        {
            texWidth = currentLineWidth>texWidth ? currentLineWidth : texWidth;
            currentLineWidth = 0;
            continue;
        }
        if (string[i] == '\n')
        {
            texHeight += fontHeight;
            texWidth = currentLineWidth>texWidth ? currentLineWidth : texWidth;
            currentLineWidth = 0;
            lineStarts.push_back(i + 1);
            continue;
        }
        //Add char width to current (if not a space on a newline)
        if (!(string[i] == ' '&&currentLineWidth==0))
            currentLineWidth += font->glyph->advance.x >> 6;
        //If this exceeds wrap dist, revert to currentLine and create newline
        if (currentLineWidth>wrapDistance&&lastSpace>0)
        {
            texWidth = lastSpace>texWidth ? lastSpace : texWidth;
            currentLineWidth -= lastSpace;
            lastSpace = 0;
            texHeight += fontHeight;
            lineStarts.push_back(i + 1);
        }
    }
    //Allocate tex
    TextureFont tex(texWidth, texHeight);

    int pen_x = 0;
    int pen_y = texHeight;
    lastSpace = 0;
    currentLineWidth = 0;
    unsigned int nextLineStart = 0;
    for (unsigned int i = 0; i < stringLen;i++)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(font, string[i]);
        //Defaults to unicode charmap
        error = FT_Load_Char(font, string[i], FT_LOAD_RENDER);
        if (error)
        {
            fprintf(stderr, "An unexpected error occured whilst loading glyph '%c': %i\n", string[i], error);
            continue;
        }
        //Remember last space so we know for wrapping
        if (string[i] == ' ')
        {
            lastSpace = currentLineWidth;
        }
        //handle carraige return and newline
        if (string[i] == '\r')
        {
            texWidth = currentLineWidth>texWidth ? currentLineWidth : texWidth;
            currentLineWidth = 0;
            continue;
        }
        //If we wrap at this char, update pen position
        if (i<lineStarts.size() && lineStarts[nextLineStart] == i)
        {
            pen_x = 0;
            pen_y -= fontHeight;
        }
        //Add glyph to tex
        tex.paintGlyph(font->glyph, pen_x, pen_y);

        /* increment pen position */
        if (!(string[i] == ' '&&pen_x == 0))
            pen_x += font->glyph->advance.x >> 6;

        /*Once you have a bitmapped glyph image, you can access it directly through glyph->bitmap (a simple descriptor for bitmaps or pixmaps), 
        and position it through glyph->bitmap_left and glyph->bitmap_top. 
        For optimal rendering on a screen the bitmap should be used as an alpha channel in linear blending with gamma correction.

        Note that bitmap_left is the horizontal distance from the current pen position to the leftmost border of the glyph bitmap, 
        while bitmap_top is the vertical distance from the pen position (on the baseline) to the topmost border of the glyph bitmap. 
        It is positive to indicate an upwards distance.
        */
    }
}
void Text::setStringLen()
{
    stringLen = 0;
    while (string[stringLen++] != '\0');
}