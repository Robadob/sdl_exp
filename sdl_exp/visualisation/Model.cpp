#include "Model.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
Model::Model(const char *modelPath)
	: modelPath(modelPath)
	, boundingBox()
{
	
}
Model::~Model()
{
	freeModel();
}
void Model::freeModel()
{
	
}


void Model::reload()
{
	freeModel();
	loadModel();
}
void Model::loadModel()
{
	//Import model with assimp
	const aiScene* scene = aiImportFile(modelPath, aiProcessPreset_TargetRealtime_MaxQuality);

	//Calculate bounding box
	boundingBox = std::make_shared<BoundingBox>(scene);

	//Build VBOs from Scene

	aiReleaseImport(scene);
}