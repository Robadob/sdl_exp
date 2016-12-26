#ifndef __GaussianBlur_h__
#define __GaussianBlur_h__
#include "ComputeShader.h"
#include <memory>

/**
 * Provides convenient compute shader Gaussian blur functionality
 * Currently only supports 2D textures of format R32F (single channel textures)
 * Following the existing code it would be trivial to add multi-channel versions
 * The max filter width is limited by the max permitted shader shared memory, work group size and channels.
 * The minimum required shared memory is 32kb, aka 8k single precision floating point values
 */
class GaussianBlur : public Reloadable
{
    const char *GAUSSIAN_BLUR_SHADER_PATH = "../shaders/gaussian_blur.comp";
public:
    /**
     * Builds the compute shader and preconfigures the weights
     */
	GaussianBlur(unsigned int filterRadius, float sigma = 1.0f);
	~GaussianBlur();
	void blurR32F(GLuint inTex, GLuint outTex, glm::uvec2 texDims);
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