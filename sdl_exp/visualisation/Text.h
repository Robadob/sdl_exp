#ifndef __Text_h__
#define __Text_h__
#include "ft2build.h"
#include FT_FREETYPE_H
#include "texture/TextureFont.h"
#include "Overlay.h"
/*
Class for rendering strings to screen.
Windows stores font name-file name mappings in HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
Those installed fonts are then stored in C:/Windows/Fonts/
*/
class Text : public Overlay
{
    const char *FONT_ARIAL = "C:/Windows/Fonts/Arial.ttf";
    Text(char *string, char const *fontFile = 0, unsigned int faceIndex = 0);
    ~Text(); 
    void setFontHeight(unsigned int pixels);
	void reload() override;
	void _render() override;
private:
    void recomputeTex();
    void setStringLen();
    FT_Library  library;
    FT_Face     font;
    char *string;
    unsigned int stringLen;
    unsigned int fontHeight;
    unsigned int wrapDistance;
    char **tex;
};
#endif