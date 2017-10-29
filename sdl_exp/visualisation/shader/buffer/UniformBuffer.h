#ifndef __UniformBuffer_h__
#define __UniformBuffer_h__
#include <GL/glew.h>
#include <set>
#include "BufferCore.h"

/**
 * Representative of GL_UNIFORM_BUFFER
 * These can be used for reading from during shader execution
 * Particularly useful for passing light/material info or arrays of transformation matrices
 * The spec requires a max size of atleast 16KB (likely much bigger, e.g. 65KB on modern NV hardware)
 */
class UniformBuffer : public BufferCore
{
public:
	UniformBuffer(size_t bytes, void* data = nullptr);
	~UniformBuffer();
	static GLint MaxSize();
	static GLint MaxBuffers();
private:
	GLint allocateBindPoint();
	static std::set<GLint> allocatedBindPoints;
};

#endif //__UniformBuffer_h__