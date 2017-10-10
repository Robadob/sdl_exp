#ifndef __Overlay_h__
#define __Overlay_h__
#include "texture/Texture2D.h"
#include <memory>
#include "HUD.h"

/*
Represents a 2d quad rendered in orthographic over the screen.
*/
class Overlay
{
	friend class HUD;
public:
	Overlay(std::shared_ptr<Shaders> shaders, unsigned int width=0, unsigned int height=0);
	virtual ~Overlay(){};
	void _reload() { shaders->reload(); reload(); };
	virtual void reload()=0;
	void render(const glm::mat4 *mv, const glm::mat4 *proj, GLuint vbo);
	unsigned int getWidth() const
	{ return width; };
	unsigned int getHeight() const
	{ return height; }; 
	std::shared_ptr<Shaders> Overlay::getShaders() const
	{ return shaders; }
    void setVisible(bool isVisible);
    bool getVisible() const
	{
		return visible;
	}
	void setWidth(unsigned int w);
	void setHeight(unsigned int h);
    void setDimensions(unsigned int w, unsigned int h);
    void flipVertical();
    void flipHorizontal();
protected:
	std::weak_ptr<HUD::Item> hudItem;
private:
    bool visible;
	void setHUDItem(std::shared_ptr<HUD::Item> ptr);
	std::shared_ptr<Shaders> shaders;
	unsigned int width;
	unsigned int height;
    bool flippedV, flippedH;
};

#endif //ifndef __Overlay_h__