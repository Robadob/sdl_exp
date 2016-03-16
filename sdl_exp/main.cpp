#include "EntityScene.h"

int main()
{
    //const char *colorModelPath = "C:/Users/Robadob/Desktop/rothwell-wy-1.obj";
    //Entity e(colorModelPath);
    //printf("fin");
    //getchar();
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
    EntityScene *scene = new EntityScene(v);
    
    v.run();

    return 0;
}

