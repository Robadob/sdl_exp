#SDL_EXP
A simple template implementation using SDL for quickly applying the modern OpenGL pipeline to projects.

##Usage
The easiest way to utilise this library appears to be:
* Clone the contents of the `include` directory into your own include directory's root.
* Clone the contents of the platform relevant `lib\Win32` or `lib\x64` directory into your relevant lib directories.
* Clone the platform relevant runtime libraries (.dll) from `Debug\Win32` or `Debug\x64` ino your relevant build directories.
* Update the linker to link against:
  * `SDL2.lib`
  * `SDL2_image.lib`
  * `glew32.lib`
  * `OpenGL32.lib`
  * `glu32.lib`
* Delete the sample `main.cpp`, to remove the duplicate entry point.
* Copy the `sdl_exp/visualisation` directory into your own source directory.
* Create a new class which extends `Scene` and overrides all of its virtual methods 
  * `EntityScene.h` & `EntityScene.cpp` are provided as an example of how to configure your own `Scene`.
  * Make sure to call the constructor of Scene, as this will automatically manage your Projection and ModelView matrices!
* Within your main method you can now create the Visualisation:
```C++
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
    EntityScene *scene = new EntityScene(v);
    v.run();
```

###Shader Uniforms & Attributes
The `Shaders` and `Entity` objects attempt to automatically manage uniforms and attributes, you can assist their functioning by using the below naming schemes for your uniforms and vertex attributes. You can also configure your own static and dynamic uniform floats and ints by calling `addStaticUniform()` and `addDynamicUniform()` on the relevant `Shaders` object.

* Uniforms:
  * `_modelViewMat` - ModelView Matrix[Mat4]
  * `_projectionMat` - Projection Matrix[Mat4]
  * `_modelViewProjectionMat` - ModelViewProjection Matrix[Mat4]
  * `_texture` - Texture Sampler[sampler2D/samplerCube]
* Atributes:
  * `_vertex` - Vertex Position[Vec3/Vec4]
  * `_normal` - Vertex Normal[Vec3/Vec4]
  * `_color` - Vertex Color[Vec3/Vec4]
  * `_texCoords` - Texture Coordinates[Vec2/Vec3]

###CUDA-GL interop
The header `visualisation/util/cuda.cuh` provides functionality for allocating and freeing OpenGL texture buffers that can be accessed by CUDA. 
If you wish to write to these textures asynchronously of their accesses by shaders it is recommended you use some form of locking.
The header was initially written for use with the CUDA 7.5 API.

Usage:
* Create a `CUDATextureBuffer` using `mallocGLInteropTextureBuffer()`
* You can now access the texture buffer using CUDA:
  * You can use the `d_mappedPointer` member variable to read/write from the buffer as you would with any normal device pointer.
  * Within CUDA kernels you can read the texture buffers using  the [texture object API texture fetch functions](http://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#texture-object-api-appendix) (e.g. `tex1Dfetch<float>()`) passing the `cuTextureObj` member variable as the first argument.
    * Copy the `cuTextureObj` member variable to a device constant for convenience!
  * Write a vertex shader:
    * *The existing shader file `instanced.vert` provides an example.*
	* *`i` and `u` can be prepended to `samplerBuffer`, `texelFetch()` and `vec4` in the following steps to work with `int` and `unsigned int` data instead of the defaults of `float`.*
	* Declare a `uniform samplerBuffer` sampler inside your shader.
	* To access an element of the texture buffer use `texelFetch()`, passing the identifer of your `samplerBuffer` as the first argument, and the desired index as the second argument.
	  * This returns a `vec4`, if your element has less than 4 components ignore those you do not require.
	  * If you are performing instanced rendering, the second argument is likely `gl_InstanceID`.  
    * If you are writing a vertex shader using a version earlier than 140 you must follow some additional steps:
	  * Add `#extension GL_EXT_gpu_shader4 : require` to the shader on the line directly after `#version`.
        * This enables the extension for reading data from the texture buffer.
      * Replace all call calls to `texelFetch()`, `itexelFetch()` and `utexelFetch()` with calls to `texelFetchBuffer()`. This function returns the right type of `vec4`, based on the `samplerBuffer` type.
  * Create a `Shaders` object from your vertex shader source:
    * Call `addTextureUniform()` on your `Shaders` object:
        * The first argument should be the `glTexName` member variable.
        * The second argument should be the identifier of the `uniform sampleBuffer` within your vertex shader source.
        * The third argument should be `GL_TEXTURE_BUFFER`.
  * Pass your `Shaders` object to an `Entity` and render!
  * After you have finished using the texture buffer with both OpenGL and CUDA, pass the `CUDATextureBuffer` to `freeGLInteropTextureBuffer(CUDATextureBuffer *texBuf)`. This will deallocate the texture buffer and delete the `CUDATextureBuffer` struct.

Additional Usage:
* To access the texture buffer using OpenGL:
  * Before doing anything with the texture buffer in OpenGL you should bind the buffer using [`glBindBuffer()`](https://www.opengl.org/sdk/docs/man/html/glBindBuffer.xhtml) passing the `glTBO` member variable as the argument.
    * The `glTBO` member variable is the buffers 'name', you can use this with various OpenGL buffer methods.
  * To copy data to the texture buffer using OpenGL you can use [`glBufferData()`](https://www.opengl.org/sdk/docs/man/html/glBufferData.xhtml) passing `GL_TEXTURE_BUFFER` as the first argument.
  * It is not possible to copy data back from a texture buffer using OpenGL.
  * You can clear the active buffer by calling `glBindBuffer(0)`.
* To use the texture buffer when manually rendering with OpenGL:
  * After you have linked the shader program:
    * You must set the value of the `uniform samplerBuffer` within the shader, to the texture unit into which you will load the desired texture buffer.
      * This should be any value from 0 to that returned by `glGet(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)-1` (remember this value, it must be reused at render). In most cases you can simply increment this value from 0 for each unique texture, texture units can be reused across seperate render calls per frame.
      * You can use [`glUniform1i()`](https://www.opengl.org/sdk/docs/man/html/glUniform.xhtml) to set this value within the shader, where the second parameter can be gained using [glGetUniformLocation()](https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glGetUniformLocation.xml) if not stated within the vertex shader source.
  * Within your render loop:
    * Activate the texture unit using [`glActiveTexture()`](https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glActiveTexture.xml)
      * The argument passed to this should be `GL_TEXTURE0` + the value you earlier stored in the shaders `uniform samplerBuffer` 
    * Bind the texture buffer to the texture unit using [`glBindTexture()`](https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glBindTexture.xml) passing `GL_TEXTURE_BUFFER` as the first argument, and the `glTexName` member variable as the second argument.
    * After the render call use `glActiveTexture()` again, followed by `glBindBuffer(0)` to clear the texture unit.

##Dependencies
All dependent libraries are included within the repo, licenses are available on their relevant websites.

* [SDL](https://www.libsdl.org/) 2.4.0
* [SDL_image](https://www.libsdl.org/projects/SDL_image/) 2.0.1 *(for texture loading)*
* [GLM](http://glm.g-truc.net/) 0.9.7.3 *(for consistent C/GLS vector maths functionality)*
* [GLEW](http://glew.sourceforge.net/) 1.13.0
