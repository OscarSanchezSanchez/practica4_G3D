#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outVertex;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec3 tangent;
in vec2 texCoord;

uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D normalTex;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 N;
float alpha = 500.0;
vec3 Ke;

//Propiedades de la luz
vec3 Ia = vec3 (0.3);
vec3 Id = vec3 (1.0);
vec3 Is = vec3 (0.7);
vec3 lpos = vec3 (0.0);
vec3 C_atenuacion = vec3(0.5,0.1,0.0);

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ke = texture(emiTex, texCoord).rgb;
	Ks = vec3 (1.0);

	N = normalize(norm);
//	vec3 T = normalize(tangent);
//	vec3 B = cross(N, T);
//
//	mat3 TBN = mat3(T, B, N);
//
//	N = texture(normalTex, texCoord).rgb;
//	N = normalize(N * 2.0 - 1.0);
//	N = normalize(TBN * N);
	outVertex = vec4(pos.zzz, 0);

	outColor = vec4(shade(), 1.0);   
}

vec3 shade()
{
	float d = distance(pos,lpos);
    float atenuation_factor = 1.0/(C_atenuacion.z * d*d + C_atenuacion.y * d + C_atenuacion.x) ;
    float Fatt = min(atenuation_factor,1);

	vec3 c = vec3(0.0);
	c = Ia * Ka;

	vec3 L = normalize (lpos - pos);
	vec3 diffuse = Id * Kd * dot (L,N) * Fatt;
	c += clamp(diffuse, 0.0, 1.0);
	
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max (dot (R,V), 0.01);
	vec3 specular = Is * Ks * pow(factor,alpha) * Fatt;
	c += clamp(specular, 0.0, 1.0);

	c+=Ke;
	
	return c;
}
