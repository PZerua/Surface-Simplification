/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com

	MAIN:
	 + This file creates the window and the app instance. 
	 + It also contains the mainloop
	 + This is the lowest level, here we access the system to create the opengl Context
	 + It takes all the events from SDL and redirect them to the game
*/

#include "includes.h"
#include "application.h"


int main(int argc, char **argv)
{
	//launch the app (app is a global variable)
	Application* app = new Application( "My app", 800, 600 );
	app->init();
	app->start();

	return 0;
}
