#ifndef __TowerCrane_h__
#define __TowerCrane_h__
#include "model/SceneGraphItem.h"

/**
 * This class wraps a scene graph for a basic animated tower crane
 */
class TowerCrane: public SceneGraphItem
{
public:
	static std::shared_ptr<TowerCrane> make();
private:
private:
	using SceneGraphItem::attach;
};

#endif //__TowerCrane_h__