#include "TextBillboard.h"
#include <glm/gtc/matrix_transform.hpp>
#include "CellBillboard.h"

TextBillboard::TextBillboard(std::shared_ptr<const NoClipCamera> camera, const std::string &str)
    : billboardShaders(std::make_shared<Shaders>("../sdl_exp/primage/text_billboard.vert", "../sdl_exp/primage/text.frag"))
    , billboardData(nullptr)
    //: billboardShaders(std::make_shared<Shaders>("../sdl_exp/primage/instanced.vert", "../sdl_exp/primage/tumourcell_flat.frag"))
    //: billboardShaders(std::make_shared<Shaders>("../sdl_exp/primage/billboard.vert", "../sdl_exp/primage/tumourcell_flat.frag"))
    , billboardVBO(0)
    , billboardFVBO(0)
    , text(std::make_shared<Text>(str.c_str(), 45, glm::vec3(0), Stock::Font::ARIAL))
    , modelMat(glm::scale(glm::mat4(1), glm::vec3(0.1f)))
{
    //Setup our billboard
    this->billboardShaders->setModelMatPtr(&modelMat);
    this->billboardShaders->addDynamicUniform("_up", reinterpret_cast<const GLfloat *>(camera->getUpPtr()), 3);
    this->billboardShaders->addDynamicUniform("_right", reinterpret_cast<const GLfloat *>(camera->getRightPtr()), 3);
    const glm::vec4 BLACK = glm::vec4(0,0,0,1);
    const glm::vec4 TRANSPARENT = glm::vec4(0);
    this->billboardShaders->addStaticUniform("_col", reinterpret_cast<const GLfloat *>(&BLACK), 4);
    this->billboardShaders->addStaticUniform("_backCol", reinterpret_cast<const GLfloat *>(&TRANSPARENT), 4);
    this->billboardShaders->addDynamicUniform("loc", reinterpret_cast<const GLfloat *>(&translation), 3);
    unsigned int bufferSize = 0;
    bufferSize += 4 * sizeof(glm::vec3); //4 points to a quad
    bufferSize += 4 * sizeof(glm::vec2);//4 points to a tex coord
    billboardData = malloc(bufferSize);
    //Setup tex coords
    glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(billboardData) + 4));
    texCoords[0] = glm::vec2(0.0f, 1.0f); //TopLeft
    texCoords[1] = glm::vec2(0.0f, 0.0f); //BottomLeft
    texCoords[2] = glm::vec2(1.0f, 1.0f); //TopRight
    texCoords[3] = glm::vec2(1.0f, 0.0f); //BottomRight
                                          //Initalise buffer
    GL_CALL(glGenBuffers(1, &billboardVBO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, billboardVBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, billboardData, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Set texture
    this->billboardShaders->addTexture("_texture", text->getTex());
    //Setup vertices
    glm::vec3 *topLeft = static_cast<glm::vec3*>(billboardData);
    glm::vec3 *bottomLeft = topLeft + 1;
    glm::vec3 *topRight = bottomLeft + 1;
    glm::vec3 *bottomRight = topRight + 1;
    const glm::uvec2 stringDims = glm::uvec2(text->getTex()->getDimensions().y, text->getTex()->getDimensions().x);
    *topLeft = glm::vec3(-(stringDims.x / 2.0f), -(stringDims.y / 2.0f), 0);
    *bottomLeft = glm::vec3((stringDims.x / 2.0f), -(stringDims.y / 2.0f), 0);
    *topRight = glm::vec3(-(stringDims.x / 2.0f), (stringDims.y / 2.0f), 0);
    *bottomRight = glm::vec3((stringDims.x / 2.0f), (stringDims.y / 2.0f), 0);
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, billboardVBO));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), billboardData));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Link Vertex Attributes
    Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
    pos.vbo = billboardVBO;
    pos.count = 4;
    pos.data = billboardData;
    pos.offset = 0;
    pos.stride = 0;
    this->billboardShaders->setPositionsAttributeDetail(pos);
    Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
    texCo.vbo = billboardVBO;
    texCo.count = 4;
    texCo.data = texCoords;
    texCo.offset = 4 * sizeof(glm::vec3);
    texCo.stride = 0;
    this->billboardShaders->setTexCoordsAttributeDetail(texCo);
    //Setup faces
    GL_CALL(glGenBuffers(1, &billboardFVBO));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardFVBO));
    const int faces[] = { 0, 1, 2, 3 };
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(int), &faces, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
TextBillboard::~TextBillboard()
{
    text.reset();
    if (billboardData)
        free(billboardData);
    if (billboardVBO)
        GL_CALL(glDeleteBuffers(1, &billboardVBO));
    if (billboardFVBO)
        GL_CALL(glDeleteBuffers(1, &billboardFVBO));
}

void TextBillboard::render(const glm::vec3 &translation) {
    this->translation = translation;
    //Render them
   // GL_CALL(glEnable(GL_BLEND));
   // GL_CALL(glDisable(GL_CULL_FACE));
    this->billboardShaders->useProgram();
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardFVBO));
    GL_CALL(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr));
    //Unload shader
    //glEnable(GL_CULL_FACE);
    //GL_CALL(glDisable(GL_BLEND));
    this->billboardShaders->clearProgram();
}

void TextBillboard::reload()
{
    this->billboardShaders->reload();
}
void TextBillboard::setViewMatPtr(glm::mat4 const *viewMat)
{
    this->billboardShaders->setViewMatPtr(viewMat);
}
void TextBillboard::setProjectionMatPtr(glm::mat4 const *projectionMat)
{
    this->billboardShaders->setProjectionMatPtr(projectionMat);
}
void TextBillboard::setLightsBuffer(const GLuint &bufferBindingPoint)
{
    this->billboardShaders->setLightsBuffer(bufferBindingPoint);
}