/* lab-2.1
   Copyright (C) 2011 Mathias Broxvall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "general.h"
#include "glUtil.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <math.h> //mathematical functions
#include <ctime> //capture time for alpha angle
#include <iostream>
#include "glm/ext.hpp"
#include <string>
/** Container for all OpenGL related resources such as textures and shaders that are used in the labs */
class Resources {
public:
  Resources();
  ~Resources();

  AC3DObject* object;

  /** Function to (re)load the content of all resources such as textures and shaders */
  void reload();

  /* note: in a "real" program, you should protect all the resources in
     private variables with 'inlined' accessors (but not modifier)
     functions... but this is not a course about proper C++
     programming so we're gonna be lazy for now */
  GLuint fragmentShader, vertexShader, shaderProgram, diffuseTexture, specularTexture;

  /** Mark if the loaded shader code is "ok" or not - do not attempt to use any shaders or 
      draw anything if false. */
  int isOk;
};

/** Container for the world state representation. Eg., camera position, players/enemies/simulation data */
class World {
public:
  World();
  ~World();
  /** Traverses the AC3DObject */
  void World::ModelRedraw(AC3DObject*, Resources*);
  /** Draws the primitives of the leaves of the tree */
  void World::ModelRedrawLeaf(AC3DObject*);
  /** Draws all graphics, using the given OpenGL resources */
  void doRedraw(Resources*);
  /** Updates the world */
  void tick(double deltaTime);
  /** Handles keyboard input */
  void doKeyboard(int,int,int);
private:
  double time;
};

/* Important global variables */
int screenWidth, screenHeight, isRunning;
Resources *resources;
World *world;


Resources::Resources() {
  /* Create shader and program objects, attach shaders to program */
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  shaderProgram = glCreateProgram();
  /*GLuint textures[2];
  glGenTextures(2, &textures[0]);
  diffuseTexture = textures[0];
  specularTexture = textures[1];*/
  AC3DObjectFactory factory = AC3DObjectFactory();
  object = factory.loadAC3D("fighter.ac");
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  reload();
}
Resources::~Resources() {
  /* Free shader and program objects */
  glDeleteProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Resources::reload() {
  isOk=0;

  loadTexture(diffuseTexture, "texture1.png", 1);
  loadTexture(specularTexture, "texture2.png", 1);
  
  /* Recompile shader objects, mark resources as not-ok if any shader fails */
  if(compileShader(fragmentShader, "fragmentShader.fs", "fragmentShader.log")) return;
  if(compileShader(vertexShader, "vertexShader.vs", "vertexShader.log")) return;
  /* Link all shader programs */
  if(linkProgram(shaderProgram,"program.log")) return;
  isOk=1;
}

World::World() {
  time=0.0;
}

void World::ModelRedraw(AC3DObject*node, Resources*r)
{
	if (node->nChildren > 0)
	{
		for (int i = 0; i < node->nChildren; i++)
		{
			ModelRedraw(node->children[i], r);
			//return;
		}
	}
	else
	{
		ModelRedrawLeaf(node);
	}
}

void World::ModelRedrawLeaf(AC3DObject* obj)
{
	glm::ivec4* indices = new glm::ivec4[obj->nSurfaces];
	glm::vec4* normals = new glm::vec4[obj->nVertices];
	glm::vec4* surface_normals = new glm::vec4[obj->nSurfaces];

	//debug crap======================================================================================
	/*for (int i = 0; i < obj->nVertices; i++)
	{
	for (int j = 0; j < 4; j++)
	{
	std::cout << normals[i][j] << ", ";
	}
	std::cout << std::endl;
	}
	system("pause");*/
	//end debug crap======================================================================================

	// Create indices
	for (int i = 0; i<obj->nSurfaces; i++)
	{
		AC3DSurface surface = obj->surfaces[i];
		for (int j = 0; j < surface.nVertices; j++) // nVertices is 3 or 4
		{
			indices[i][j] = surface.vert[j].index;
		}
	}

	//debug crap ==================================================
	//for (int i = 0; i < obj->nSurfaces; i++)
	//{
	//	for (int j = 0; j < obj->surfaces[i].nVertices; j++)
	//	{
	//		std::cout << obj->surfaces[i].vert[j].index << ", ";
	//	}
	//	std::cout << std::endl;
	//}
	//end debug crap ==================================================

	//compute surface normals
	for (int i = 0; i < obj->nSurfaces; i++)
	{
		glm::vec3 corners[3];
		//iterate over 3 vertices
		for (int j = 0; j < 3; j++)
		{
			glm::vec3 corner;
			//base index for point coordinates
			int index = obj->surfaces[i].vert[j].index * 4;
			int counter = 0;
			//iterate over 3 coordinates for each vertex (no homogeneus coordinates)
			for (int k = index; k < index + 3; k++)
			{
				corner[counter] = obj->vertices[k];
				counter++;
			}
			corners[j] = corner;
		}

		/*glm::vec3 vertex0;
		glm::vec3 vertex1;
		glm::vec3 vertex2;
		for (int j = 0; j < 3; j++)
		{
			vertex0[j] = obj->vertices[obj->surfaces[i].vert[0].index * 4 + j];
			vertex1[j] = obj->vertices[obj->surfaces[i].vert[1].index * 4 + j];
			vertex2[j] = obj->vertices[obj->surfaces[i].vert[2].index * 4 + j];
		}*/

		//get homogeneus normalize vector coordinate
		surface_normals[i] = glm::vec4(glm::cross(glm::normalize(corners[1] - corners[0]), glm::normalize(corners[2] - corners[1])), 0.0f);
		/*surface_normals[i] = glm::vec4(cross(glm::normalize(vertex1 - vertex0), glm::normalize(vertex2 - vertex1)), 0.0f);

		normals[obj->surfaces[i].vert[0].index] += surface_normals[i];
		normals[obj->surfaces[i].vert[1].index] += surface_normals[i];
		normals[obj->surfaces[i].vert[2].index] += surface_normals[i];*/
	}

	//compute vertex normals
	//iterate over all surfaces
	for (int i = 0; i < obj->nSurfaces; i++)
	{
		//iterate over all 3or4 vertices of a surface
		for (int j = 0; j < obj->surfaces[i].nVertices; j++)
		{
			normals[obj->surfaces[i].vert[j].index] += surface_normals[i];
		}
	}

	/*GLfloat* outNormals = obj->vertices;
	int counter = 0;
	for (int i = 0; i < obj->nVertices; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			outNormals[counter] = normals[i][j];
		}
	}*/

	// Set up and enable glVertexAttribPointers
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)obj->vertices);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *) normals);
	glEnableVertexAttribArray(1);

	// Draw:
	for (int i = 0; i < obj->nSurfaces; i++) {

		if (obj->surfaces[i].nVertices == 3)
		{
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &indices[i][0]);
		}
		else
		{
			glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, &indices[i][0]);
		}
	}
}

void World::doRedraw(Resources *r) {
  GLuint error;

  /* If the resources (shaders etc.) are not ok, then don't draw anything */
  if(!r->isOk) return;

  /* Setup the viewport to use all of the SDL window */
  glViewport(0,0,(GLsizei) screenWidth,(GLsizei) screenHeight); 

  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* Specify four vertices, each consisting of four XYZW points. */
  float sw=screenWidth, sh=screenHeight;   

  //crap test------------------------------------------------------------------------------------------------------------------------------------------
  //GLfloat vertices[12][4] = { { -1.0, -1.0, 0.0, 1.0 },
  //{ 1.0, -1.0, 0.0, 1.0 },
  //{ 1.0, +1.0, 0.0, 1.0 },
  //{ -1.0, +1.0, 0.0, 1.0 },
  ////new vertices
  //{ -0.5, -0.5, 0.0, 1.0 },
  //{ 0.5, -0.5, 0.0, 1.0 },
  //{ 0.5, 0.5, 0.0, 1.0 },
  //{ -0.5, 0.5, 0.0, 1.0 },
  ////extra vertices
  //{ -0.0, -0.5, 0.0, 1.0 },
  //{ 0.5, 0.0, 0.0, 1.0 },
  //{ 0.0, 0.5, 0.0, 1.0 },
  //{ -0.5, 0.0, 0.0, 1.0 }
  //};

  ///* Give colours for each such vertice */
  //GLfloat colours[12][3] = { { 0.5, 0.5, 0.0 },
  //{ 0.5, 0.5, 0.0 },
  //{ 0.5, 0.5, 0.0 },
  //{ 0.5, 0.5, 0.0 },
  ////red
  //{ 1.0, 0.0, 0.0 },
  ////green
  //{ 0.0, 1.0, 0.0 },
  ////blue
  //{ 0.0, 0.0, 1.0 },
  ////yellow
  //{ 1.0, 1.0, 0.0 },
  ////red
  //{ 1.0, 0.0, 0.0 },
  ////green
  //{ 0.0, 1.0, 0.0 },
  ////blue
  //{ 0.0, 0.0, 1.0 },
  ////yellow
  //{ 1.0, 1.0, 0.0 }
  //};
  ///* This is a list of graphic primitives (as triangles), that reference the above vertices */
  //GLuint indices[6][3] = { { 0, 1, 2 },
  //{ 2, 3, 0 },
  ////first screenshot
  //{ 7, 6, 5 },
  //{ 5, 4, 7 },
  //// 2nd screenshot
  //{ 6, 5, 4 },
  //{ 4, 7, 6 } };
  //end of crap test------------------------------------------------------------------------------------------------------------------------------------------

  //n is near clipping plane, f is far clipping plane
  GLfloat n = 1;
  GLfloat f = 10;
  
  //computed from n f
  GLfloat a = (f + n) / (f - n);
  GLfloat b = 2 * f*n / (f - n);
  
  //Homogeneus matrix for the perspective transformation
  GLfloat projectionMatrix[4][4] = {
		  {1.0, 0.0, 0.0, 0.0},
		  {0.0, 1.0, 0.0, 0.0},
		  {0.0, 0.0, -a, -b},
		  {0.0, 0.0, -1, 0.0}
  };

  //GLfloat lightOpos[4] = { 2, 1, 0, 1 };
  glm::vec4 lightOpos = glm::vec4(2.0f, 1.0f, 0.0f, 1.0f);

  //angle value
  //time_t timev = std::time(0);
  GLfloat alpha = time;
  GLfloat inc_z = -4.0;

  //homogeneus matrix to translate -4u along the z axis
  glm::mat4 transZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
 //homogeneus matrices to rotate around the x & y axis according to time
  glm::mat4 rotY = glm::rotate(glm::mat4(1.f), (float) time, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 rotX = glm::rotate(glm::mat4(1.f), (float)time*0.4f, glm::vec3(1.0f, 0.0f, 0.0f));
  //transformation matrix combining all previous transformations
  glm::mat4 modelViewMatrix = transZ * rotY * rotX;
  glm::vec2 textureCoordinate = glm::vec2(0.1f, 0.1f);


  //Pointer to the shader variable 'projectionMatrix'
  GLint pMatrix = glGetUniformLocation(r->shaderProgram, "projectionMatrix");
 //Pointer to the shader variable 'modelViewMatrix'
  GLint mvMatrix = glGetUniformLocation(r->shaderProgram, "modelViewMatrix");
  //Pointer to lightOpos in fragment shader
  GLint lightOposFragmentShader = glGetUniformLocation(r->shaderProgram, "lightOpos");

  glUseProgram(r->shaderProgram);

  /* Attrib, N-Elements, Type, Normalize, Stride, Pointer */
  //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) vertices);  
  //glEnableVertexAttribArray(0);

  //glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) colours);
  //glEnableVertexAttribArray(1);

  /* Pass projection Matrix to the GPU*/
  glUniformMatrix4fv(pMatrix, 1, GL_TRUE, &projectionMatrix[0][0]);
  /* Pass modelview Matrix to the GPU*/
  //glUniformMatrix4fv(mvMatrix, 1, GL_TRUE, &modelviewMatrix[0][0]);
  glUniformMatrix4fv(mvMatrix, 1, GL_FALSE , &modelViewMatrix[0][0]);
  
  glUniform4fv(lightOposFragmentShader, 1, glm::value_ptr(lightOpos));
  //glUniformMatrix4fv(lightOposFragmentShader, 1, GL_TRUE, &lightOpos[0]);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  //crap test =========================================================================================================================
  ///* Attrib, N-Elements, Type, Normalize, Stride, Pointer */
  //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)&vertices);
  //glEnableVertexAttribArray(0);

  //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)&colours);
  //glEnableVertexAttribArray(1);
  //glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, &indices[2][0]);
  //crap test =========================================================================================================================

  ModelRedraw(r->object, r);

  if((error=glGetError()) != GL_NO_ERROR) {
    fprintf(stderr,"GL Error: %s\n",gluErrorString(error));
  }
}

void World::doKeyboard(int key,int mouseX,int mouseY) {
  /** TODO - implement a keyhandler that does something interesting when given inputs */
  switch(key) {
  }
}

void World::tick(double deltaTime) { time += deltaTime; }


/** Main function, initializing the program and performing the draw/event/tick loop */
int main(int argc,char **args) {
  SDL_Event event;
  SDL_MouseButtonEvent *e=(SDL_MouseButtonEvent*)&event;
  
  /* Initialize SDL and create window */
  if(SDL_Init(SDL_INIT_VIDEO) == -1) {
    printf("Failed to initialize SDL. Error '%s'\n",SDL_GetError());
    exit(-1);
  }
  atexit(SDL_Quit);
  SDL_WM_SetCaption("Datorgrafik","Datorgrafik");
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
  screenWidth=800; screenHeight=600;

  /* Add flag SDL_FULLSCREEN if you like, but be carefull since this
     may crash the computer if you have any bugs in your program! */
  SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_SWSURFACE | SDL_OPENGL);

  if(glewInit() != GLEW_OK) {
    fprintf(stderr,"WARNING - failed to initialize GLEW. This will most likely fail your program\n");
  }

  resources = new Resources();
  world = new World();

  isRunning=1;
  double oldTime, newTime, timeDelta;
  oldTime=SDL_GetTicks()/1000.0;
  while(isRunning) {
    /* Yield some processor time to other processes, otherwise windows might hang */
    SDL_Delay(1);

    /* Compute how much time has elapsed since last frame */
    newTime=SDL_GetTicks()/1000.0;
    timeDelta = newTime - oldTime;
    oldTime=newTime;

    /* Update world */
    world->tick(timeDelta);

    /* Draw world */   
    world->doRedraw(resources);
    glFlush();
    SDL_GL_SwapBuffers();

    /* Process any events that have occured */
    while(SDL_PollEvent(&event)) 
      switch(event.type) {
      case SDL_QUIT: isRunning=0; break;
      case SDL_MOUSEBUTTONDOWN:
	/* do something here. e->button is mouse button, e->x and e->y is coordinate of mouse */
	break;
      case SDL_KEYDOWN:
	if(event.key.keysym.sym == SDLK_ESCAPE) {
	  isRunning=0; break;
	} else {
	  int mouseX, mouseY;
	  Uint8 mouseButtonState=SDL_GetMouseState(&mouseX,&mouseY);
	  switch(event.key.keysym.sym) {
	  case 27:
	    exit(0);
	    break;
	  case 'l':
	    resources->reload();
	    break;
	  default:
	    world->doKeyboard(event.key.keysym.sym,mouseX,mouseY);
	  } 
	}
      }
  }
  SDL_Quit();
}

#ifdef WINDOWS
#include "stdafx.h"
int _tmain(int argc, _TCHAR* argv[]) {
  main(0,NULL);
  return 0;
}
#endif
