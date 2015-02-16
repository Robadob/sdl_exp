#pragma once

#include "GL\glew.h"

class Texture
{
public:
	Texture();
	~Texture();

	void createTextureBufferObject(GLuint *tbo, GLuint *texture, GLuint size);
	void deleteTextureBufferObject(GLuint *tbo);
	void checkGLError();
};

