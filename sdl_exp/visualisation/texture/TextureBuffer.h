#ifndef __TexturBuffer_h__
#define __TexturBuffer_h__
#include "Texture.h"
#ifdef __CUDACC__
#include "../util/cuda.cuh"
#endif

/*
This class represents a GL_TEXTURE_BUFFER
*/
template<class T>
class TextureBuffer : public Texture
{
public:
    TextureBuffer(char *uniformName, const unsigned int elementCount, const unsigned int componentCount = 1, T *data = 0);
#ifdef __CUDACC__
    TextureBuffer(char *uniformName, CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation);
#endif
    ~TextureBuffer();

    void setData(T *data, unsigned int size=0, unsigned int offset=0); 
    void getData(T *dataReturn, unsigned int size=0, unsigned int offset=0);
    void reload() override { };
private:
    GLuint _getInternalFormat() const;
    //GL TexBuf
    const unsigned int elementCount;
    const unsigned int componentCount;
    GLuint TBO;
#ifdef __CUDACC__
    //Cuda TexBuf
    CUDATextureBuffer<T> *cuTexBuf;
    const bool handleDeallocation;
#endif
};

#endif //ifndef __TexturBuffer_h__