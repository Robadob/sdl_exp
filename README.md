#SDL_EXP
A simple template implementation using SDL for quickly applying the modern OpenGL pipeline to projects.

##Usage
The easiest way to utilise this library appears to be;
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
* Within your main method you can now create the Visualisation;
```C++
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
    EntityScene *scene = new EntityScene(v);
    v.run();
```

##Note
The `Shaders` and `Entity` objects attempt to automatically manage uniforms and attributes, you can assist their functioning by using the below naming schemes for your matrix uniforms and virtex attributes.
* `_modelViewMat` - ModelView Matrix[Mat4]
* `_projectionMat` - Projection Matrix[Mat4]
* `_vertex` - Vertex Position[Vec3/Vec4]
* `_normal` - Vertex Normal[Vec3/Vec4]
* `_color` - Vertex Color[Vec3/Vec4]

##Dependencies
All dependent libraries are included within the repo, licenses are available on their relevant websites.

* [SDL](https://www.libsdl.org/) 2.4.0
* [SDL_image](https://www.libsdl.org/projects/SDL_image/) 2.0.1 *(for texture loading)*
* [GLM](http://glm.g-truc.net/) 0.9.7.3 *(for consistent C/GLS vector maths functionality)*
* [GLEW](http://glew.sourceforge.net/) 1.13.0