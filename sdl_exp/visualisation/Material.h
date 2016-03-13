#ifndef __Material_h__
#define __Material_h__

#include <glm/glm.hpp>

class Material
{
public:
    Material();
    Material(glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, glm::vec4 emission, float shininess, float dissolve);
    ~Material();

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 emission; // Unused;
    float shininess;
    float dissolve;

    void useMaterial();
    void printToConsole();
};

#endif //ifndef __Maerial_h__