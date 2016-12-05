#include "GaussianBlur.h"
#include <glm/gtc/type_ptr.hpp>


GaussianBlur::GaussianBlur(unsigned int filterRadius, float sigma)
    : sigma(sigma)
    , filterRadius(filterRadius)
    , filterWidth(2*filterRadius+1)
    , blurShader(std::make_shared<ComputeShader>(GAUSSIAN_BLUR_SHADER_PATH))
    , filter(nullptr)
{
    //For the time being we will allocate ourselves the final 2 texture units
    GLint maxTex;
    GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTex));
    this->inBufferBind = --maxTex;
    this->outBufferBind = --maxTex;
    //Generate the Gaussian filter (manually at current)
    this->filter = (float*)malloc(filterWidth*sizeof(float));
    generateFilter();
    //Copy that to a uniform buffer
    GL_CALL(glGenBuffers(1, &this->filterBuffer));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, this->filterBuffer));
    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, filterWidth*sizeof(float), this->filter, GL_STATIC_READ));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    //Configure uniforms
    blurShader->addBuffer("_filterWeights", GL_UNIFORM_BUFFER, this->filterBuffer);
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
        filter[x] = x <= filterRadius ? ++i : --i;
        //filter[x] /= pow(2,filterRadius+1);//Normalize
    }
}
void GaussianBlur::blur2D(GLuint inTex, GLuint outTex, glm::uvec2 texDims)
{
    //Update shader config
    this->imageDimensions = texDims;
    //Bind textures (manually atm)
    GL_CALL(glActiveTexture(GL_TEXTURE0 + this->inBufferBind));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, inTex));
    GL_CALL(glActiveTexture(GL_TEXTURE0 + this->outBufferBind));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, outTex));
    //Launch shader
    blurShader->launch();
    //Synchronise written memory
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}