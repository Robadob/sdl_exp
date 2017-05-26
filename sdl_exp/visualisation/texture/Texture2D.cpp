#include "Texture2D.h"

/*
Loads the 2D texture
@param texturePath The path to the image to be used as the texture
@param uniformName Name of the uniform to be used, defaults to the value of Texture::TEXTURE_UNIFORM_NAME
*/
Texture2D::Texture2D(const char *texturePath, const char *uniformName)
    : Texture(GL_TEXTURE_2D, texturePath, uniformName)
    , texturePath(texturePath)
    , dimensions(0)
{
    _reload();
}
/*
Loads the provided 2D texture into texture memory
This paramters of this function wrap setTexture()
@param uniformName Name of the uniform to be used, defaults to the value of Texture::TEXTURE_UNIFORM_NAME
@see setTexture(...)
*/
Texture2D::Texture2D(const char *uniformName, void *image, size_t imageSize, unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type)
    : Texture(GL_TEXTURE_2D, "", uniformName)
    , texturePath(0)
{
    setTexture(image, imageSize, width, height, internalFormat, format, type);
}
/*
Loads a texture from the provided pointer
@param image The pointer to the texture data
@param imageSize Size of the tex pointed to by image
@param width Width of the image
@param height Height of the image
@param internalFormat See GL docs for glTexImage2D
@param format See GL docs for glTexImage2D
@param type See GL docs for glTexImage2D
@note This function will not free the memory, you should do that yourself
@see https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
*/
void Texture2D::setTexture(void *image, size_t imageSize, unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type)
{
    GL_CALL(glBindTexture(texType, texName));
    if (width%4!=0)
    {
        GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    }
    GL_CALL(glTexImage2D(texType, 0, internalFormat, width, height, 0, format, type, image));
    GL_CALL(glBindTexture(texType, 0));
}
/**
 * Allocates a texture with specified params
@param width Width of the image
@param height Height of the image
@param internalFormat See GL docs for glTexImage2D
@param format See GL docs for glTexImage2D
@param type See GL docs for glTexImage2D
@note This function will not free the memory, you should do that yourself
@see https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
 */
void Texture2D::setTexture(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type)
{
	setTexture(nullptr, 0, width, height, internalFormat, format, type);
}
/*
Loads the 2D texture
@overrides Texture::reload()
*/
void Texture2D::reload() {
    _reload();
}
/*
Loads the 2D texture
@note this method is required, so that the constructor doesn't call a virtual function
*/
void Texture2D::_reload()
{
    if (texturePath)
    {
        SDL_Surface *img = readImage(texturePath);
        if (!img)
            return;
		flipRows(img);
        dimensions = glm::uvec2((unsigned int)img->w, (unsigned int)img->h);
        Texture::setTexture(img);
    }
}
/**
 * We use this when loading an image with SDL_Image to invert the image rows.
 * This is because most image formats label images with the origin in the top left corner
 * Whereas glTexImage2D expects the origin to be in the bottom left corner.
 * We could handle this by negating Y when using texcoords, however this better allows us to standardise shaders
 * @param img The SDL_Surface to be flipped
 * @return 0 on success, else failure
 * @note original source: http://www.gribblegames.com/articles/game_programming/sdlgl/invert_sdl_surfaces.html
 */
int Texture2D::flipRows(SDL_Surface *img)
{
	if (!img)
	{
		SDL_SetError("Surface is NULL");
		return -1;
	}
	int pitch = img->pitch;
	int height = img->h;
	void* image_pixels = img->pixels;
	int index;
	void* temp_row;
	int height_div_2;

	temp_row = (void *)malloc(pitch);
	if (NULL == temp_row)
	{
		SDL_SetError("Not enough memory for image inversion");
		return -1;
	}
	//if height is odd, don't need to swap middle row
	height_div_2 = (int)(height * .5);
	for (index = 0; index < height_div_2; index++) 	{
		//uses string.h
		memcpy((Uint8 *)temp_row,
			(Uint8 *)(image_pixels)+
			pitch * index,
			pitch);

		memcpy(
			(Uint8 *)(image_pixels)+
			pitch * index,
			(Uint8 *)(image_pixels)+
			pitch * (height - index - 1),
			pitch);
		memcpy(
			(Uint8 *)(image_pixels)+
			pitch * (height - index - 1),
			temp_row,
			pitch);
	}
	free(temp_row);
	return 0;
}