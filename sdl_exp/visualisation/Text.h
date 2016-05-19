#ifndef __Text_h__
#define __Text_h__
#include "ft2build.h"
#include FT_FREETYPE_H
#include "Overlay.h"
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
		unsigned char **tex;
		unsigned int width;
		unsigned int height;
	};
public:
	static const char *FONT_ARIAL;
    Text(char *string, char const *fontFile = 0, unsigned int faceIndex = 0);
    virtual ~Text(); 
    void setFontHeight(unsigned int pixels);
	void reload() override;
private:
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