#ifndef __Texture2D_Multisample_h__
#define __Texture2D_Multisample_h__
#include "Texture.h"

class Texture2D_Multisample : public Texture
{
public:
    static std::shared_ptr<Texture2D_Multisample> make(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const void *data = nullptr, const unsigned long long &options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);
    static std::shared_ptr<Texture2D_Multisample> make(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const unsigned long long &options);

    //Cant fill multi sample texture from host so don't allow copy (can be done with compute shader?)
    Texture2D_Multisample(const Texture2D_Multisample& b) = delete;
    Texture2D_Multisample(const Texture2D_Multisample&& b) = delete;
    Texture2D_Multisample& operator= (const Texture2D_Multisample& b) = delete;
    Texture2D_Multisample& operator= (const Texture2D_Multisample&& b) = delete;
    void resize(const glm::uvec2 &dimensions, unsigned int samples = 0);
    glm::uvec2 getDimensions() const { return dimensions; }
    unsigned int getWidth() const { return dimensions.x; }
    unsigned int getHeight() const { return dimensions.y; }
    bool isBound() const override;
private:
    Texture2D_Multisample(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples=4, const void *data = nullptr, const unsigned long long &options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);
    void allocateMultisampleTextureMutable(const glm::uvec2 &dimensions, unsigned int samples = 0);
    static GLuint genTextureUnit();
    glm::uvec2 dimensions;
    static const char *RAW_TEXTURE_FLAG;
    unsigned int samples;
};

#endif //__Texture2D_Multisample_h__
