#include "TextureBuffer.h"

/*
Creates the texture buffer
@param uniformName Name of the uniform to be used
@param elementCount The number of elements to be stored in the buffer
@param componentCount The number of components per element (1-4)
*/
template<class T>
TextureBuffer<T>::TextureBuffer(char *uniformName, const unsigned int elementCount, const unsigned int componentCount, T *data)
    : Texture(GL_TEXTURE_BUFFER, nullptr, uniformName)
    , elementCount(elementCount)
    , componentCount(componentCount)
#ifdef __CUDACC__
    , cuTexBuf(0)
    , handleDeallocation(true)
#endif
{//Gen tex
    GL_CALL(glGenTextures(1, &texName));
    //Gen buffer
    GL_CALL(glGenBuffers(1, &TBO));
    //Size buffer and tie to tex
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, TBO));
    GL_CALL(glBufferData(GL_TEXTURE_BUFFER, sizeof(T)*elementCount * componentCount, (void*)data, GL_STATIC_DRAW));//TODO dynamic draw better?

    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, texName));
    GL_CALL(glTexBuffer(GL_TEXTURE_BUFFER, _getInternalFormat(), TBO));
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, 0));
}

#ifdef __CUDACC__
/*
Creates the texture buffer from a preallocated CUDATextureBuffer
@param uniformName Name of the uniform to be used
@param cuTexBuf The pointer to the CUDATextureBuffer
@param handleDeallocation If true, at destruction of this object the CUDATextureBuffer will be deallocated
*/
template<class T>
TextureBuffer<T>::TextureBuffer(char *uniformName, CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation)
    : Texture(GL_TEXTURE_BUFFER, 0, uniformName)
    , cuTexBuf(cuTexBuf)
    , handleDeallocation(handleDeallocation)
    , elementCount(cuTexBuf->elementCount)
    , componentCount(cuTexBuf->componentCount)
    , TBO(cuTexBuf->glTBO)
{
    texName = cuTexBuf->glTexName;
}
#endif
/*
Generic destructor, deallocates the texture buffer if required
*/
template<class T>
TextureBuffer<T>::~TextureBuffer(){

#ifdef __CUDACC__
    if (cuTexBuf)
    {
        if (handleDeallocation)
        {
            freeGLInteropTextureBuffer(cuTexBuf);
        }
    }
    else
#endif
    {
        //Deallocate the buffer object
        GL_CALL(glDeleteBuffers(1, &TBO));
        TBO = 0;
        //Deallocate the texture
        //(do this because I don't trust the super destructor to be called due to apparent 'hiding')
        GL_CALL(glDeleteTextures(1, &texName));
        texName = 0;
    }
}
/*
Copies data into the texture buffer
@param data Pointer to the source data to be copied
@param size The number of bytes to be copied. If 0 this defaults to the size of the buffer
@param offset The byte offset into the buffer. Defaults to 0
*/
template<class T>
void TextureBuffer<T>::setData(T *data, unsigned int size, unsigned int offset){
    if (size == 0)
        size = sizeof(T)*elementCount*componentCount;
    GL_CALL(glNamedBufferSubData(TBO, offset, size, (void*)data));
}
/*
Copies data out of the texture buffer
@param dataReturn Pointer to the destination that data will be copied
@param size The number of bytes to be copied. If 0 this defaults to the size of the buffer
@param offset The byte offset into the buffer. Defaults to 0
*/
template<class T>
void TextureBuffer<T>::getData(T *dataReturn, unsigned int size, unsigned int offset){
    if (size == 0)
        size = sizeof(T)*elementCount*componentCount;
    GL_CALL(glGetNamedBufferSubData(TBO, offset, size, (void*)dataReturn));
}
/*
Returns the internal format of a float buffer based on the componentCount
@return The internal format of the element
*/
template<>
GLuint TextureBuffer<float>::_getInternalFormat() const {
    if (componentCount == 1) return GL_R32F;
    if (componentCount == 2) return GL_RG32F;
    if (componentCount == 3) return GL_RGB32F;
    if (componentCount == 4) return GL_RGBA32F;
    return 0;
}
/*
Returns the internal format of an int buffer based on the componentCount
@return The internal format of the element
*/
template<>
GLuint TextureBuffer<unsigned int>::_getInternalFormat() const {
    if (componentCount == 1) return GL_R32UI;
    if (componentCount == 2) return GL_RG32UI;
    if (componentCount == 3) return GL_RGB32UI;
    if (componentCount == 4) return GL_RGBA32UI;
    return 0;
}
/*
Returns the internal format of an unsigned int buffer based on the componentCount
@return The internal format of the element
*/
template<>
GLuint TextureBuffer<int>::_getInternalFormat() const {
    if (componentCount == 1) return GL_R32I;
    if (componentCount == 2) return GL_RG32I;
    if (componentCount == 3) return GL_RGB32I;
    if (componentCount == 4) return GL_RGBA32I;
    return 0;
}

//We only want this class to be instantiated as float, unsigned int or int
template class TextureBuffer < float >;
template class TextureBuffer < unsigned int >;
template class TextureBuffer < int >; 