#include "EntityScene.h"
#include "TwoPassScene.h"
#include "visualisation/vr/VRExampleScene.h"

int main(int count, char **args)
{
	VisualisationVR v = VisualisationVR("Visulisation Example", 1280, 720);
    v.setScene(std::make_unique<VRExampleScene>(v));
    //int sceneId = 0;
    //if (count > 1)
    //    sceneId = atoi(args[1]);
    //Visualisation v = Visualisation("Visulisation Example", 1280, 720);
    //switch (sceneId)
    //{
    //    case 0:
    //        {
    //            v.setScene(std::make_unique<EntityScene>(v));
    //        }
    //        break;
    //    case 1:
    //    default:
    //        {
    //           v.setScene(std::make_unique<TwoPassScene>(v));
    //        }
    //        break;
    //}
    v.run();

    return 0;
}

