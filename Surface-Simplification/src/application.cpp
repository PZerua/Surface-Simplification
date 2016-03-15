#include "application.h"
#include "utils.h"
#include "image.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"

Camera* camera = NULL;
Mesh* mesh = NULL;
Matrix44 model_matrix;
Shader* phong = NULL;

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	eye = Vector3(0, 20, 40);
	specularColor = Vector3(0.5, 0.5, 0.5);
	ambientColor = Vector3(0.1, 0.1, 0.1);
	diffuseColor = Vector3(0.5, 0.5, 0.5);
	light = Vector3(0, 20, 60);
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(eye, Vector3(0,10,0),Vector3(0,1,0));
	camera->setPerspective(60,window_width / window_height,0.1,10000);

	//then we load a mesh
	mesh = new Mesh();
	mesh->loadOBJ("data/lee.obj");
	mesh->calculateCost();
	//mesh->edgeContraction();
	/*mesh->edgeContraction();
	mesh->edgeContraction();
	mesh->edgeContraction();*/

	//we load a shader
	phong = new Shader();
	phong->load("data/phong.vs", "data/phong.ps");

}

//render one frame
void Application::render(void)
{
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST );

	//Get the viewprojection
	Matrix44 viewprojection = camera->getViewProjectionMatrix();
	Matrix44 mvp = model_matrix * viewprojection;
	Matrix44 modelt = model_matrix.getRotationOnly(); 
	
	//Phong shader
	phong->enable();
	//upload info to the shader
	phong->setMatrix44("mvp", mvp);
	phong->setVector3("view", eye);
	phong->setVector3("light", light);
	phong->setMatrix44("model", modelt);
	phong->setVector3("specularColor", specularColor);
	phong->setVector3("ambientColor", ambientColor);
	phong->setVector3("diffuseColor", diffuseColor);
	//render the data
	mesh->render(GL_TRIANGLES);
	//disable render
	phong->disable();

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_W])
	{
		model_matrix.rotateLocal(seconds_elapsed, Vector3(1, 0, 0));
	}
	if (keystate[SDL_SCANCODE_A])
	{
		model_matrix.rotateLocal(seconds_elapsed, Vector3(0, 1, 0));
	}
	if (keystate[SDL_SCANCODE_S])
	{
		model_matrix.rotateLocal(seconds_elapsed, Vector3(-1, 0, 0));
	}
	if (keystate[SDL_SCANCODE_D])
	{
		model_matrix.rotateLocal(seconds_elapsed, Vector3(0, -1, 0));
	}
	if (keystate[SDL_SCANCODE_RIGHT])
	{
		model_matrix.traslateLocal(1, 0, 0);
	}
	if (keystate[SDL_SCANCODE_LEFT])
	{
		model_matrix.traslateLocal(-1, 0, 0);
	}
	if (keystate[SDL_SCANCODE_UP])
	{
		model_matrix.traslateLocal(0, 1, 0);
	}
	if (keystate[SDL_SCANCODE_DOWN])
	{
		model_matrix.traslateLocal(0, -1, 0);
	}
	if (keystate[SDL_SCANCODE_KP_PLUS])
	{
		model_matrix.traslateLocal(0, 0, 1);
	}
	if (keystate[SDL_SCANCODE_KP_MINUS])
	{
		model_matrix.traslateLocal(0, 0, -1);
	}
	if (keystate[SDL_SCANCODE_I])
	{
		light.set(light.x, light.y + 1, light.z);
	}
	if (keystate[SDL_SCANCODE_K])
	{
		light.set(light.x, light.y - 1, light.z);
	}
	if (keystate[SDL_SCANCODE_J])
	{
		light.set(light.x - 1, light.y, light.z);
	}
	if (keystate[SDL_SCANCODE_L])
	{
		light.set(light.x + 1, light.y, light.z);
	}
	if (keystate[SDL_SCANCODE_H])
	{
		light.set(light.x, light.y, light.z - 1);
	}
	if (keystate[SDL_SCANCODE_U])
	{
		light.set(light.x, light.y, light.z + 1);
	}

}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	int t_count = 0;
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: exit(0); break; //ESC key, kill the app
		case SDLK_m:
			if (event.type == SDL_KEYUP){
				/*printf ("Enter desired number of triangles: ");
				scanf ("%d", &t_count);
				printf ("Triangles: %d", t_count);*/
				mesh->edgeContraction(5);
				//TODO: execute surface simplification method
			}
			break;
		case SDLK_z:
			if (event.type == SDL_KEYUP) {
				GLint previous[2];
				glGetIntegerv(GL_POLYGON_MODE, previous);
				if (previous[1] == GL_FILL) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
			break;
		case SDLK_1:
			if (event.type == SDL_KEYUP){
				mesh->clear();
				mesh->loadOBJ("data/lee.obj");
			}
			break;
		case SDLK_2:
			if (event.type == SDL_KEYUP){
				mesh->clear();
				mesh->loadOBJ("data/man.obj");
			}
			break;
	}
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{

	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
