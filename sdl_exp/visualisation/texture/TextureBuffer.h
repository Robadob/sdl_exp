#ifndef __TexturBuffer_h__
#define __TexturBuffer_h__
#include "Texture.h"
#ifdef __CUDACC__
#include "../util/cuda.cuh"
#endif

/**
 * This class represents a GL_TEXTURE_BUFFER
 * It contains optional CUDA support if necessary files are renamed from .cpp.cu to .cu and set to build with CUDA
 */
template<class T>
class TextureBuffer : public Texture
{
public:
	/**
	 * Creates the texture buffer
	 * @param elementCount The number of elements to be stored in the buffer
	 * @param componentCount The number of components per element (1-4)
	 */
	static std::shared_ptr<TextureBuffer<T>> make(const unsigned int elementCount, const unsigned int componentCount = 1, T *data = nullptr);
#ifdef __CUDACC__
	/**
	 * Creates a handle to the texture buffer from a preallocated CUDATextureBuffer
	 * @param cuTexBuf The pointer to the CUDATextureBuffer
	 * @param handleDeallocation If true, at destruction of this object the CUDATextureBuffer will be deallocated
	 */
	static std::shared_ptr<TextureBuffer<T>> make(CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation);
#endif
	TextureBuffer(const TextureBuffer<T>& b);
	TextureBuffer(const TextureBuffer<T>&& b) = delete;
	TextureBuffer<T>& operator= (const TextureBuffer<T>& b) = delete;
	TextureBuffer<T>& operator= (const TextureBuffer<T>&& b) = delete;
    ~TextureBuffer();	
	/**
	 * Copies data into the texture buffer
	 * @param data Pointer to the source data to be copied
	 * @param size The number of bytes to be copied. If 0 this defaults to the size of the buffer
	 * @param offset The byte offset into the buffer. Defaults to 0
	 */
	void setData(const T *data, size_t size = 0, size_t offset = 0);
	/**
	 * Copies data out of the texture buffer
	 * @param dataReturn Pointer to the destination that data will be copied
	 * @param size The number of bytes to be copied. If 0 this defaults to the size of the buffer
	 * @param offset The byte offset into the buffer. Defaults to 0
	 */
	void getData(T *dataReturn, size_t size = 0, size_t offset = 0);
	/**
	 * Returns whether this is the currently bound texture
	 * @note This does not check whether it is the currently bound buffer!
	 */
	bool isBound() const override;
private:
	/**
	 * Private constructor
	 * @see make(const unsigned int, const unsigned int, T*)
	 */
	TextureBuffer(const unsigned int elementCount, const unsigned int componentCount = 1, T *data = nullptr);
#ifdef __CUDACC__
	/**
	 * Private constructor
	 * @see make(CUDATextureBuffer<T>, bool)
	 */
	TextureBuffer(CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation);
#endif
	/**
	 * Required methods for handling texture units
	 */
	static GLuint genTextureUnit();
	/**
	 * Returns the internal format of a buffer based on the componentCount (and template arg)
	 * @return The internal format of each texel (element)
	 */
	static GLenum _getInternalFormat(unsigned int componentCount);
	/**
	 * Returns the format of a buffer based on the componentCount (and template arg)
	 * @return The format of each texel (element)
	 */
	static GLenum _getFormat(unsigned int componentCount);
	/**
	 * Returns the storage type of a buffer based on the template arg
	 * @return The stprage type of each texel (element)
	 */
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