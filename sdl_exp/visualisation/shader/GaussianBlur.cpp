#include "GaussianBlur.h"
#include <glm/gtc/type_ptr.hpp>


GaussianBlur::GaussianBlur(unsigned int filterRadius, float sigma)
	: sigma(sigma)
	, filterRadius(glm::clamp<unsigned int>(filterRadius, 0, 16))
	, filterWidth(2 * glm::clamp<unsigned int>(filterRadius, 0, 16) + 1)
	, blurShader(std::make_shared<ComputeShader>(GAUSSIAN_BLUR_SHADER_PATH))
	, filter(nullptr)
{
	if (filterRadius != this->filterRadius)
		fprintf(stderr, "Gaussian filter radius was clamped from %d to %d.\n", filterRadius, this->filterRadius);
	//For the time being we will allocate ourselves the final 2 image units
	this->inBufferBind = 0;
	this->outBufferBind = 1;
	//Generate the Gaussian filter (manually at current)
	this->filter = (float*)malloc(filterWidth*sizeof(float));
	generateFilter();
	//Copy that to a uniform buffer
	filterBuffer = std::make_shared<UniformBuffer>(filterWidth*sizeof(float), this->filter);
	//Configure uniforms
	blurShader->addBuffer("_filterWeights", this->filterBuffer);
	blurShader->addStaticUniform("_filterRadius", &this->filterRadius);
	blurShader->addStaticUniform("_filterWidth", &this->filterWidth);
	blurShader->addStaticUniform("_imageIn", &this->inBufferBind);
	blurShader->addStaticUniform("_imageOut", &this->outBufferBind);
	blurShader->addDynamicUniform("_imageDimensions", glm::value_ptr(this->imageDimensions), 2);
}
GaussianBlur::~GaussianBlur()
{
	this->filterBuffer.reset();
	free(filter);
}
void GaussianBlur::generateFilter()
{//http://www.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
	//Count filter, so it can be normalised
	float c = 0.0f;
	//Fill filter without sigma
	//unsigned int i = 0;
	//for (unsigned int x = 0; x < filterWidth; ++x)
	//{
	//    filter[x] = (float)(x <= filterRadius ? ++i : --i);
	//    c += filter[x];
	//}
	//Fill filter with sigma
	int i = filterRadius;
	for (unsigned int x = 0; x < filterWidth; ++x)
	{
		float _j = (float)(x < filterRadius ? i-- : i++);
		filter[x] = (1.0f / (sqrt(2.0f*glm::pi<float>())*this->sigma))*glm::exp(-(_j*_j) / (2 * this->sigma*this->sigma));
		c += filter[x];
	}
	//Normalise filter
	for (unsigned int x = 0; x < filterWidth; ++x)
	{
		filter[x] /= c;
	}
}
void GaussianBlur::blurR32F(GLuint inTex, GLuint outTex, glm::uvec2 texDims)
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