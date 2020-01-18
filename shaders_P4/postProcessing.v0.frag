#version 330 core
//Color de salida
layout (location = 0) out vec4 outColor;

//Textura
uniform sampler2D colorTex;

//Variables Variantes
in vec2 texCoord;

void main()
{
//Código del Shader
//NOTA: Sería más adecuado usar texelFetch.
//NOTA: No lo hacemos porque simplifica el paso 5
outColor = vec4(textureLod(colorTex, texCoord,0).xyz, 0.6);
//outColor = vec4(texCoord,vec2(1.0));
}