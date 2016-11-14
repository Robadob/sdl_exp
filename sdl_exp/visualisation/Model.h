#ifndef __Model_h__
#define __Model_h__
#include "Reloadable.h"
#include "glm/glm.hpp"
#include <memory>
#include "ModelNode.h"
#include "Shaders.h"
struct VFCcount;

class Bone
{
	struct VWPair
	{
		unsigned int vertex;
		float weight;
	};
	glm::mat4 offsetMatrix;
	std::vector<VWPair> vertexWeights;
};
struct VFCcount
{
	VFCcount(int i = 1) :v(0), f(0), c(i), b(0){}
	unsigned int v;
	unsigned int f;
	unsigned int c;
	unsigned int b;
	VFCcount &operator +=(const VFCcount &rhs)
	{
		v += rhs.v;
		f += rhs.f;
		c += rhs.c;
		b += rhs.b;
		return *this;
	}
};

struct ModelData
{
	ModelData(
		size_t vertices,
		size_t normals,
		size_t colors,
		size_t texcoords,
		size_t bones,
		size_t materials,
		size_t faces,
		size_t transforms
			)
			: vertices(nullptr)
			, normals(nullptr)
			, colors(nullptr)
			, texcoords(nullptr)
			, bones(nullptr)
			//, materials(nullptr)
			, faces(nullptr)
			, transforms(nullptr)

	{
		this->vertices = static_cast<glm::vec3 *>(malloc(vertices * sizeof(glm::vec3)));
		if (normals)
			this->normals = static_cast<glm::vec3 *>(malloc(normals * sizeof(glm::vec3)));
		if (colors)
			this->colors = static_cast<glm::vec4 *>(malloc(colors * sizeof(glm::vec4)));
		if (texcoords)
			this->texcoords = static_cast<glm::vec3 *>(malloc(texcoords * sizeof(glm::vec3)));

		if (bones)
			this->bones = static_cast<Bone *>(malloc(bones * sizeof(Bone)));

		//if (materials)
		//	this->materials = static_cast<Material *>(malloc(materials * sizeof(Material)));

		this->faces = static_cast<unsigned int *>(malloc(faces * sizeof(unsigned int)));
		this->transforms = static_cast<glm::mat4 *>(malloc(transforms * sizeof(glm::mat4)));
	}
	~ModelData()
	{
		//Free memory
		free(vertices);
		free(normals);
		free(colors);
		free(texcoords);
		free(bones);
		//free(materials);
		free(faces);
		free(transforms);
	}
	//Vertex attributes
	glm::vec3 *vertices;
	glm::vec3 *normals;
	glm::vec4 *colors;
	glm::vec3 *texcoords;

	//Bones
	Bone *bones;

	//Materials
	//Material *materials;//mat4?

	//Component attributes
	unsigned int *faces;
	glm::mat4 *transforms;
};
class Model : public Reloadable
{
public:
	/**
	*
	*/
	Model(const char *modelPath);
	~Model();
	/**
	* Reloads the model from file, rewriting GPU buffers
	*/
	void reload() override;

	void render(std::shared_ptr<Shaders> shaders=nullptr);
private:
	std::shared_ptr<ModelNode> buildHierarchy(const struct aiScene* scene, const struct aiNode* nd, VFCcount &vfc) const;
	void loadModel();
	void freeModel();

	std::shared_ptr<ModelNode> root;
	std::shared_ptr<ModelData> data;
	VFCcount vfc;
	const char *modelPath;

	//VBOs
	GLuint vbo;
	GLuint fbo;
	glm::vec3 location;
	glm::vec4 rotation;


	/**
	* Holds information for binding the vertex positions attribute
	*/
	Shaders::VertexAttributeDetail positions;
	/**
	* Holds information for binding the vertex normals attribute
	*/
	Shaders::VertexAttributeDetail normals;
	/**
	* Holds information for binding the vertex colours attribute
	*/
	Shaders::VertexAttributeDetail colors;
	/**
	* Holds information for binding the vertex texture coordinates attribute
	*/
	Shaders::VertexAttributeDetail texcoords;

};

#endif //__Model_h__