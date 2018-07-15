#include "Texture2D_Multisample.h"


const char *Texture2D_Multisample::RAW_TEXTURE_FLAG = "Texture2D_Multisample";

std::shared_ptr<Texture2D_Multisample> Texture2D_Multisample::make(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const void *data, const unsigned long long &options)
{
    return std::shared_ptr<Texture2D_Multisample>(new Texture2D_Multisample(dimensions, format, samples, data, options));
}
std::shared_ptr<Texture2D_Multisample> Texture2D_Multisample::make(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const unsigned long long &options)
{
    return std::shared_ptr<Texture2D_Multisample>(new Texture2D_Multisample(dimensions, format, samples, nullptr, options));
}
Texture2D_Multisample::Texture2D_Multisample(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const void *data, const unsigned long long &options)
    :Texture(GL_TEXTURE_2D_MULTISAMPLE, genTextureUnit(), format, RAW_TEXTURE_FLAG, options)
    , dimensions(dimensions)
    , samples(samples)
{

    allocateMultisampleTextureMutable(dimensions, samples);
    applyOptions();
}
void Texture2D_Multisample::resize(const glm::uvec2 &dimensions, unsigned int samples)
{
    this->dimensions = dimensions;
    this->samples = samples ? samples : this->samples;
    allocateMultisampleTextureMutable(this->dimensions, this->samples);
}
void Texture2D_Multisample::allocateMultisampleTextureMutable(const glm::uvec2 &dimensions, unsigned int samples)
{

    GL_CALL(glTexImage2DMultisample(type, samples, format.internalFormat, dimensions.x, dimensions.y, true));
}
/**
* Required methods for handling texture units
*/
GLuint Texture2D_Multisample::genTextureUnit()
{
    static GLuint texUnit = 1;
    GLint maxUnits;
    GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));//192 on Modern GPUs, spec minimum 80
#ifdef _DEBUG
    assert(texUnit < (GLuint)maxUnits);
#endif
    if (texUnit >= (GLuint)maxUnits)
    {
        texUnit = 1;
        fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_2D_MULTISAMPLE, enable texture switching.\n");
        //If we ever notice this being triggered, need to add a static flag to Shaders which tells it to rebind textures to units at use.
        //Possibly even notifying it of duplicate units
    }
    return texUnit++;
}
bool Texture2D_Multisample::isBound() const
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + textureUnit));
    GLint whichID;
    GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &whichID));
    return whichID == glName;
}
glm::uvec2 Texture2D_Multisample::getDimensions(const GLuint &texName)
{
    glm::ivec2 rtn(0);
    GL_CALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texName));
    GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_WIDTH, &rtn.x));
    GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_HEIGHT, &rtn.y));
    GL_CALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));
    return rtn;
}