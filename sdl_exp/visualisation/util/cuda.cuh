#include "../GLcheck.h"
#include <cstring>
#include <typeinfo>   // operator typeid
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

//Ignore the cuda checks if CUDA_CALL isn't defined elsewhere
#ifndef CUDA_CALL
#define CUDA_CALL(fn) fn
#endif
#ifndef GL_CALL
#define GL_CALL(fn) fn
#endif
template <class T>
struct CUDATextureBuffer
{
    CUDATextureBuffer(
        const GLuint glTexName, 
        const GLuint glTBO,
        T *d_mappedPointer, 
        const cudaGraphicsResource_t cuGraphicsRes, 
        cudaTextureObject_t cuTextureObj,
        const unsigned int elementCount,
        const unsigned int componentCount
        )
        : glTexName(glTexName)
        , glTBO(glTBO)
        , d_mappedPointer(d_mappedPointer)
        , cuGraphicsRes(cuGraphicsRes)
        , cuTextureObj(cuTextureObj)
        , elementCount(elementCount)
        , componentCount(componentCount)
    { }
    const GLuint glTexName;
    const GLuint glTBO;
    T *d_mappedPointer;
    const cudaGraphicsResource_t cuGraphicsRes;//These are typedefs over pointers, need to store the actual struct?
    const cudaTextureObject_t cuTextureObj;
    const unsigned int elementCount;
    const unsigned int componentCount;
};

/*
Hidden internal functions
*/
namespace
{
    /*
    Internal function used to detect the required internal format
    @param componentCount The number of components
    @param a A garbage value to specify the desired type because specialised templates werent working
    @return the internal format
    @see https://www.opengl.org/sdk/docs/man/html/glTexBuffer.xhtml
    */
    GLuint _getInternalFormat(const unsigned int componentCount, float a=0)
    {

        if (componentCount == 1) return GL_R32F;
        if (componentCount == 2) return GL_RG32F;
        if (componentCount == 3 || componentCount == 4) return GL_RGBA32F;
        return 0;
    }
    GLuint _getInternalFormat(const unsigned int componentCount, unsigned int a = 0)
    {
        if (componentCount == 1) return GL_R32UI;
        if (componentCount == 2) return GL_RG32UI;
        if (componentCount == 3 || componentCount == 4) return GL_RGBA32UI;
        return 0;
    }
    GLuint _getInternalFormat(const unsigned int componentCount, int a = 0)
    {
        if (componentCount == 1) return GL_R32I;
        if (componentCount == 2) return GL_RG32I;
        if (componentCount == 3 || componentCount == 4) return GL_RGBA32I;
        return 0;
    }
    /*
    @param componentCount The number of components (1-2,4). Passing 3 will be treated as 4
    @param bufferSize The total size of the buffer in bytes
    @param d_TexPointer A device pointer to the mapped texture buffer
    @return The filled cudaResourDesc (memset to 0 if invalid inputs)
    */
    template <class T>
    cudaResourceDesc _getCUDAResourceDesc(const unsigned int componentCount, const unsigned int bufferSize, const T *d_TexPointer)
    {
        cudaResourceDesc resDesc;
        memset(&resDesc, 0, sizeof(cudaResourceDesc));
        //Return empty if invalid input
        if (d_TexPointer == nullptr ||
            bufferSize == 0 ||
            componentCount == 0 ||
            componentCount > 4)
            return resDesc;
        //Linear because its a texture buffer, not a texture
        resDesc.resType = cudaResourceTypeLinear;
        //Mapped pointer to the texture buffer on the device
        resDesc.res.linear.devPtr = (void *)d_TexPointer;
        //The type of the components

        if (typeid(T) == typeid(float))
        {
            resDesc.res.linear.desc.f = cudaChannelFormatKindFloat;
        }
        else if (typeid(T) == typeid(unsigned int))
        {
            resDesc.res.linear.desc.f = cudaChannelFormatKindUnsigned;
        }
        else if (typeid(T) == typeid(int))
        {
            resDesc.res.linear.desc.f = cudaChannelFormatKindSigned;
        }
        //The number of bits per component (0 if not used)
        resDesc.res.linear.desc.x = 32;
        if (componentCount>=2)
        {
            resDesc.res.linear.desc.y = 32;
            if (componentCount >= 3)
            {
                resDesc.res.linear.desc.z = 32;
                resDesc.res.linear.desc.w = 32;
            }
        }
        //The total buffer size
        resDesc.res.linear.sizeInBytes = bufferSize;
        return resDesc;
    }
}
/*
Allocates a GL_TEXTURE_BUFFER of the desired size and binds it for use with CUDA-GL interop
@param texReturn The location of 
@param elementCount The number of elements in the texture buffer
@param componentCount The number of components per element (either 1,2,3 or 4, default 1)
@param componentType The type of the data to be stored in the texture buffer (either GL_FLOAT, GL_INT or GL_UNSIGNED_INT, default GL_FLOAT)
@return The struct storing data related to the generated texture buffer (0 if invalid input)
@see freeGLInteropTextureBuffer(CUDATextureBuffer *)
@see http://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__OPENGL.html#group__CUDART__OPENGL
@see https://www.opengl.org/sdk/docs/man/html/glTexBuffer.xhtml
*/
template <class T>
CUDATextureBuffer<T> *mallocGLInteropTextureBuffer(const unsigned int elementCount, const unsigned int t_componentCount = 1)
{
    if (elementCount==0||
        t_componentCount == 0 ||
        t_componentCount>4)
        return nullptr;
    //Temporary storage of return values
    GLuint glTexName;
    GLuint glTBO;
    T *d_MappedPointer;
    cudaGraphicsResource_t cuGraphicsRes;
    cudaTextureObject_t cuTextureObj;

    //Interpretation of buffer type/component details
    const unsigned int componentCount = t_componentCount == 3 ? 4 : t_componentCount;
    const unsigned int componentSize = sizeof(T);
    const unsigned int elementSize = componentSize*componentCount;
    const unsigned int bufferSize = elementSize * elementCount;
    const GLuint internalFormat = _getInternalFormat(componentCount,(T)0);
   
    //Gen tex
    GL_CALL(glGenTextures(1, &glTexName));
    //Gen buffer
    GL_CALL(glGenBuffers(1, &glTBO));
    //Size buffer and tie to tex
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, glTBO));
    GL_CALL(glBufferData(GL_TEXTURE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW));                                    //TODO dynamic draw better?

    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, glTexName));
    GL_CALL(glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, glTBO));
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, 0));

    //Get CUDA handle to texture
    CUDA_CALL(cudaGraphicsGLRegisterBuffer(&cuGraphicsRes, glTBO, cudaGraphicsMapFlagsNone));
    //Map/convert this to something cuGraphicsRes
    CUDA_CALL(cudaGraphicsMapResources(1, &cuGraphicsRes));
    CUDA_CALL(cudaGraphicsResourceGetMappedPointer((void**)&d_MappedPointer, 0, cuGraphicsRes));
    CUDA_CALL(cudaGraphicsUnmapResources(1, &cuGraphicsRes, 0));
    //Create a texture object from the cuGraphicsRes
    cudaResourceDesc resDesc = _getCUDAResourceDesc(componentCount, bufferSize, d_MappedPointer);
    cudaTextureDesc texDesc;
    memset(&texDesc, 0, sizeof(cudaTextureDesc));
    texDesc.readMode = cudaReadModeElementType;//Read as actual type, other option is normalised float
    //texDesc.addressMode[0] = cudaAddressModeWrap;//We can only affect the address mode for first 3 dimensions, so lets leave it default
    CUDA_CALL(cudaCreateTextureObject(&cuTextureObj, &resDesc, &texDesc, nullptr));
    //Copy the generated data
    return new CUDATextureBuffer<T>(glTexName, glTBO, d_MappedPointer, cuGraphicsRes, cuTextureObj, elementCount, componentCount);
}
/*
Deallocates all data allocated by the matching call to mallocGLInteropTextureBuffer()
@param texBuf The texture buffer to be deallocated
@see mallocGLInteropTextureBuffer(const unsigned int, const unsigned int, const GLuint)
*/
template <class T>
void freeGLInteropTextureBuffer(CUDATextureBuffer<T> *texBuf)
{
    cudaDestroyTextureObject(texBuf->cuTextureObj);
    cudaGraphicsUnregisterResource(texBuf->cuGraphicsRes);
    GL_CALL(glDeleteBuffers(1, &texBuf->glTBO));
    GL_CALL(glDeleteTextures(1, &texBuf->glTexName));
    delete texBuf;
}