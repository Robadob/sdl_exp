#ifndef __Sprite2D_h__
#define __Sprite2D_h__
#include "Texture/Texture2D.h"
#include "Overlay.h"

/*
Class for rendering 2D graphics to the screen via an orthographic projection
*/
class Sprite2D : public Overlay
{
public:
    Sprite2D(char *imagePath, unsigned int width=0, unsigned int height=0);
    virtual ~Sprite2D(){};
    void reload() override{};
private:
    Texture2D tex;
};
#endif //__Sprite2D_h__