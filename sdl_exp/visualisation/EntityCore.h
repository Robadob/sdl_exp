#ifndef __EntityCore_h__
#define __EntityCore_h__
#include "Reloadable.h"
#include "Shaders.h"
#include <memory>

class EntityCore : public Reloadable
{
protected:
	/**
	 *
	 */
	EntityCore(char *modelPath, std::unique_ptr<Shaders>);
public:
	/**
	 * Reloads the model and also triggers shader reload
	 */
	void reload() override;
	void render();
	//void renderInstanced(unsigned int count);
	
	std::shared_ptr<Shaders> shaders;
};

#endif //__EntityCore_h__