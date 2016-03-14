#include "EntityScene.h"

int main()
{
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
    EntityScene *scene = new EntityScene(v);
    
    v.run();

    return 0;
}

