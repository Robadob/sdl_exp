#ifndef __Sprite2D_h__
#define __Sprite2D_h__
#include "Texture/Texture2D.h"
#include "Overlay.h"
class Shaders;
/*
Class for rendering 2D graphics to the screen via an orthographic projection
*/
class Sprite2D : public Overlay
{
public:
    /**
     * Creates a new 2D Sprite overlay from the given image
     * @param imagePath Path to the image to be used
     * @param shader The shader to be used, if not provided will render as though standard RGBA image
     * @param width The width of the overlay, if 0 is passed the image width will be used
     * @param height The height of the overlay, if 0 is passed, the image height will be used unless width was specified, in which case the image will be scaled, preserving aspect ratio
     */
	Sprite2D(const char *imagePath, std::shared_ptr<Shaders> shader = nullptr, unsigned int width = 0, unsigned int height = 0);
    /**
     * Creates a new 2D Sprite overlay from the provide GL_TEXTURE_2D
     * @param texName The GL texture name as provided by glGenTextures()
     * @param shader The shader to be used, if not provided will render as though standard RGBA image
     * @param width The width of the overlay
     * @param height The height of the overlay
     */
	Sprite2D(GLuint texName, unsigned int width, unsigned int height, std::shared_ptr<Shaders> shader = nullptr);
    virtual ~Sprite2D(){};
    void reload() override{};
private:
    std::shared_ptr<const Texture2D> tex;
};
#endif //__Sprite2D_h__