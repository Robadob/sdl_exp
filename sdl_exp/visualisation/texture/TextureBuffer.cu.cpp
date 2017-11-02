#include "TextureBuffer.h"
#include <cassert>

/**
 * Constructors
 */
template<class T>
TextureBuffer<T>::TextureBuffer(const unsigned int elementCount, const unsigned int componentCount, T *data)
	: Texture(GL_TEXTURE_BUFFER, genTextureUnit(), Format(_getFormat(componentCount), _getInternalFormat(componentCount), componentCount*sizeof(T), _getType()), "TextureBuffer", 0)
    , elementCount(elementCount)
    , componentCount(componentCount)
#ifdef __CUDACC__
    , cuTexBuf(0)
    , handleDeallocation(true)
#endif
{
    //Gen buffer
    GL_CALL(glGenBuffers(1, &TBO));
    //Size buffer and tie to tex
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, TBO));
    GL_CALL(glBufferData(GL_TEXTURE_BUFFER, format.pixelSize*elementCount, (void*)data, GL_STATIC_DRAW));
    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, glName));
	GL_CALL(glTexBuffer(GL_TEXTURE_BUFFER, _getInternalFormat(componentCount), TBO));
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, 0));
}
template<class T>
TextureBuffer<T>::TextureBuffer(const TextureBuffer<T>& b)
	: Texture(GL_TEXTURE_BUFFER, genTextureUnit(), b.format, "TextureBuffer", 0)
	, elementCount(b.elementCount)
	, componentCount(b.componentCount)
#ifdef __CUDACC__
	, cuTexBuf(0)
	, handleDeallocation(true)
#endif
{
	//Gen buffer
	GL_CALL(glGenBuffers(1, &TBO));
	//Get buffer data
	size_t bufSize = format.pixelSize*elementCount;
	void * bufData = malloc(bufSize);
	b.getData(bufData, bufSize);
	//Bind new buffer to new texture
	GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, TBO));
	GL_CALL(glBufferData(GL_TEXTURE_BUFFER, bufSize, bufData, GL_STATIC_DRAW));
	GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, glName));
	GL_CALL(glTexBuffer(GL_TEXTURE_BUFFER, _getInternalFormat(componentCount), TBO));
	GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
	GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, 0));
	//Free buffer data
	free(bufData);	
}
#ifdef __CUDACC__
template<class T>
TextureBuffer<T>::TextureBuffer(CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation)
	: Texture(GL_TEXTURE_BUFFER, genTextureUnit(), Format(_getFormat(cuTexBuf->componentCount), _getInternalFormat(cuTexBuf->componentCount)), "TextureBuffer", 0, cuTexBuf->glTexName)
    , cuTexBuf(cuTexBuf)
    , handleDeallocation(handleDeallocation)
    , elementCount(cuTexBuf->elementCount)
    , componentCount(cuTexBuf->componentCount)
    , TBO(cuTexBuf->glTBO)
{ }
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
    }
}
/**
 * Factories
 */
template<class T>
std::shared_ptr<TextureBuffer<T>> TextureBuffer<T>::make(const unsigned int elementCount, const unsigned int componentCount, T *data)
{
	return std::make_shared<TextureBuffer<T>>(elementCount, componentCount, data);
}

#ifdef __CUDACC__
template<class T>
std::shared_ptr<TextureBuffer<T>> TextureBuffer<T>::make(CUDATextureBuffer<T> *cuTexBuf, bool handleDeallocation)
{
	return std::make_shared<TextureBuffer<T>>(cuTexBuf, handleDeallocation);
}
#endif
/**
 * Buffer accessors
 */
template<class T>
void TextureBuffer<T>::setData(const T *data, size_t size, size_t offset){
    if (size == 0)
		size = format.pixelSize*elementCount;
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, TBO));
    GL_CALL(glBufferSubData(GL_TEXTURE_BUFFER, offset, size, (void*)data));
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
    //GL_CALL(glNamedBufferSubData(TBO, offset, size, (void*)data));//GL4.5+
}
template<class T>
void TextureBuffer<T>::getData(T *dataReturn, size_t size, size_t offset){
    if (size == 0)
		size = format.pixelSize*elementCount;
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, TBO));
    GL_CALL(glGetBufferSubData(GL_TEXTURE_BUFFER, offset, size, (void*)dataReturn));
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
}
/**
 * Static enum calculators
 */
template<>
GLenum TextureBuffer<float>::_getInternalFormat(unsigned int componentCount) {
    if (componentCount == 1) return GL_R32F;
    if (componentCount == 2) return GL_RG32F;
    if (componentCount == 3) return GL_RGB32F;
    if (componentCount == 4) return GL_RGBA32F;
    return 0;
}
template<>
GLenum TextureBuffer<unsigned int>::_getInternalFormat(unsigned int componentCount) {
    if (componentCount == 1) return GL_R32UI;
    if (componentCount == 2) return GL_RG32UI;
    if (componentCount == 3) return GL_RGB32UI;
    if (componentCount == 4) return GL_RGBA32UI;
    return 0;
}
template<>
GLenum TextureBuffer<int>::_getInternalFormat(unsigned int componentCount) {
    if (componentCount == 1) return GL_R32I;
    if (componentCount == 2) return GL_RG32I;
    if (componentCount == 3) return GL_RGB32I;
    if (componentCount == 4) return GL_RGBA32I;
    return 0;
}
template<>
GLenum TextureBuffer<float>::_getType() {
	return GL_FLOAT;
}
template<>
GLenum TextureBuffer<unsigned int>::_getType() {
	return GL_UNSIGNED_INT;
}
template<>
GLenum TextureBuffer<int>::_getType() {
	return GL_INT;
}
template<class T>
GLenum TextureBuffer<T>::_getFormat(unsigned int componentCount) {
	if (componentCount == 1) return GL_RED;
	if (componentCount == 2) return GL_RG;
	if (componentCount == 3) return GL_RGB;
	if (componentCount == 4) return GL_RGBA;
	return 0;
}
/**
 * Util
 */
template<class T>
GLuint TextureBuffer<T>::genTextureUnit()
{
	static GLuint texUnit = 1;
	GLint maxUnits;
	GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));//192 on Modern GPUs, spec minimum 80
#ifdef _DEBUG
	assert(texUnit < (GLuint)maxUnits);
#endif
	if (texUnit < (GLuint)maxUnits)
	{
		texUnit = 1;
		fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_2D, enable texture switching");
		//If we ever notice this being triggered, need to add a static flag to Shaders which tells it to rebind textures to units at use.
		//Possibly even notifying it of duplicate units
	}
	return texUnit++;
}
template<class T>
bool TextureBuffer<T>::isBound() const
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + textureUnit));
	GLint whichID;
	GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID));
	return whichID == glName;
}

//We only want this class to be instantiated as float, unsigned int or int
template class TextureBuffer < float >;
template class TextureBuffer < unsigned int >;
template class TextureBuffer < int >; 