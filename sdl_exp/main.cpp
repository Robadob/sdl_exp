
#include "Visualisation.h";
#include <stdio.h>
#include "SDL/SDL.h"
#undef main

int main()
{
	Visualisation v = Visualisation("Visulisation Example", 1280, 720);
	v.run();

	return 0;
}

