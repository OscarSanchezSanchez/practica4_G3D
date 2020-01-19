#version 330 core

//Variables Variantes
in vec2 texCoord;

//Color de salida
layout(location = 0) out vec4 outColor;

//Textura
uniform sampler2D colorTex;
uniform sampler2D vertexTex;

const float focalDistance = -25.0;
const float maxDistanceFactor = 1.0/5.0;

uniform float mask[25];
uniform float maskFactor;

#define MASK_SIZE 25u
const vec2 texIdx[MASK_SIZE] = vec2[](
	vec2(-2.0,2.0), vec2(-1.0,2.0), vec2(0.0,2.0), vec2(1.0,2.0), vec2(2.0,2.0),
	vec2(-2.0,1.0), vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0), vec2(2.0,1.0),
	vec2(-2.0,0.0), vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0), vec2(2.0,0.0),
	vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0), vec2(2.0,-1.0),
	vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2(0.0,-2.0), vec2(1.0,-2.0), vec2(2.0,-2.0));

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0));
	float dof = abs(texture(vertexTex,texCoord).r -focalDistance) * maxDistanceFactor;

	dof = clamp (dof, 0.0, 1.0);
	dof *= dof;
	vec4 color = vec4 (0.0);

	for (uint i = 0u; i < MASK_SIZE; i++)
	{
		vec2 iidx = texCoord + ts * texIdx[i]*dof;
		color += texture(colorTex, iidx,0.0) * mask[i];
	}
	outColor = color;
}