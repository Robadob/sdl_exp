#ifndef __Overlay_h__
#define __Overlay_h__
#include "texture/Texture2D.h"

/*
Represents a 2d quad rendered in orthographic over the screen.
*/
class Overlay : Texture2D
{
public:
	Overlay();
	virtual void reload() = 0;
	void render(const glm::mat4 *mv, const glm::mat4 *proj, GLuint vbo);
	unsigned int getWidth() const
	{ return width; };
	unsigned int getHeight() const
	{ return height; }; 
	Shaders *Overlay::getShaders() const
	{ return shaders; }
protected:
	void setWidth(unsigned int w);
	void setHeight(unsigned int h);
	void setDimensions(unsigned int w, unsigned int h);
private:
	Shaders *shaders;
	unsigned int width;
	unsigned int height;
};

#endif //ifndef __Overlay_h__