#ifndef __Text_h__
#define __Text_h__
#include "ft2build.h"
#include FT_FREETYPE_H
#include "Overlay.h"

namespace Stock
{
    namespace Font
    {
        extern const char* ARIAL;
        extern const char* LUCIDIA_CONSOLE;
        extern const char* SEGOE_UI;
        extern const char* JOKERMAN;
        extern const char* TIMES_NEW_ROMAN;
        extern const char* VIVALDI;
    };
};
/*
Class for rendering strings to screen.
Windows stores font name-file name mappings in HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
Those installed fonts are then stored in C:/Windows/Fonts/
*/
class Text : public Overlay
{
	class TextureString : public Texture
	{
		public:
		TextureString(unsigned int width, unsigned int height);
		~TextureString();
		void paintGlyph(FT_GlyphSlot glyph, unsigned int penX, unsigned int penY);
		void updateTex(std::shared_ptr<Shaders> shaders);
		void reload() override;
	private:
		unsigned char **texture;
		unsigned int width;
		unsigned int height;
	};
public:
    Text(char *string, unsigned int fontHeight, glm::vec3 color, char const *fontFile = 0, unsigned int faceIndex = 0);
    Text(char *string, unsigned int fontHeight = 20, glm::vec4 color = glm::vec4(0.0f,0.0f,0.0f,1.0f), char const *fontFile = 0, unsigned int faceIndex = 0);
    virtual ~Text(); 
    void setFontHeight(unsigned int pixels);
	void reload() override;
    void setColor(glm::vec3 color);
    void setColor(glm::vec4 color);
    void setBackgroundColor(glm::vec3 color);
    void setBackgroundColor(glm::vec4 color);
    glm::vec4 getColor();
    glm::vec4 getBackgroundColor();
private:
    glm::vec4 color;
    glm::vec4 backgroundColor;
    void recomputeTex();
    void setStringLen();
    FT_Library  library;
    FT_Face     font;
    char *string;
    unsigned int stringLen;
    unsigned int fontHeight;
    unsigned int wrapDistance;
	std::unique_ptr<TextureString> tex;
};
#endif