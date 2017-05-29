#ifndef __Texture2D_h__
#define __Texture2D_h__
#include "Texture.h"

class Texture2D : public Texture
{
public:
	Texture2D(const char *texturePath, const char *uniformName = 0);
	Texture2D(const char *uniformName, void *image, size_t imageSize, unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type);
    void reload() override;
	void setTexture(void *image, size_t imageSize, unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type);
	void setTexture(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type);
    unsigned int getWidth() const { return dimensions.x; }
    unsigned int getHeight() const { return dimensions.y; }
	glm::uvec2 getDimensions() const { return dimensions; }
private:
	static int flipRows(SDL_Surface *img);
    void _reload();//Used so we don't call a virtual fn from the constructor
    const char *texturePath;
    glm::uvec2 dimensions;
};
#endif //ifndef __Texture2D_h__