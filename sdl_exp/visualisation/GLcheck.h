#ifndef __GLcheck_h__
#define __GLcheck_h__

#include <GL/glew.h>

//Define EXIT_ON_ERROR to cause the program to exit when a GL error occurs

#ifdef _DEBUG //VS standard debug flag

#include <stdlib.h>
#include <stdio.h>

inline static void HandleGLError(const char *file, int line) {
    GLuint error = glGetError();
    if (error != GL_NO_ERROR)
    {
        printf("%s(%i) GL Error Occurred;\n%s\n", file, line, gluErrorString(error));
#if EXIT_ON_ERROR
        getchar();
        exit(1);
#endif
    }
}

#define GL_CALL( err ) err ;HandleGLError(__FILE__, __LINE__)
#define GL_CHECK() (HandleGLError(__FILE__, __LINE__))

#else //ifdef _DEBUG
//Remove the checks when running release mode.
#define GL_CALL( err ) err
#define GL_CHECK() 

#endif //ifdef  _DEBUG

inline static void InitGlew() {
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", (char *)glewGetErrorString(err));
        getchar();
        exit(1);
    }
}
#define GLEW_INIT() (InitGlew())

#endif //ifndef __GLcheck_h__
