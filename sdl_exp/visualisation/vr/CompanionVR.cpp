#include "CompanionVR.h"

CompanionVR::CompanionVR()
    :Overlay(std::make_shared<Shaders>(Stock::Shaders::SPRITE2D))
{
    //Generate 2D frames to render eyes to here
    //See SetupCompanionWindow()
    //See Sprite2D
}
