#ifndef __MaterialOld_h__
#define __MaterialOld_h__

#include <glm/glm.hpp>

class MaterialOld
{
public:
    MaterialOld();
    MaterialOld(glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, glm::vec4 emission, float shininess, float dissolve);
    ~MaterialOld();

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 emission; // Unused;
    float shininess;
    float dissolve;

    void useMaterial();
    void printToConsole();
};

#endif //ifndef __MaterialOld_h__