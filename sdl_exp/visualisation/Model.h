#ifndef __Model_h__
#define __Model_h__
#include "Reloadable.h"
#include "glm/glm.hpp"
#include "BoundingBox.h"
#include <memory>

class aiScene;
class Model : public Reloadable
{
protected:
	/**
	*
	*/
	Model(const char *modelPath);
	~Model();
public:
	/**
	* Reloads the model from file, rewriting GPU buffers
	*/
	void reload() override;
private:
	void loadModel();
	void freeModel();
	void setupBoundingBox(aiScene *scene);
	const char *modelPath;
	std::shared_ptr<BoundingBox> boundingBox;
};

#endif //__Model_h__