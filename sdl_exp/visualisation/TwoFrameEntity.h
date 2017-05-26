#ifndef __TwoFrameEntity_h__
#define __TwoFrameEntity_h__

#include "GLcheck.h"

#include <memory>
#include <glm/glm.hpp>

#include "Material.h"
#include "Shaders.h"
#include "texture/Texture2D.h"
#include "Camera.h"
#include "Viewport.h"
/*This class is a quick hack to produce a two-frame/animated entity.
The whole entity class needs a huge refactor to better faciliate many the diverse forms of entity (instanced, static, animated, 2frame animated etc)
However we don't currently have the time for that
*/
namespace Stock
{
	namespace Model
	{
		namespace TwoFrame
		{
			struct Set
			{
				char *modelPath1;
				char *modelPath2;
				char *texturePath1;
				char *texturePath2;
				Stock::Shaders::ShaderSet defaultShaders;
			};
			const Set PEDESTRIAN{ "../models/pedestrian-left.obj", "../models/pedestrian-right.obj", nullptr, nullptr, Stock::Shaders::TWO_FRAME_FLAT };
		}
	};
};
class TwoFrameEntity
{
public:
	explicit TwoFrameEntity(
		Stock::Model::TwoFrame::Set const model,
		float scale = 1.0f,
		std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
		std::shared_ptr<Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	virtual ~TwoFrameEntity();
	virtual void render();
	void renderInstances(int count);
private:
	std::shared_ptr<Shaders> shaders;
	std::shared_ptr<Texture> texture;
	//World scale of the longest side (in the axis x, y or z)
	const float SCALE;
	const char *modelPath1;
	const char *modelPath2;
	//Model vertex and face counts
	unsigned int vn_count;
	Shaders::VertexAttributeDetail positions1, normals1, colors1, texcoords1;
	Shaders::VertexAttributeDetail positions2, normals2, colors2, texcoords2;
	Shaders::VertexAttributeDetail faces;
		
};

#endif