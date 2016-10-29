#include "EntityScene.h"

int main()
{
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
	v.setScene(std::make_unique<EntityScene>(v));
    v.run();

    return 0;
}

