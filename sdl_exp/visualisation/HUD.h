#ifndef __HUD_h__
#define __HUD_h__

#include <list>
#include <tuple>
#include <memory>

#include "Overlay.h"

/*
Represents the orthographic plane covering the screen
Add/Remove overlays (e.g. text) to control where HUD elements are to be rendered.
*/
class HUD
{
	enum class HUDAnchorV { North, Center, South };
	enum class HUDAnchorH { West, Center, East };
	class Item
	{
	public:
		Item(std::shared_ptr<Overlay> overlay, int x, int y, HUDAnchorV anchorV = HUDAnchorV::Center, HUDAnchorH anchorH = HUDAnchorH::Center, int zIndex = 0);
		void resizeWindow(const unsigned int w, const unsigned int h);
		std::shared_ptr<Overlay> overlay;
		const int x;
		const int y;
		const HUDAnchorV anchorV;
		const HUDAnchorH anchorH;
		const int zIndex;
		GLuint vbo;
		GLuint fvbo;
		void *data;
		const int faces[] = { 0, 1, 2, 3 };
	};
public:
	HUD(unsigned int width, unsigned int height);
	/*
	Adds an overlay to the stack at the specified z-index
	If two items share the same z-index, the new item will insert as though it has the lower z-index (and be rendered underneath)
	*/
	void add(std::shared_ptr<Overlay> overlay, int x, int y, HUDAnchorV anchorV = HUDAnchorV::Center, HUDAnchorH anchorH = HUDAnchorH::Center, int zIndex = 0);

	/*
	Removes all Overlays from the stack which contain the passed shared pointer
	*/
	unsigned int removeAll(std::shared_ptr<Overlay> overlay);
	void clear();
	unsigned int getCount();
	void reload();
	void render();
	/*
	Recalculates projection matrix
	*/
	void resizeWindow(const unsigned int width, const unsigned int height);
private:
	const glm::mat4 modelViewMat;
	glm::mat4 projectionMat;
	//Holds the overlay elements to be rendered inr evers z-index order
	std::list<Item> stack;
};

#endif //ifndef __Scene_h__