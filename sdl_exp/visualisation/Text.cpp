#include "Text.h"
#include <vector>
const char* Text::FONT_ARIAL = "C:/Windows/Fonts/Arial.ttf";
//http://www.freetype.org/freetype2/docs/tutorial/step1.html
//http://www.freetype.org/freetype2/docs/tutorial/step2.html
Text::Text(char *string, unsigned int fontHeight, char const *fontFile, unsigned int faceIndex)
	: Overlay(std::make_shared<Shaders>(Stock::Shaders::TEXT))
	, library()
    , font()
    , string(string)
    , fontHeight(fontHeight)
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

void Text::reload()
{
	recomputeTex();
}


void Text::recomputeTex()
{
	setStringLen();
    FT_Error error;
    //Measure tex width and height
    unsigned int texWidth = 0;
    unsigned int texHeight = fontHeight*2;
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
	//Consider line if no wrap was applied
	texWidth = currentLineWidth>texWidth ? currentLineWidth : texWidth;
    //Allocate tex/Deallocate old tex
	tex = std::make_unique<TextureString>(texWidth, texHeight);

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
        //handle carriage return and newline
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
        tex->paintGlyph(font->glyph, pen_x, pen_y);

        /* increment pen position */
        if (!(string[i] == ' '&&pen_x == 0))
            pen_x += font->glyph->advance.x >> 6;
    }
	//link tex to shader
	tex->updateTex(getShaders());
	//Set width
	setDimensions(texWidth, texHeight);
}
void Text::setStringLen()
{
    stringLen = 0;
    while (string[stringLen++] != '\0');
    stringLen--;
}

Text::TextureString::TextureString(unsigned int width, unsigned int height)
	: Texture(GL_TEXTURE_2D, "")//_texture as default
	, texture(0)
	, width(width)
	, height(height)
{
	texture = (unsigned char**)malloc(sizeof(char*)*height);
	texture[0] = (unsigned char*)malloc(sizeof(char)*width*height);
	memset(texture[0], 0, sizeof(char)*width*height);
	for (unsigned int i = 1; i < height; i++)
	{
		texture[i] = texture[i - 1] + width;
	}
}
void Text::TextureString::updateTex(std::shared_ptr<Shaders> shaders)
{
	GL_CALL(glBindTexture(texType, texName));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
	GL_CALL(glTexImage2D(texType, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture[0]));
	GL_CALL(glBindTexture(texType, 0));
	bindToShader(shaders.get(), 0);
}
Text::TextureString::~TextureString()
{
	free(texture[0]);
	free(texture);
}
void Text::TextureString::paintGlyph(FT_GlyphSlot glyph, unsigned int penX, unsigned int penY)
{
    penY -= glyph->bitmap_top;
    //penX -= glyph->bitmap_left;
	for (unsigned int y = 0; y<glyph->bitmap.rows; y++)
	{
		//src ptr maps to the start of the current row in the glyph
		unsigned char *src_ptr = glyph->bitmap.buffer + y*glyph->bitmap.pitch;
		//dst ptr maps to the pens current Y pos, adjusted for the current glyph row
		//unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
		unsigned char *dst_ptr = texture[penY + y] + penX;
		//copy entire row
		memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph->bitmap.pitch);
		//for (int x = 0; x<glyph->bitmap.pitch; x++)
		//{
		//	dst_ptr[x] = src_ptr[x];
		//}
	}
}
void Text::TextureString::reload() {
	//Nothing
}