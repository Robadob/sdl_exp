#ifndef __GaussianBlur_h__
#define __GaussianBlur_h__
#include "ComputeShader.h"
#include <memory>

/**
 * Provides convenient compute shader Gaussian blur functionality
 */
class GaussianBlur : public Reloadable
{
    const char *GAUSSIAN_BLUR_SHADER_PATH = "../shaders/gaussian_blur.comp";
public:
    /**
     * Builds the compute shader and preconfigures the weights
     */
    GaussianBlur(unsigned int filterRadius, float sigma=1.0f);
    ~GaussianBlur();
    void blur2D(GLuint inTex, GLuint outTex, glm::uvec2 texDims);
    virtual void reload() override;
private:
    void generateFilter();
    //These are mapped to the shader uniforms
    const float sigma;
    const unsigned int filterRadius;
    const unsigned int filterWidth;
    glm::uvec2 imageDimensions;
    GLuint filterBuffer;
    GLint inBufferBind;
    GLint outBufferBind;
    std::shared_ptr<ComputeShader> blurShader;
    float *filter;
};

#endif //__GaussianBlur_h__