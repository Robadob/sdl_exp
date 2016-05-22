#define _CRT_SECURE_NO_WARNINGS //vsnprintf()
#include "Text.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <stdarg.h>
#include <freetype/ftglyph.h>

namespace Stock
{
    /*
    Fast access fonts available to most Windows machines
    */
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
/*
Creates a text overlay with the provided string
@param string The text to be included in the overlay
@param fontHeight The pixel height of the text
@param color The rgb(0-1) color of the font
@param fontFile The path to the desired font
@param faceIndex The face within the font file to be used (most likely 0)
*/
Text::Text(char *string, unsigned int fontHeight, glm::vec3 color, char const *fontFile,unsigned int faceIndex)
    :Text(string, fontHeight, glm::vec4(color,1.0f),fontFile, faceIndex)
{}
/*
Creates a text overlay with the provided string
@param _string The text to be included in the overlay
@param fontHeight The pixel height of the text
@param color The rgba(0-1) color of the font
@param fontFile The path to the desired font
@param faceIndex The face within the font file to be used (most likely 0)
*/
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
        fprintf(stderr, "The font file %s is of an unsupport format, defaulting to Arial\n", fontFile);
        fontFile = Stock::Font::ARIAL;
        error = FT_New_Face(library,
            fontFile,
            0,
            &this->font);
    }
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst loading font file %s: %i\n", fontFile, error);
        return;
    }
    error = FT_Set_Pixel_Sizes(
        this->font,   /* handle to face object */
        0,      /* pixel_width           */
        this->fontHeight);/* pixel_height          */
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst setting font size: %i\n", error);
        return;
    }
    printf("Font %s was loaded successfully.\n", fontFile);
    setString(_string);
}
/*
Deallocates the loaded font and other allocated elements
*/
Text::~Text() {
    if (this->font)
        FT_Done_Face(this->font);
    if (this->library)
        FT_Done_FreeType(this->library);
    if (this->string)
        free(this->string);
}
/*
Repaints the text to a texture, according to the provided parameters
*/
void Text::reload() {
	recomputeTex();
}
/*
Structure used within Text::recomputeTex() to keep info about each glyph in a single structure
*/
struct  TGlyph
{
    FT_UInt    index;  /* glyph index                  */
    FT_Vector  pos;    /* glyph origin on the baseline */
    FT_Glyph   image;  /* glyph image                  */
    char       c;      /* char                         */
    int        line;   /* Line number                  */
};
/*
Repaints the text to a texture, according to the provided parameters
@note Based on http://www.freetype.org/freetype2/docs/tutorial/step2.html
*/
void Text::recomputeTex() {
	setStringLen();
    FT_Error error;

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

        //Add kerning if present
        if (use_kerning && previous && glyphs[i].index)
        {
            FT_Vector  delta;
            FT_Get_Kerning(font, previous, glyphs[i].index, FT_KERNING_DEFAULT, &delta);
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
    //Now we wrap the glyphs
    //Set origin to the bb min of 0th glyph
    FT_BBox  glyph_bbox;
    FT_Glyph_Get_CBox(glyphs[0].image, ft_glyph_bbox_pixels, &glyph_bbox);
    glm::ivec2 origin = glm::ivec2(glyph_bbox.xMin, glyph_bbox.yMin);
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
                //Only paint is the glyph is within bounds of the texture (report err if our maths is bad)
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
/*
Internal method used to update the variable stringLen according to the length of string
*/
void Text::setStringLen() {
    stringLen = 0;
    // ReSharper disable once CppPossiblyErroneousEmptyStatements
    while (string[stringLen++] != '\0');
    stringLen--;
}
/*
Sets the height of the font
@param pixels The height of the font in pixels
@param refreshTex Whether to automatically refresh the texture
*/
void Text::setFontHeight(unsigned int pixels, bool refreshTex) {
    if (!font)
    {
        fprintf(stderr, "Unable to set font height, font has not been loaded.\n");
        return;
    }
    this->fontHeight = pixels;
    FT_Error error = FT_Set_Pixel_Sizes(
        font,   /* handle to face object */
        0,      /* pixel_width           */
        this->fontHeight);/* pixel_height          */
    if (error)
    {
        fprintf(stderr, "An unexpected error occured whilst setting font size: %i\n", error);
        return;
    }
    if (refreshTex)
        recomputeTex();
}
/*
Returns the currently stored font height (measured in pixels)
@return The stored font height
@note If refreshTex was set to false when the height was updated, this may not reflect the rendered text
*/
unsigned int Text::getFontHeight() {
    return this->fontHeight;
}
/*
Sets the padding (distance between text bounding box and teture bounding box) of the overlay
@param pixels The padding of overlay in pixels
@param refreshTex Whether to automatically refresh the texture
*/
void Text::setPadding(unsigned int padding, bool refreshTex) {
    this->padding = padding;
    if (refreshTex)
        recomputeTex();
}
/*
Returns the currently stored padding (measured in pixels)
@return The stored padding
@note If refreshTex was set to false when the padding was updated, this may not reflect the rendered text
*/
unsigned int Text::getPadding() {
    return this->padding;
}
/*
Sets the maximum width of the texture, if text extends past this it will be wrapped
@param pixels The max width of the texture in pixels
@param refreshTex Whether to automatically refresh the texture
*/
void Text::setMaxWidth(unsigned int maxWidth, bool refreshTex) {
    this->wrapDistance = wrapDistance;
    if (refreshTex)
        recomputeTex();
}
/*
Returns the maximum width of the texture, if text extends past this it will be wrapped
@return The maximum width of the texture
@note If refreshTex was set to false when the max width was updated, this may not reflect the rendered text
*/
unsigned int Text::getMaxWidth() {
    return this->wrapDistance;
}
/*
Sets the line spacing, this is measured as a proporition of the line height
@param lineSpacing The line spacing, recommended values are 0.2-0.35
@param refreshTex Whether to automatically refresh the texture
*/
void Text::setLineSpacing(float lineSpacing, bool refreshTex) {
    this->lineSpacing = lineSpacing;
    if (refreshTex)
        recomputeTex();
}
/*
Returns the line spacing of the text
@return The line spacing of the text
@note If refreshTex was set to false when the line spacing was updated, this may not reflect the rendered text
*/
float Text::getLineSpacing() {
    return this->lineSpacing;
};
/*
Creates a new TextureString which represents the texture holding the glyphs of the string
@param width The width of the texture to be created
@param height The height of the texture to be created
*/
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
/*
Sets the color of the font
@param color The RGB(0-1) font color to be used when rendering the text
*/
void Text::setColor(glm::vec3 color) {
    this->color = glm::vec4(color, 1.0f);
}
/*
Sets the color of the font
@param color The RGBA(0-1) font color to be used when rendering the text
@note If an alpha value of -1 is used, you can achieve transparent text on a background
*/
void Text::setColor(glm::vec4 color) {
    this->color = color;
}
/*
Sets the color of the texture
@param color The RGB(0-1) background color to be used when rendering the text
*/
void Text::setBackgroundColor(glm::vec3 color) {
    this->backgroundColor = glm::vec4(color, 1.0f);
}
/*
Sets the color of the texture
@param color The RGBA(0-1) background color to be used when rendering the text
*/
void Text::setBackgroundColor(glm::vec4 color) {
    this->backgroundColor = color;
}
/*
Returns the font color used to render the text
*/
glm::vec4 Text::getColor() {
    return color;
}
/*
Returns the background color of the texture
*/
glm::vec4 Text::getBackgroundColor() {
    return backgroundColor;
}
/*
Updates the string using a string format
@param fmt Matches those used by functions such as printf(), sprintf() etc
@note This function simply wraps snprintf() for convenience
@note This function will always refresh the texture
*/
void Text::setString(char*fmt, ...) {
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
/*
Updates the GL texture to match the painted texture
@param shaders The shader object to bind the texture to
*/
void Text::TextureString::updateTex(std::shared_ptr<Shaders> shaders) {
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
/*
Frees the texture's data
*/
Text::TextureString::~TextureString() {
    free(texture[0]);
    free(texture);
}
/*
Paints a single character glyph to the texture at the specified location
@param penX The x coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
@param penY The y coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
*/
void Text::TextureString::paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY) {
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
/*
Not used
*/
void Text::TextureString::reload() {
    //Nothing
}