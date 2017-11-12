#ifndef __Entity_h__
#define __Entity_h__

#include "util/GLcheck.h"

#include <memory>
#include <glm/glm.hpp>

#include "Material.h"
#include "shader/Shaders.h"
#include "texture/Texture2D.h"
#include "Camera.h"
#include "interface/Viewport.h"
#include "interface/Renderable.h"
#include "model/SceneGraphVertex.h"

namespace Stock
{
    namespace Models
    {
        struct Model
        {
            char *modelPath;
            char *texturePath;
            Stock::Shaders::ShaderSet defaultShaders;
		};
		const Model SPHERE{ "../models/sphere.obj", 0, Stock::Shaders::PHONG };
        const Model ICOSPHERE{ "../models/icosphere.obj", 0, Stock::Shaders::FLAT };
        const Model ICOSPHERE_COLOR{ "../models/icosphere_color.obj", 0, Stock::Shaders::COLOR };//Remove texture?
        const Model CUBE{ "../models/cube.obj", 0, Stock::Shaders::COLOR };
        const Model ROTHWELL{ "../models/rothwell-wy-1.obj", 0, Stock::Shaders::PHONG };
		const Model DEER{ "../models/deer.obj", "../textures/deer.tga", Stock::Shaders::TEXTURE };
		const Model TEAPOT{ "../models/teapot.obj", 0, Stock::Shaders::PHONG };
		const Model PLANE{ "../models/plane.obj", 0, Stock::Shaders::FLAT };
    };
};
/**
 * A renderable model loaded from a .obj file
 */
class Entity : public Renderable, public SceneGraphVertex, public Reloadable
{
    friend class Shaders;
public:
	static std::shared_ptr<Entity> load(
		Stock::Models::Model const model,
		float scale = 1.0f,
		std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		Stock::Models::Model const model,
		float scale,
		Stock::Shaders::ShaderSet const ss,
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		const char *modelPath,
		float modelScale = 1.0f,
		Stock::Shaders::ShaderSet const ss = Stock::Shaders::FIXED_FUNCTION,
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		); 
	static std::shared_ptr<Entity> load(
		const char *modelPath,
		float modelScale,
		std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		Stock::Models::Model const model,
		float scale = 1.0f,
		std::initializer_list<std::shared_ptr<Shaders>> shaders = {},
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		Stock::Models::Model const model,
		float scale,
		std::initializer_list<const Stock::Shaders::ShaderSet> ss,
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		const char *modelPath,
		float modelScale = 1.0f,
		std::initializer_list<const Stock::Shaders::ShaderSet> ss = {},
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		const char *modelPath,
		float modelScale,
		std::initializer_list<std::shared_ptr<Shaders>> shaders = {},
		std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr)
		);
	static std::shared_ptr<Entity> load(
		const char *modelPath,
		float modelScale,
		std::vector<std::shared_ptr<Shaders>> shaders,
		std::shared_ptr<const Texture> texture
		);
protected:    
	explicit Entity(
		const char *modelPath,
		float modelScale,
		std::vector<std::shared_ptr<Shaders>> shaders,
        std::shared_ptr<const Texture> texture
		);
public:
    virtual ~Entity();
	virtual void render(unsigned int shaderIndex = 0);
	virtual void renderInstances(int count, unsigned int shaderIndex = 0);
	virtual void render(const glm::mat4 &transform) override;
    void setColor(glm::vec3 color);
    inline void clearMaterial();
    void exportModel() const;
    virtual void reload();
	std::shared_ptr<Shaders> getShaders(unsigned int shaderIndex=0) const;
    void setViewMatPtr(glm::mat4 const *modelViewMat) override;
    void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
    void flipVertexOrder();
	void setCullFace(const bool cullFace);
	glm::vec3 getMin() const { return modelMin; }
	glm::vec3 getMax() const { return modelMax; }
	glm::vec3 getDimensions() const { return modelDims; }
protected:
    std::vector<std::shared_ptr<Shaders>> shaders;
    std::shared_ptr<const Texture> texture;
    //World scale of the longest side (in the axis x, y or z)
    const float SCALE;
    const char *modelPath;
    //Model vertex and face counts
    unsigned int vn_count;
    Shaders::VertexAttributeDetail positions, normals, colors, texcoords, faces;

    //Optional material (loaded automaically if detected within model file)
    Material *material;
    glm::vec4 color;

    static void createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size, void *data);
    static void deleteVertexBufferObject(GLuint *vbo);
    bool loadModelFromFile();
    void loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName);
    void freeMaterial();
    void generateVertexBufferObjects();
private:
	glm::vec3 modelMin, modelMax, modelDims;
	static std::vector<std::shared_ptr<Shaders>> convertToShader(std::initializer_list<const Stock::Shaders::ShaderSet> ss)
	{
		std::vector<std::shared_ptr<Shaders>> rtn;
		for (auto&& s : ss)
			rtn.push_back(std::make_shared<Shaders>(s.vertex, s.fragment, s.geometry));
		return rtn;
	}
    //Set by importModel if the imported model was of an older version.
    bool needsExport;
    bool cullFace;
    const static char *OBJ_TYPE;
    const static char *EXPORT_TYPE;
    inline static bool endsWith(const char *candidate, const char *suffix);
	bool importModel(const char *path);
    struct ExportMask
    {
        unsigned char FILE_TYPE_FLAG;
        unsigned char VERSION_FLAG;
        unsigned char SIZE_OF_FLOAT;
        unsigned char SIZE_OF_UINT;
        float         SCALE;
        unsigned int  VN_COUNT;
        unsigned int  FILE_HAS_VERTICES_3 : 1;
        unsigned int  FILE_HAS_VERTICES_4 : 1;
        unsigned int  FILE_HAS_NORMALS_3 : 1;//Currently normals can only be len 3, reserved regardless
        unsigned int  FILE_HAS_COLORS_3 : 1;
        unsigned int  FILE_HAS_COLORS_4 : 1;
        unsigned int  FILE_HAS_TEXCOORDS_2 : 1;
        unsigned int  FILE_HAS_TEXCOORDS_3 : 1;
        unsigned int  FILE_HAS_FACES_3 : 1;
        unsigned int  RESERVED_SPACE : 32;
    };
    const static unsigned char FILE_TYPE_FLAG = 0x12;
    const static unsigned char FILE_TYPE_VERSION = 1;
	glm::mat4 renderModelMat;
};
#endif //ifndef __Entity_h__
