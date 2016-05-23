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
		TextureString();
        ~TextureString(); 
        void resize(unsigned int width, unsigned int height);
        void paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
        void paintGlyphMono(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
		void updateTex(std::shared_ptr<Shaders> shaders);
		void reload() override;
	private:
		unsigned char **texture;
		unsigned int width;
		unsigned int height;
	};
public:
    Text(const char *string, unsigned int fontHeight, glm::vec3 color, char const *fontFile = 0, unsigned int faceIndex = 0);
    Text(const char *string, unsigned int fontHeight = 20, glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), char const *fontFile = 0, unsigned int faceIndex = 0);
    virtual ~Text(); 
	void reload() override;
    void setColor(glm::vec3 color);
    void setColor(glm::vec4 color);
    void setBackgroundColor(glm::vec3 color);
    void setBackgroundColor(glm::vec4 color);
    void setFontHeight(unsigned int pixels, bool refreshTex = true);
    unsigned int getFontHeight();
    void setPadding(unsigned int padding, bool refreshTex=true);
    unsigned int getPadding();
    void setMaxWidth(unsigned int maxWidth, bool refreshTex = true);
    unsigned int getMaxWidth();
    void setLineSpacing(float lineSpacing, bool refreshTex = true); 
    bool getUseAA();
    void setUseAA(bool useAA, bool refreshTex = true);
    float getLineSpacing();
    glm::vec4 getColor();
    glm::vec4 getBackgroundColor(); 
    void setString(const char*fmt, ...);
private:
    bool printMono;
    unsigned int padding;
    float lineSpacing;//Line spacing calculated as a percentage of font Height
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
#endif //__Text_h__