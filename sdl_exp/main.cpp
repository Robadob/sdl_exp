#include "visualisation/Visualisation.h"

int main()
{
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
    v.setRenderAxis(true);
    v.run();

    return 0;
}

