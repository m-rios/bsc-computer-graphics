/** vertexShader.fs
    Example vertexShader */

#version 330

layout(location=0) in vec4 inPosition;
layout(location=1) in vec4 inColour;
layout(location=2) in vec4 inNormal;
layout(location=3) in vec2 inTextureCoordinate;

out vec4 colour;
out vec4 position;
out vec4 normal;
out vec2 textureCoordinate;

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

		position = modelViewMatrix*inPosition;
  colour = inColour;
  normal = modelViewMatrix*inNormal;
  textureCoordinate = inTextureCoordinate;

	gl_Position = projectionMatrix*modelViewMatrix * inPosition;



}
