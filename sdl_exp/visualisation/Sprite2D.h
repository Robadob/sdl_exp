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
    /**
     * Creates a new 2D Sprite overlay from the given image
     * @param imagePath Path to the image to be used
     * @param width The width of the overlay, if 0 is passed the image width will be used
     * @param height The height of the overlay, if 0 is passed, the image height will be used unless width was specified, in which case the image will be scaled, preserving aspect ratio
     */
    Sprite2D(char *imagePath, unsigned int width = 0, unsigned int height = 0);
    /**
     * Creates a new 2D Sprite overlay from the provide GL_TEXTURE_2D
     * @param texName The GL texture name as provided by glGenTextures()
     * @param width The width of the overlay
     * @param height The height of the overlay
     */
    Sprite2D(GLuint texName, unsigned int width, unsigned int height);
    virtual ~Sprite2D(){};
    void reload() override{};
private:
    std::shared_ptr<Texture2D> tex;
};
#endif //__Sprite2D_h__