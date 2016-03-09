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
* Copy the .h and .cpp files from `sdl_exp` into a named directory within your own source (e.g. `sdl_exp`).
* Where required, include e.g `sdl_exp/Visualisation.h`.
* Declare a `Visualisation` object, and call the `run()` method.

##Dependencies
All dependent libraries are included within the repo, licenses are available on their relevant websites.

* [SDL](https://www.libsdl.org/) 2.4.0
* [SDL_image](https://www.libsdl.org/projects/SDL_image/) 2.0.1 *(for texture loading)*
* [GLM](http://glm.g-truc.net/) 0.9.7.3 *(for consistent C/GLS vector maths functionality)*
* [GLEW](http://glew.sourceforge.net/) 1.13.0