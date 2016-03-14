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
    inline void clearMaterial();
protected:
    Shaders *shaders;
    //World scale of the longest side (in the axis x, y or z)
    const float SCALE;
    //Model vertex and face counts
    int v_count, f_count;
    glm::vec3 *vertices, *normals;
    glm::ivec3 *faces;
    GLuint vertices_vbo;
    GLuint faces_vbo;
    //Optional material (loaded automaically if detected within model file)
    Material *material;
    glm::vec3 color;
    glm::vec3 location;
    glm::vec4 rotation;
    
    void createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size);
    void deleteVertexBufferObject(GLuint *vbo);
    void fillBuffers();
    void loadModelFromFile(const char *path, float modelScale);
    void loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName);
    void allocateModel();
    void freeModel();
    void scaleModel(float modelScale);
    void freeMaterial();
};

#endif //ifndef __Entity_h__