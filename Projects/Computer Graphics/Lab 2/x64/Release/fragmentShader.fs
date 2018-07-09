/** fragmentShader.fs
    Example fragmentShader */

#version 330 


in vec4 position;
in vec4 colour;
in vec4 normal;
in vec2 textureCoordinate;

out vec4 fragmentColour;

uniform vec4 lightOpos;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main() {
    //Lighting parameters
  vec3 Ca = { 0.8, 0.8, 0.5 };
//  vec4 Ca = texture(diffuseTexture, textureCoordinate);
//  vec3 Cd = { 0.8, 0.8, 0.5 };
//  vec3 Cs = { 1.0, 1.0, 1.0 };
  vec4 Cd = texture(diffuseTexture, textureCoordinate*8);
  vec4 Cs = texture(specularTexture, textureCoordinate*8);
  vec3 Ia = { 0.2, 0.2, 0.2};
  vec3 Id = { 0.8, 0.8, 0.8};
  vec3 Is = { 3.0, 3.0, 3.0};
  float f = 20.0;

  vec4 n = normalize(normal);
  vec4 v = normalize(-position);
  vec4 wi = normalize(lightOpos - position);
//  vec4 wr = normalize(2*(dot(wi,n))*n - wi);
vec4 wr = reflect(-wi,n);
  vec3 C = Ca.rgb*Ia + Cd.rgb*Id*dot(n,wi) + Cs.rgb*Is*pow(dot(wr,v),f);
  vec3 ambient = Ca.rgb*Ia;
  vec3 diffuse = Cd.rgb*Id*dot(n,wi);
  vec3 specular = (Cs.rgb*Is)*pow(max(dot(v,wr),0.0f),f);
//    fragmentColour.rgb = colour.rgb + C;
//    fragmentColour = colour;
  fragmentColour.rgb = ambient + diffuse + specular;
}
