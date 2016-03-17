#ifndef __Entity_h__
#define __Entity_h__

#include "GLcheck.h"

#include <glm/glm.hpp>

#include "Material.h"
#include "Shaders.h"

/*
A renderable model loaded from a .obj file
*/
class Entity
{
public:
    Entity(const char *modelPath, float modelScale=1.0, Shaders *shaders=0);
    ~Entity();
    void render();
    void renderInstances(int count, GLuint vertLocation = 0, GLuint normalLocation = 1);
    void setColor(glm::vec3 color);
    void setLocation(glm::vec3 location);
    void setRotation(glm::vec4 rotation);
    glm::vec3 getLocation() const;
    glm::vec4 getRotation() const;
    inline void clearMaterial(); 
    void exportModel() const;
protected:
    Shaders *shaders;
    //World scale of the longest side (in the axis x, y or z)
    const float SCALE;
    const char *modelPath;
    //Model vertex and face counts
    unsigned int vn_count;
    unsigned int v_count, n_count, c_count, t_count, f_count;//Number of
    unsigned int v_size, t_size, c_size;//Number of components

    float *vertices, *normals, *colors, *textures;
    unsigned int *faces;

    GLuint vertices_vbo, normals_vbo, colors_vbo, textures_vbo, faces_vbo;

    //Optional material (loaded automaically if detected within model file)
    Material *material;
    glm::vec3 color;
    glm::vec3 location;
    glm::vec4 rotation;
    
    void createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size, void *data);
    void deleteVertexBufferObject(GLuint *vbo);
    void loadModelFromFile();
    void loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName);
    void freeModel();
    void scaleModel(float modelScale);
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
        unsigned int  FILE_HAS_TEXTURES_2 : 1;
        unsigned int  FILE_HAS_TEXTURES_3 : 1;
        unsigned int  FILE_HAS_FACES_3 : 1;
        unsigned int  RESERVED_SPACE : 32;
    };
    const static unsigned char FILE_TYPE_FLAG = 0x12;
    const static unsigned char FILE_TYPE_VERSION = 1;
};
#endif //ifndef __Entity_h__
