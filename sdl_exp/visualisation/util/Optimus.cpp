#include "GLcheck.h"
//This export force enables optimus on laptops with hybrid graphics
//Withtout the export it's necessary to notify the NVidia driver of this executable manually
#ifdef FORCE_OPTIMUS
extern "C" {
    _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
#endif