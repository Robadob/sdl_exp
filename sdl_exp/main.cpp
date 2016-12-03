#include "EntityScene.h"
#include "TwoPassScene.h"
#include "visualisation/multipass/FrameBufferAttachment.h"

int main()
{
    Visualisation v = Visualisation("Visulisation Example", 1280, 720);
	v.setScene(std::make_unique<TwoPassScene>(v));
    v.run();

    return 0;
}

