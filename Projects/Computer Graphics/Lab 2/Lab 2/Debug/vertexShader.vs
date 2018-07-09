/** vertexShader.fs
    Example vertexShader */

#version 330

layout(location=0) in vec4 inPosition;
layout(location=1) in vec4 inColour;

out vec4 colour;

//Homogeneus matrix for the perspective transformation
uniform mat4 projectionMatrix;
//matrix that transforms from world to view coordinates
uniform mat4 modelViewMatrix;

void main() {
	//Perform the transformation of each vertex
	//copy position vector, to be able to modify it
	//vec4 position = inPosition;
	//update z coordinate
	//position.z = position.z - 4;
	gl_Position = projectionMatrix * modelViewMatrix * inPosition;
  colour = inColour;

}
