#include "primage/TumourScene.h"
#include "visualisation/multipass/FrameBufferAttachment.h"

int main(int count, char **args)
{
    int sceneId = 0;
    if (count > 1)
        sceneId = atoi(args[1]);
    Visualisation v("Tumour Visualisation", 720, 720);

    v.setScene(std::make_unique<TumourScene>(v, fs::path("C:/Users/rob/Desktop/PRIMAGE/FGPU-Kenneth/gtc")));

    v.run();

    return 0;
}

