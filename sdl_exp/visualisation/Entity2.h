#ifndef __Entity2_h__
#define __Entity2_h__

#include <string>
#include "interface/Renderable.h"
#include <vector>
#include "shader/Shaders.h"
#include "texture/Texture2D.h"
#include <memory>

class Entity2 : public Renderable
{
	struct RenderGroup
	{
		RenderGroup(int materialId, unsigned int faceIndexStart, unsigned int faceComponentCount=3)
			: materialId(materialId)
			, faceIndexStart(faceIndexStart)
			, faceIndexCount(0)
			, faceComponentCount(faceComponentCount)
			, hasColors(false)
			, hasNormals(false)
			, hasTexCoords(false)
		{
		}

		int materialId;
		unsigned int faceIndexStart;
		unsigned int faceIndexCount;//Redundant, could just use next struct in array	
        unsigned int faceComponentCount;
        bool hasColors, hasNormals, hasTexCoords;
	};
	struct MaterialFile
	{
		MaterialFile()
			: opacity(1.0f)
			, shininess(1)
			, illuminationModel(-1)
		{
		}

		glm::vec3 diffuse;
		float opacity;
		glm::vec3 specular;
		float shininess;//Exponent?
		glm::vec3 ambient;
		int illuminationModel;
		std::string tex_diffuse;
		std::string tex_specular;
		std::string tex_ambient;
		std::string tex_specular_highlight;
		std::string tex_alpha;
		std::string tex_bump;
		std::string tex_displacement;
		std::string tex_decal;
		std::string tex_reflection;
	};
public:
    Entity2(const char * modelPath);
    Entity2(
        const float *vertices, unsigned int vComponents,
        const float *normals, unsigned int nComponents,
        const float *colors, unsigned int cComponents,
        const float *tCoords, unsigned int tcComponents,
        unsigned int count,
        const void *faces, size_t fSize, unsigned int fComponents, unsigned int fCount,
        std::shared_ptr<Texture2D> texture = nullptr);

    ~Entity2();
	void reload() override;
	void setViewMatPtr(glm::mat4 const *viewMat) override;
    void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
	glm::mat4 render(const unsigned int &shaderIndex=0, glm::mat4 transform=glm::mat4()) override;
	void setLocation(glm::vec3 location){ this->location = location; }
	void setRotation(glm::vec4 rotation){ this->rotation = rotation; }
	void setModelMat(glm::mat4 modelMat){ this->modelMat = modelMat;  };
private:
	void loadModel();
    void addFace(const unsigned int(&faceData)[3][4], const unsigned int(&faceDataCount)[3], unsigned int &faceCap,
        const float *t_verts, const float*t_colors, const float *t_texCoords, const float *t_normals,
        const RenderGroup &renderGroup);
    void generateVertexBufferObjects();
    const std::string modelPath;
    Shaders::VertexAttributeDetail vertices, normals, colors, texcoords, faces;
	std::vector<RenderGroup> renderGroup;				//Holds indexes of each render group and material
    std::vector<MaterialFile> materials;
    std::shared_ptr<Shaders> shaders;
    glm::vec4 rotation;
    glm::vec3 location;
	glm::mat4 modelMat;
    std::shared_ptr<Texture> texture;
};

#endif __Entity2_h__