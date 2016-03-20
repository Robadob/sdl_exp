#ifndef __Entity_h__
#define __Entity_h__

#include "GLcheck.h"

#include <memory>
#include <glm/glm.hpp>

#include "Material.h"
#include "Shaders.h"
#include "Texture2D.h"

namespace Stock
{
    namespace Models
    {
        struct Model
        {
            char *modelPath;
            char *texturePath;
            Stock::Shaders::ShaderSet defaultShaders;;
        };
        const Model ICOSPHERE{ "../models/icosphere.obj", 0, Stock::Shaders::FLAT };
        const Model ICOSPHERE_COLOR{ "../models/icosphere_color.obj", "../textures/deer.tga", Stock::Shaders::COLOR };//Remove texture?
        const Model CUBE{ "../models/cube.obj", 0, Stock::Shaders::COLOR };
        const Model ROTHWELL{ "../models/rothwell-wy-1.obj", 0, Stock::Shaders::PHONG };
        const Model DEER{ "../models/deer.obj", "../textures/deer.tga", Stock::Shaders::TEXTURE };
    };
};
/*
A renderable model loaded from a .obj file
*/
//template<class T>
class Entity
{
public:
    template<class T = Texture2D>
    Entity(
        Stock::Models::Model const model,
        float scale = 1.0f,
        std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
        std::shared_ptr<T> texture = std::shared_ptr<T>(nullptr)
        );
    template<class T = Texture2D>
    Entity(
        Stock::Models::Model const model,
        float scale,
        Stock::Shaders::ShaderSet const ss,
        std::shared_ptr<T> texture = std::shared_ptr<T>(nullptr)
        );
    template<class T = Texture2D>
    Entity(
        const char *modelPath,
        float modelScale = 1.0f,
        Stock::Shaders::ShaderSet const ss = Stock::Shaders::FLAT,
        std::shared_ptr<T> texture = std::shared_ptr<T>(nullptr)
        );
    template<class T = Texture2D>
    Entity(
        const char *modelPath,
        float modelScale,
        std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
        std::shared_ptr<T> texture = std::shared_ptr<T>(nullptr)
        ); 
    virtual ~Entity();
    void render();
    void renderInstances(int count);
    void setColor(glm::vec3 color);
    void setLocation(glm::vec3 location);
    void setRotation(glm::vec4 rotation);
    glm::vec3 getLocation() const;
    glm::vec4 getRotation() const;
    inline void clearMaterial();
    void exportModel() const;
    std::shared_ptr<Shaders> getShaders() const;
protected:
    std::shared_ptr<Shaders> shaders;
    std::shared_ptr<Texture> texture;
    //World scale of the longest side (in the axis x, y or z)
    const float SCALE;
    const char *modelPath;
    //Model vertex and face counts
    unsigned int vn_count;
    Shaders::VertexAttributeDetail positions, normals, colors, texcoords, faces;

    //Optional material (loaded automaically if detected within model file)
    Material *material;
    glm::vec3 color;
    glm::vec3 location;
    glm::vec4 rotation;

    static void createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size, void *data);
    static void deleteVertexBufferObject(GLuint *vbo);
    void loadModelFromFile();
    void loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName);
    void freeMaterial();
    void Entity::generateVertexBufferObjects();
private:
    const static char *OBJ_TYPE;
    const static char *EXPORT_TYPE;
    inline static bool endsWith(const char *candidate, const char *suffix);
    void importModel(const char *path);
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
};
#endif //ifndef __Entity_h__
