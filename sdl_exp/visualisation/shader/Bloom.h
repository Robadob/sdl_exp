#ifndef __Bloom_h__
#define __Bloom_h__
#include <GL/glew.h>
#include "GaussianBlur.h"
#include "Shaders.h"

/**
 * Convenience wrapper for a Bloom post process step
 * Blurs the brightness texture
 * Composites the render and brightness into the active framebuffer
 */
class Bloom
{
public:
    /**
     * brightInTexName must have internal format R32F
     */
    Bloom(std::shared_ptr<Texture2D> renderTex, std::shared_ptr<Texture2D> brightnessTex);
    ~Bloom();
    void resize(const unsigned int &width, const unsigned int &height);
    void reload();
    void doBloom();
private:
    void blurBrightTex();
    void compositeTextures(); 
    std::shared_ptr<Texture2D> renderTex, brightInTex;
    std::unique_ptr<GaussianBlur> blurTool;
    std::shared_ptr<Texture2D> brightOutTex;
    //Bloom pass properties
    //This is basically a manual HUD::Item and Overlay, which should scale with calls to resize()
    glm::mat4 bloomProjMat;
    const glm::mat4 bloomViewMat;
    void *bloomQuadData;
    GLuint bloomQuadVbo;
    std::unique_ptr<Shaders> bloomQuadShader;
    void initBloomQuadProperties();
    void freeBloomQuadProperties();
    void updateBloomProjMat(const glm::uvec2 dimensions);
    void updateBloomQuad(const glm::uvec2 dimensions);
};

#endif //__Bloom_h__