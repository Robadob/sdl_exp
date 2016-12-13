#include "GaussianBlur.h"
#include <glm/gtc/type_ptr.hpp>


GaussianBlur::GaussianBlur(unsigned int filterRadius, float sigma)
    : sigma(sigma)
    , filterRadius(glm::clamp<unsigned int>(filterRadius,1,16))
    , filterWidth(2 * glm::clamp<unsigned int>(filterRadius, 1, 16) + 1)
    , blurShader(std::make_shared<ComputeShader>(GAUSSIAN_BLUR_SHADER_PATH))
    , filter(nullptr)
{
    if (filterRadius != this->filterRadius)
        fprintf(stderr, "Gaussian filter radius was clamped from %d to %d.\n", filterRadius, this->filterRadius);
    //For the time being we will allocate ourselves the final 2 texture units
    GLint maxTex;
    //GL_CALL(glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxTex));
    this->inBufferBind = 0;
    this->outBufferBind = 1;
    //Generate the Gaussian filter (manually at current)
    this->filter = (float*)malloc((filterWidth+1)*sizeof(float));
    generateFilter();
    //Copy that to a uniform buffer
    GL_CALL(glGenBuffers(1, &this->filterBuffer));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, this->filterBuffer));
    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, (filterWidth + 1)*sizeof(float), this->filter, GL_STATIC_READ));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    //Configure uniforms
    blurShader->addBuffer("_filterWeights", GL_UNIFORM_BLOCK, this->filterBuffer);
    blurShader->addStaticUniform("_filterRadius", &this->filterRadius);
    blurShader->addStaticUniform("_filterWidth", &this->filterWidth);
    blurShader->addStaticUniform("_imageIn", &this->inBufferBind);
    blurShader->addStaticUniform("_imageOut", &this->outBufferBind);
    blurShader->addDynamicUniform("_imageDimensions", glm::value_ptr(this->imageDimensions), 2);
}
GaussianBlur::~GaussianBlur()
{
    GL_CALL(glDeleteBuffers(1,&this->filterBuffer));
    free(filter);
}
void GaussianBlur::generateFilter()
{
    //Fill filter
    unsigned int i = 0;
    for (unsigned int x = 0; x < filterWidth; ++x)
    {
        filter[x] = (float)(x <= filterRadius ? ++i : --i);
    }
    filter[filterWidth] /= (float)pow(2,filterRadius+1);//Normalize divisor
}
void GaussianBlur::blur2D(GLuint inTex, GLuint outTex, glm::uvec2 texDims)
{
    //Update shader config
    this->imageDimensions = texDims;
    blurShader->useProgram();
    GL_CALL(glBindImageTexture(this->inBufferBind, inTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F));
    GL_CALL(glBindImageTexture(this->outBufferBind, outTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F));
    blurShader->launch((texDims / glm::uvec2(16)) + glm::uvec2(1));
    //Synchronise written memory
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
void GaussianBlur::reload()
{
    blurShader->reload();
}