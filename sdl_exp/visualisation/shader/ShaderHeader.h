#ifndef __ShaderHeader_h__
#define __ShaderHeader_h__
/**
 * This header provides structs for use within shaders
 * The __cplusplus ifdefs, exist to allow inclusion in cpp code for binding the shaders
 * Structures include padding and member order to ensure that nothing crosses 4 byte lines
 */
#ifdef __cplusplus
#include <glm/glm.hpp>
#define vec3 glm::vec3
#endif
    
    struct MaterialProperties
    {
#ifdef __cplusplus
        MaterialProperties()
            : diffuse(0)
            , specular(0)
            , ambient(0)
            , emissive(0)
            , transparent(0)
            , opacity(1.0f)
            , shininess(0)
            , shininessStrength(1.0f)
            , refractionIndex(1.0f)
            , _padding(0)//Redundant
        { }
#endif
        vec3 diffuse;           //Diffuse color
        float opacity;
        vec3 specular;          //Specular color
        float shininess;
        vec3 ambient;           //Ambient color
        float shininessStrength;
        vec3 emissive;          //Emissive color (light emitted)
        float refractionIndex;
        vec3 transparent;       //Transparent color, multiplied with translucent light to construct final color
        float _padding;
    };

#ifdef __cplusplus
#undef vec3
#endif
#endif __ShaderHeader_h__

    //glGetUniformBlockIndex(GLuint program, const char *name);
    //glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);


    //GL_CALL(glGenBuffers(1, &ubo));
    //GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
    //GL_CALL(glBufferData(GL_UNIFORM_BUFFER, <size>, <pointer>, GL_STATIC_READ));
    //GLuint location = GL_CALL(glGetProgramResourceIndex(this->programId, GL_UNIFORM_BUFFER, "_material"));
    //if(location!=GL_INVALID_INDEX)
        //GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, location, ubo));
