#define _CRT_SECURE_NO_WARNINGS //vsnprintf()
#include "Text.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <stdarg.h>
#include <freetype/ftglyph.h>

namespace Stock
{
    namespace Font
    {
        const char* ARIAL = "C:/Windows/Fonts/Arial.ttf";
        const char* LUCIDIA_CONSOLE = "C:/Windows/Fonts/lucon.TTF.ttf";
        const char* SEGOE_UI = "C:/Windows/Fonts/segoeui.ttf";
        const char* JOKERMAN = "C:/Windows/Fonts/JOKERMAN.TTF";
        const char* TIMES_NEW_ROMAN = "C:/Windows/Fonts/times.ttf";
        const char* VIVALDI = "C:/Windows/Fonts/VIVALDII.TTF";
    };
};
//http://www.freetype.org/freetype2/docs/tutorial/step1.html
//http://www.freetype.org/freetype2/docs/tutorial/step2.html
Text::Text(char *string, unsigned int fontHeight, glm::vec3 color, char const *fontFile,unsigned int faceIndex)
    :Text(string, fontHeight, glm::vec4(color,1.0f),fontFile, faceIndex)
{}
Text::Text(char *_string, unsigned int fontHeight, glm::vec4 color, char const *fontFile,unsigned int faceIndex)
	: Overlay(std::make_shared<Shaders>(Stock::Shaders::TEXT))
	, padding(5)
    , lineSpacing(0.2f)
    , color(color)
    , backgroundColor(0.0f)
    , library()
    , font()
    , string(0)
    , fontHeight(fontHeight)
    , wrapDistance(800)
{
    //
    getShaders()->addDynamicUniform("_col", glm::value_ptr(this->color), 4);
    getShaders()->addDynamicUniform("_backCol", glm::value_ptr(this->backgroundColor), 4);
    if (!fontFile)
        fontFile = Stock::Font::ARIAL;
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
        fontFile = Stock::Font::ARIAL;
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
    setString(_string);
}
Text::~Text()
{
    if (font)
        FT_Done_Face(font);
    if (library)
        FT_Done_FreeType(library);
    if (string)
        free(this->string);
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

struct  TGlyph
{
    FT_UInt    index;  /* glyph index                  */
    FT_Vector  pos;    /* glyph origin on the baseline */
    FT_Glyph   image;  /* glyph image                  */
    char       c;
    int        line;
};
void Text::recomputeTex()
{
	setStringLen();
    FT_Error error;
    ////Measure tex width and height
    //unsigned int texWidth = 0;
    //unsigned int texHeight = fontHeight*2;
    //unsigned int lastSpace = 0;
    //unsigned int currentLineWidth = 0;
    //std::vector<int> lineStarts = {0};

    //First load and position all glyphs on a straight line
    TGlyph *glyphs = (TGlyph *)malloc(stringLen*sizeof(TGlyph));   /* glyph image    */
    unsigned int glyphPtr = 0;
    bool use_kerning = FT_HAS_KERNING(font)!=0;
    unsigned int previous = 0;

    int penX = 0, penY = 0;
    unsigned int i = 0;
    for (unsigned int n = 0; n < stringLen; n++)
    {
        glyphs[i].line = 0;
        glyphs[i].c = string[n];
        glyphs[i].index = FT_Get_Char_Index(font, string[n]);

        if (use_kerning && previous && glyphs[i].index)
        {
            FT_Vector  delta;


            FT_Get_Kerning(font, previous, glyphs[i].index,
                FT_KERNING_DEFAULT, &delta);

            penX += delta.x >> 6;
        }
        glyphs[i].pos.x = penX * 64;
        glyphs[i].pos.y = penY * 64;
        
        error = FT_Load_Glyph(font, glyphs[i].index, FT_LOAD_DEFAULT);
        if (error) continue;

        error = FT_Get_Glyph(font->glyph, &glyphs[i].image);
        if (error) continue;

        /* translate the glyph image now */
        FT_Glyph_Transform(glyphs[i].image, 0, &glyphs[i].pos); 
        
        penX += font->glyph->advance.x >> 6;
        previous = glyphs[i].index;
        i++;
    }
    //i Glyphs were loaded
    const unsigned int num_glyphs = i;
    glm::ivec2 origin;
    //Now we wrap the glyphs
    //Set origin to the bb min of 0th glyph

    FT_BBox  glyph_bbox;
    FT_Glyph_Get_CBox(glyphs[0].image, ft_glyph_bbox_pixels, &glyph_bbox);
    origin = glm::ivec2(glyph_bbox.xMin, glyph_bbox.yMin);
    for (i = 0; i < num_glyphs; i++)
    {
        if (!(glyphs[i].c == '\n' || glyphs[i].c == '\r'))
        {
            //Get chars bbox max x
            FT_Glyph_Get_CBox(glyphs[i].image, ft_glyph_bbox_pixels, &glyph_bbox);
            //Calculate the further most x coordinate of this char
            int xMax = (padding * 2) + glyph_bbox.xMax - origin.x;
            //If char exceeds wrapping dist
            if (xMax > (int)wrapDistance)
            {
                //Find the most recent space
                int j;
                for (j = i - 1; j >= 0; j--)
                {
                    if (glyphs[j].c == ' ')
                    {
                        //Mark char as not required (so we can ignore at render)
                        //and set this char to current
                        glyphs[j].c = '\n';
                        i = j;
                        break;
                    }
                }
                //Words exceeds wrap length, cancel wrapping
                if (j < 0)
                {
                    break;
                }
            }
            else
                continue;
        }
        //Move all subsequent chars to the next line
        if (i + 1 >= num_glyphs)
            continue;
        FT_Vector  newLineOffset;
        bool newline = true;
        if (glyphs[i].c == '\r')
            newline = false;
        newLineOffset.y = 0;
        /*else
            newLineOffset.y = font->height;*/
        FT_Glyph_Get_CBox(glyphs[i+1].image, ft_glyph_bbox_pixels, &glyph_bbox);
        newLineOffset.x = -(glyph_bbox.xMin - origin.x) * 64;
        for (unsigned int j = (i + 1); j < num_glyphs;j++)
        {
            if (newline)
                glyphs[j].line++;
            FT_Glyph_Transform(glyphs[j].image, 0, &newLineOffset);
        }
        //Continue
    }
    //Calculate the bounding box
    FT_BBox  bbox;
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;
    const int lineHeight = (int)((font->height >> 6)*(lineSpacing+1.0f));
    for (i = 0; i < num_glyphs; i++)
    {
        FT_Glyph_Get_CBox(glyphs[i].image, ft_glyph_bbox_pixels, &glyph_bbox);
        glyph_bbox.yMin += (glyphs[i].line*lineHeight);
        glyph_bbox.yMax += (glyphs[i].line*lineHeight);
        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;

        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;

        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;

        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }  
    if (bbox.xMin > bbox.xMax)
    {
        printf("unknown err, bounding box incorrect");
    }
    //And thus the texture size
    glm::ivec2 texDim(
        (2 * padding) + bbox.xMax - bbox.xMin,
        (2 * padding) + bbox.yMax - bbox.yMin
        );
    //Iterate chars, painting them to tex
    tex = std::make_unique<TextureString>(texDim.x, texDim.y);
    int ascender = font->ascender >> 6;//Calculate this manually with bb 36;// 
    for (i = 0; i < num_glyphs; i++)
    {
        if (!(glyphs[i].c == '\n' || glyphs[i].c == '\r'))
        {
            error = FT_Glyph_To_Bitmap(
                &glyphs[i].image,
                FT_RENDER_MODE_NORMAL,
                0,                  /* no additional translation */
                1);                /* destroy copy in "image"   */
            if (!error)
            {
                FT_BitmapGlyph  bit = (FT_BitmapGlyph)glyphs[i].image;
                penX = (int)padding + bit->left - bbox.xMin;
                penY = (int)padding - bbox.yMin - bit->top + ascender + (lineHeight*glyphs[i].line);
                if (penX >= 0 && penX + bit->bitmap.pitch < texDim.x && penY >= 0 && penY + (int)bit->bitmap.rows < texDim.y)
                    tex->paintGlyph(bit->bitmap, penX, penY);
                else
                    printf("Skipped painting char '%c' of '%s' to avoid writing out of bounds.\n", glyphs[i].c, string);
            }
        }
        FT_Done_Glyph(glyphs[i].image);
    }
	//link tex to shader
	tex->updateTex(getShaders());
	//Set width
    setDimensions(texDim.x, texDim.y);
    free(glyphs);
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
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    //GL_CALL(glTexStorage2D(texType, 1, GL_R8, width, height));//No mipmaps
    //GL_CALL(glTexSubImage2D(texType, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, texture[0]));//Don't want immutable storage, otherwise we need to regen text to resize
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MAX_LEVEL, 0));//Disable mipmaps
    GL_CALL(glTexImage2D(texType, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture[0]));
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
	GL_CALL(glBindTexture(texType, 0));
	bindToShader(shaders.get(), 0);
}
Text::TextureString::~TextureString()
{
	free(texture[0]);
	free(texture);
}
void Text::TextureString::paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY)
{
	for (unsigned int y = 0; y<glyph.rows; y++)
	{
		//src ptr maps to the start of the current row in the glyph
		unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
		//dst ptr maps to the pens current Y pos, adjusted for the current glyph row
		//unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
		unsigned char *dst_ptr = texture[penY + y] + penX;
		//copy entire row, skipping empty pixels (incase kerning causes char overlap)
		for (int x = 0; x<glyph.pitch; x++)
		{
            if (src_ptr[x])
			    dst_ptr[x] = src_ptr[x];
        }
        //memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
	}
}
void Text::TextureString::reload() {
	//Nothing
}
void Text::setColor(glm::vec3 color)
{
    this->color = glm::vec4(color, 1.0f);
}
void Text::setColor(glm::vec4 color)
{
    this->color = color;
}
void Text::setBackgroundColor(glm::vec3 color)
{
    this->backgroundColor = glm::vec4(color, 1.0f);
}
void Text::setBackgroundColor(glm::vec4 color)
{
    this->backgroundColor = color;
}
glm::vec4 Text::getColor()
{
    return color;
}
glm::vec4 Text::getBackgroundColor()
{
    return backgroundColor;
}
void Text::setString(char*fmt, ...)
{
    if (this->string)
        delete this->string;
    int bufSize = 0;
    int ct = 0;
    va_list argp;
    va_start(argp, fmt);
    char *buffer=0;
    do
    {//Repeat until buffer is large enough
        if (buffer)
            free(buffer);
        bufSize += 128;
        buffer = (char*)malloc(bufSize*sizeof(char));
        ct = vsnprintf(buffer, bufSize, fmt, argp);
    } while (ct >= bufSize);
    va_end(argp);
    this->string = buffer;
    recomputeTex();
}
