#pragma once

#include <stdio.h>
#include <gl/glew.h>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/gtc/type_ptr.hpp"

#include "VisualisationScene.h"
#include "Camera.h"
#include "Axis.h"
#include "Skybox.h"

#undef main

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

class Visualisation
{
public:
    Visualisation(char* windowTitle, int windowWidth = DEFAULT_WINDOW_WIDTH, int windowHeight = DEFAULT_WINDOW_HEIGHT);
    ~Visualisation();

    bool init();
    void handleKeypress(SDL_Keycode keycode, int x, int y);
    void close();
    void run(); // @todo - improve

    char* getWindowTitle();
    void setWindowTitle(char* windowTitle);
    
    void setQuit(bool quit);
    void toggleFullScreen();
    void toggleMouseMode();
    void resizeWindow();
    void handleMouseMove(int x, int y);
    bool isFullscreen();
    void updateFPS();

    void defaultProjection();
    void defaultLighting();
    void clearFrame();
    void renderAxis();
    void setRenderAxis(bool state);
    void setMSAA(bool state);
    Camera *getCamera();
    VisualisationScene *getScene() const;

private:
    SDL_Window* window;
    SDL_GLContext context;
    Camera camera;
    VisualisationScene* scene;
    glm::mat4 frustum;

    bool isInitialised;
    bool quit;

    bool renderAxisState;
    bool msaaState;
    Axis axis;
    Skybox *skybox;

    char* windowTitle;
    int windowWidth;
    int windowHeight;
    
    SDL_Rect windowedBounds;

    unsigned int previousTime = 0;
    unsigned int currentTime;
    unsigned int frameCount = 0;

};

