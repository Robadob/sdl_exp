#ifndef __HUD_h__
#define __HUD_h__

#include <list>
#include <tuple>
#include <memory>
#include "util/GLcheck.h"
#include <glm/glm.hpp>

class Overlay;

/*
Represents the orthographic plane covering the screen
Add/Remove overlays (e.g. Text, Sprite2D) to control where HUD elements are to be rendered.
*/
class HUD
{
public:
    enum class AnchorV { North, Center, South };
    enum class AnchorH { West, Center, East };
    /*
    Wraps Overlays which have been added to the HUD, to include details necessary for rendering them
    */
	class Item
	{
	public:
		Item(std::shared_ptr<Overlay> overlay, int x, int y, unsigned int window_w, unsigned int window_h, AnchorV anchorV = AnchorV::Center, AnchorH anchorH = AnchorH::Center, int zIndex = 0);
		void resizeWindow(const unsigned int w=0, const unsigned int h=0);
        void flipVertical();
        void flipHorizontal();
		std::shared_ptr<Overlay> overlay;
		const int x;
		const int y;
		const AnchorV anchorV;
		const AnchorH anchorH;
		const int zIndex;
		GLuint vbo;
		GLuint fvbo;
		void *data;
	};
	HUD(unsigned int width, unsigned int height);
	~HUD();
    void add(std::shared_ptr<Overlay> overlay, AnchorV anchorV = AnchorV::Center, AnchorH anchorH = AnchorH::Center, int x=0, int y=0, int zIndex = 0);
	unsigned int remove(std::shared_ptr<Overlay> overlay);
	void clear();
	unsigned int getCount();
	void reload();
	void render();
	void resizeWindow(const glm::uvec2 &dims);
private:
	const glm::mat4 modelViewMat;
	glm::mat4 projectionMat;
	//Holds the overlay elements to be rendered in reverse z-index order
	std::list<std::shared_ptr<Item>> stack;
	glm::uvec2 dims;
};

#endif //ifndef __Scene_h__