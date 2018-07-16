#include "Bloom.h"
#include "../texture/Texture2D.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shaders.h"

Bloom::Bloom(std::shared_ptr<Texture2D> renderTex, std::shared_ptr<Texture2D> brightnessTex)
    : renderTex(renderTex)
    , brightInTex(brightnessTex)
    , blurTool(std::make_unique<GaussianBlur>(5, 5.0f))
    , brightOutTex(Texture2D::make(brightnessTex->getDimensions(), Texture::Format(GL_RED, GL_R32F, sizeof(float), GL_UNSIGNED_BYTE)))
    , bloomViewMat()//Default
{
    updateBloomProjMat(brightOutTex->getDimensions());
    //Check dimensions match (although they might change later
    assert(renderTex->getDimensions() == brightnessTex->getDimensions());
    //Check that brightInTex is of appropriate internal format
    {
        GLint a = 0;
        GL_CALL(glBindTexture(GL_TEXTURE_2D, brightnessTex->getName()));
        GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &a));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
        assert(a == GL_R32F);
    }
    initBloomQuadProperties();
}
Bloom::~Bloom()
{
    freeBloomQuadProperties();
}
void Bloom::resize(const unsigned int &width, const unsigned int &height)
{
    //Resize blur output texture
    brightOutTex->resize(glm::uvec2(width, height));
    //Resize the quad that we render across screen
    updateBloomProjMat(glm::uvec2(width, height));
    updateBloomQuad(glm::uvec2(width, height));
}
void Bloom::doBloom()
{
    blurBrightTex();
    compositeTextures();
}
void Bloom::blurBrightTex()
{
    //Unsure if really necessary to repeat blur, we could just create a custom filter
    for (unsigned int i = 0; i < 5; ++i)
        blurTool->blurR32F(brightInTex, brightOutTex);
}
void Bloom::compositeTextures()
{
    bloomQuadShader->useProgram();
    GL_CALL(glDisable(GL_DEPTH_TEST));
    GL_CALL(glDisable(GL_BLEND));
    GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    bloomQuadShader->clearProgram();
}
void Bloom::updateBloomProjMat(const glm::uvec2 dimensions)
{
    bloomProjMat =
        glm::ortho<float>(
        0.0f, (float)dimensions.x,
        0.0f, (float)dimensions.y,
        0.0f, 1.0f
        );
}
void Bloom::updateBloomQuad(const glm::uvec2 dimensions)
{
    //Track parameters, so when called from overlay we can reuse previous
    const float depth = -0.5f;
    glm::vec3 *topLeft = static_cast<glm::vec3*>(bloomQuadData);
    glm::vec3 *bottomLeft = topLeft + 1;
    glm::vec3 *topRight = bottomLeft + 1;
    glm::vec3 *bottomRight = topRight + 1;
    *bottomLeft = glm::vec3(0, 0, depth);
    //Adjust other corners relative to topLeft & overlay size
    *topLeft = glm::vec3(*bottomLeft);
    *bottomRight = glm::vec3(*bottomLeft);
    *topRight = glm::vec3(*bottomLeft);
    topLeft->y += dimensions.y;
    bottomRight->x += dimensions.x;
    topRight->x += dimensions.x;
    topRight->y += dimensions.y;

    //update data within vbo
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bloomQuadVbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), bloomQuadData));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    //If required, pass to shader
    //bloomQuadShader->addStaticUniform("_viewportDims", glm::value_ptr(dimensions), 2);
}
void Bloom::initBloomQuadProperties()
{
    //Init Shader shader
    bloomQuadShader = std::make_unique<Shaders>("default.vert", "bloom.frag");
    bloomQuadShader->setProjectionMatPtr(&bloomProjMat);
    bloomQuadShader->setViewMatPtr(&bloomViewMat);
    bloomQuadShader->addTexture("t_color", renderTex->getType(), renderTex->getName(), renderTex->getTextureUnit());
    bloomQuadShader->addTexture("t_brightness", brightOutTex->getType(), brightOutTex->getName(), brightOutTex->getTextureUnit());
    //Init vbo's
    unsigned int bufferSize = 0;
    bufferSize += 4 * sizeof(glm::vec3); //4 points to a quad
    bufferSize += 4 * sizeof(glm::vec2);//4 points to a tex coord
    bloomQuadData = malloc(bufferSize);
    //Setup tex coords
    glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(bloomQuadData)+4));
    texCoords[0] = glm::vec2(0.0f, 1.0f); //TopLeft
    texCoords[1] = glm::vec2(0.0f, 0.0f); //BottomLeft
    texCoords[2] = glm::vec2(1.0f, 1.0f); //TopRight
    texCoords[3] = glm::vec2(1.0f, 0.0f); //BottomRight
	//Initalise buffer
    GL_CALL(glGenBuffers(1, &bloomQuadVbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bloomQuadVbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, bloomQuadData, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Setup vertices
    updateBloomQuad(brightOutTex->getDimensions());
    //Link Vertex Attributes TO SHADER??!?!??!?
    Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
    pos.vbo = bloomQuadVbo;
    pos.count = 4;
    pos.data = bloomQuadData;
    pos.offset = 0;
    pos.stride = 0;
    bloomQuadShader->setPositionsAttributeDetail(pos);
    Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
    texCo.vbo = bloomQuadVbo;
    texCo.count = 4;
    texCo.data = texCoords;
    texCo.offset = 4 * sizeof(glm::vec3);
    texCo.stride = 0;
    bloomQuadShader->setTexCoordsAttributeDetail(texCo);
}
void Bloom::freeBloomQuadProperties()
{
    bloomQuadShader.reset();
    free(bloomQuadData);
    GL_CALL(glDeleteBuffers(1, &bloomQuadVbo));
}