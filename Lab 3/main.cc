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
#include <vector>
#include <tuple>
/** Container for all OpenGL related resources such as textures and shaders that are used in the labs */
class Resources {
public:
  Resources();
  ~Resources();

  AC3DObject* object;
  /** buffer with vertex and normals of the whole model */
  std::vector<GLfloat> vertex_normals_vector;
  std::vector<GLuint> t_indices;
  std::vector<GLuint> q_indices;
  /** Function to (re)load the content of all resources such as textures and shaders */
  void reload();
  
  void get_3D_data(AC3DObject*);
  void get_leaf_3D_data(AC3DObject*);

  /* note: in a "real" program, you should protect all the resources in
     private variables with 'inlined' accessors (but not modifier)
     functions... but this is not a course about proper C++
     programming so we're gonna be lazy for now */
  GLuint fragmentShader, vertexShader, shaderProgram, diffuseTexture, specularTexture;
  GLuint vertexBuffer, t_ind_buffer, q_ind_buffer;

  /** Mark if the loaded shader code is "ok" or not - do not attempt to use any shaders or 
      draw anything if false. */
  int isOk;
};

/** Container for the world state representation. Eg., camera position, players/enemies/simulation data */
class World {
public:
  World();
  ~World();
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
  //populate vertex_normals
  get_3D_data(this->object);

  //Buffer for vertex / normals
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertex_normals_vector.size(), &vertex_normals_vector[0], GL_STATIC_DRAW);
  //Buffer for triangle indices
  glGenBuffers(1, &t_ind_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t_ind_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*t_indices.size(), &t_indices[0], GL_STATIC_DRAW);
  //Buffer for quad indices
  glGenBuffers(1, &q_ind_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q_ind_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*q_indices.size(), &q_indices[0], GL_STATIC_DRAW);
  reload();
}
Resources::~Resources() {
  /* Free shader and program objects */
  glDeleteProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}
void Resources::get_3D_data(AC3DObject* node)
{
	if (node->nChildren > 0)
	{
		for (int i = 0; i < node->nChildren; i++)
		{
			get_3D_data(node->children[i]);
		}
	}
	else
	{
		return get_leaf_3D_data(node);
	}
}
void Resources::get_leaf_3D_data(AC3DObject* node)
{
	glm::vec4* normals = new glm::vec4[node->nVertices];
	glm::vec4* surface_normals = new glm::vec4[node->nSurfaces];
	std::vector<GLuint> triangle_ind(node->nTriangles * 3);
	std::vector<GLuint> quad_ind(node->nQuads * 4);
	int t_counter = 0;
	int q_counter = 0;
	// Create indices
	for (int i = 0; i<node->nSurfaces; i++)
	{
		AC3DSurface surface = node->surfaces[i];

		if (surface.nVertices == 3)
		{
			for (int j = 0; j < 3; j++)
			{
				triangle_ind[t_counter] = surface.vert[j].index * 8 + this->vertex_normals_vector.size();
				t_counter++;
			}
		}
		else
		{
			for (int j = 0; j < 4; j++)
			{
				quad_ind[q_counter] = surface.vert[j].index * 8 + this->vertex_normals_vector.size();
				q_counter++;
			}
		}
	}

	//compute surface normals
	for (int i = 0; i < node->nSurfaces; i++)
	{
		glm::vec3 corners[3];
		//iterate over 3 vertices
		for (int j = 0; j < 3; j++)
		{
			glm::vec3 corner;
			//base index for point coordinates
			int index = node->surfaces[i].vert[j].index * 4;
			int counter = 0;
			//iterate over 3 coordinates for each vertex (no homogeneus coordinates)
			for (int k = index; k < index + 3; k++)
			{
				corner[counter] = node->vertices[k];
				counter++;
			}
			corners[j] = corner;
		}
		//get homogeneus normalize vector coordinate
		surface_normals[i] = glm::vec4(glm::cross(glm::normalize(corners[1] - corners[0]), glm::normalize(corners[2] - corners[1])), 0.0f);
		//compute vertex normals NOTE: no need to normalize, fragment shader does that
		for (int j = 0; j < node->surfaces[i].nVertices; j++)
		{
			normals[node->surfaces[i].vert[j].index] += surface_normals[i];
		}
	}

	//interleave position and normal in a same buffer
	int size = node->nVertices * 4 * 2;
	std::vector<GLfloat> leaf_vertex_normal(size);
	int v_counter = 0;
	int n_counter = 0;
	for (int i = 0; i < size; i+=8)
	{
		for (int j = i; j < i+4; j++)
		{
			leaf_vertex_normal[j] = node->vertices[v_counter];
			v_counter++;
		}
		for (int k = i+4; k < i+8; k++)
		{
			leaf_vertex_normal[k] = normals[n_counter][k%4];
		}
		n_counter++;
	}
	//update global variables
	this->vertex_normals_vector.insert(this->vertex_normals_vector.end(), leaf_vertex_normal.begin(), leaf_vertex_normal.end());
	this->t_indices.insert(this->t_indices.end(), triangle_ind.begin(), triangle_ind.end());
	this->q_indices.insert(this->q_indices.end(), quad_ind.begin(), quad_ind.end());
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

  /* Pass projection Matrix to the GPU*/
  glUniformMatrix4fv(pMatrix, 1, GL_TRUE, &projectionMatrix[0][0]);
  /* Pass modelview Matrix to the GPU*/
  //glUniformMatrix4fv(mvMatrix, 1, GL_TRUE, &modelviewMatrix[0][0]);
  glUniformMatrix4fv(mvMatrix, 1, GL_FALSE , &modelViewMatrix[0][0]);
  
  glUniform4fv(lightOposFragmentShader, 1, glm::value_ptr(lightOpos));
  //glUniformMatrix4fv(lightOposFragmentShader, 1, GL_TRUE, &lightOpos[0]);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  //GLint posAttrib = glGetAttribLocation(r->shaderProgram, "inPosition");
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 1 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
  //GLint normAttrib = glGetAttribLocation(r->shaderProgram, "inNormal");
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 1*sizeof(GLfloat), (void *) (4*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  	
  
  //// Set up and enable glVertexAttribPointers
  //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 1 * sizeof(GLfloat), (void *)&r->vertex_normals_vector[0]);
  //glEnableVertexAttribArray(0);
  //glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 1 * sizeof(GLfloat), (void *)&r->vertex_normals_vector[4]);
  //glEnableVertexAttribArray(1);

  if (r->t_indices.size() > 0)
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->t_ind_buffer);
	glDrawElements(GL_TRIANGLES, r->t_indices.size(), GL_UNSIGNED_INT,0);
  if (r->q_indices.size() > 0)
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->q_ind_buffer);
	glDrawElements(GL_QUADS, r->q_indices.size(), GL_UNSIGNED_INT, 0);



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
