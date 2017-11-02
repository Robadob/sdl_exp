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
    TextureBuffer(const unsigned int elementCount, const unsigned int componentCount = 1, T *data = 0);
#ifdef __CUDACC__
    TextureBuffer(CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation);
#endif
	TextureBuffer(const TextureBuffer<T>& b);
	TextureBuffer(const TextureBuffer<T>&& b) = delete;
	TextureBuffer<T>& operator= (const TextureBuffer<T>& b) = delete;
	TextureBuffer<T>& operator= (const TextureBuffer<T>&& b) = delete;
    ~TextureBuffer();

	void setData(const T *data, size_t size = 0, size_t offset = 0);
	void getData(T *dataReturn, size_t size = 0, size_t offset = 0);
	bool isBound() const override;
private:
	GLuint genTextureUnit();
	static GLenum _getInternalFormat(unsigned int componentCount);
	static GLenum _getFormat(unsigned int componentCount);
	static GLenum _getType();
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