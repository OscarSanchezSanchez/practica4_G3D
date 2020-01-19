#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500
#define MASK_SIZE 25

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);

//Vars desplazamientos por teclado
float displacement = 0.1f;
//Vars giro de c�mara por teclado
float yaw_angle = 0.01f;


//Vars movimiento de c�mara con el rat�n
const float orbitAngle = 0.1f;
float lastX = 0.0f;
float lastY = 0.0f;
float desplX = 0.0f;
float desplY = 0.0f;

//Vars motion blur
glm::vec4 blurParams(1.0f, 1.0f, 1.0f, 0.0f);
float modifyBlur = 0.1f;

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
float angle = 0.0f;

unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;

//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;
unsigned int tangentVBO;

unsigned int colorTexId;
unsigned int emiTexId;

unsigned int planeVAO;
unsigned int planeVertexVBO;

//Shaders and programs
unsigned int vshader;
unsigned int fshader;
unsigned int program;

unsigned int postProccesVShader;
unsigned int postProccesFShader;
unsigned int postProccesProgram;

//Uniform variables
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Texturas Uniform
int uColorTex;
int uEmiTex;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//Convolution variables and uniforms
int umaskfactor;
int umask;
float maskFactor = float(1.0 / 65.0);
float* mask = new float[MASK_SIZE] 
{
	1.0f * maskFactor, 2.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor, 1.0f * maskFactor,
	2.0f * maskFactor, 3.0f * maskFactor, 4.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor,
	3.0f * maskFactor, 4.0f * maskFactor, 5.0f * maskFactor, 4.0f * maskFactor, 3.0f * maskFactor,
	2.0f * maskFactor, 3.0f * maskFactor, 4.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor,
	1.0f * maskFactor, 2.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor, 1.0f * maskFactor 
};

//Uniform
unsigned int uColorTexPP;

//Atributos
int inPosPP;

unsigned int uVertexTexPP;
unsigned int vertexBuffTexId;

//Uniform DOF
unsigned int uFocalDistance;
unsigned int uMaxDistanceFactor;

//Vars DOF
float focalDistance = -25.0f;
float maxDistanceFactor = 1.0f / 5.0f;

const float modifyDist = 1.0f;
const float modDistFactor = 2.0f;

//Variables DOF by Z-buffer
unsigned int uNear;
unsigned int uFar;
float pnear = 1.0f;
float pfar = 50.0f;
unsigned int uDepthTexPP;

//Bump Mapping
unsigned int uNormalTex;

unsigned int inTangent;
unsigned int normalTexId;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);

void renderCube();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initShaderPP(const char* vname, const char* fname);
void initObj();
void initPlane();
void initFBO();
void destroy();
void resizeFBO(unsigned int w, unsigned int h);


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.vert", "../shaders_P4/fwRendering.frag");
	initShaderPP("../shaders_P4/postProcessing.vert","../shaders_P4/postProcessing.frag");

	initObj();
	initPlane();
	initFBO();
	
	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, pnear, pfar);
	view = glm::mat4(1.0f);
	view[3].z = -25.0f;
}


void initFBO()
{
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
	glGenTextures(1, &vertexBuffTexId);

	resizeFBO(SCREEN_SIZE);
}

void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO); 
	if (inTangent != -1) glDeleteBuffers(1, &tangentVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

	glDetachShader(postProccesProgram, postProccesVShader);
	glDetachShader(postProccesProgram, postProccesFShader);
	glDeleteShader(postProccesVShader);
	glDeleteShader(postProccesFShader);
	glDeleteProgram(postProccesProgram);

	glDeleteBuffers(1, &planeVertexVBO);
	glDeleteVertexArrays(1, &planeVAO);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);
	glDeleteTextures(1, &vertexBuffTexId);

}

void initShaderFw(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");
	glBindAttribLocation(program, 4, "inTangent");

	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");
	uNormalTex = glGetUniformLocation(program, "normalTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
	inTangent = glGetAttribLocation(program, "inTangent");

}

void initShaderPP(const char* vname, const char* fname)
{
	postProccesVShader = loadShader(vname, GL_VERTEX_SHADER);
	postProccesFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProccesProgram = glCreateProgram();
	glAttachShader(postProccesProgram, postProccesVShader);
	glAttachShader(postProccesProgram, postProccesFShader);
	glBindAttribLocation(postProccesProgram, 0, "inPos");
	glLinkProgram(postProccesProgram);
	int linked;
	glGetProgramiv(postProccesProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProccesProgram, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(postProccesProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProccesProgram);
		postProccesProgram = 0;
		exit(-1);
	}
	uColorTexPP = glGetUniformLocation(postProccesProgram, "colorTex");
	inPosPP = glGetAttribLocation(postProccesProgram, "inPos");

	//DOF
	uFocalDistance = glGetUniformLocation(postProccesProgram, "focalDistance");
	uMaxDistanceFactor = glGetUniformLocation(postProccesProgram, "maxDistanceFactor");

	//z-buffer 
	uNear = glGetUniformLocation(postProccesProgram, "near");
	uFar = glGetUniformLocation(postProccesProgram, "far");
	uDepthTexPP = glGetUniformLocation(postProccesProgram, "depthTex");

	//Apartado 4, uniform para las mascaras de convolucion
	umaskfactor = glGetUniformLocation(postProccesProgram, "maskFactor");
	umask = glGetUniformLocation(postProccesProgram, "mask");

	uVertexTexPP = glGetUniformLocation(postProccesProgram, "vertexTex");


}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}


	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	if (inTangent != -1)
	{
		glGenBuffers(1, &tangentVBO);
		glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexTangent, GL_STATIC_DRAW);
		glVertexAttribPointer(inTangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTangent);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex*sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
	normalTexId = loadTex("../img/normal.png");
}

void initPlane()
{
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);
	glGenBuffers(1, &planeVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3, planeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(inPosPP, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inPosPP);
}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char *fileName)
{
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/**/
	glUseProgram(program);

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	if (uNormalTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, normalTexId);
		glUniform1i(normalTexId, 2);
	}


	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	renderCube();

	std::srand(RAND_SEED);
	for (unsigned int i = 0; i < 10; i++)
	{
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2),
			float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle*2.0f*size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, angle*2.0f*size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size*0.7f)));
		renderCube();
	}
	//*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(postProccesProgram);

	if (uFocalDistance != -1) 
        glUniform1fv(uFocalDistance, 1, &focalDistance);
	if (uMaxDistanceFactor != -1) 
        glUniform1fv(uMaxDistanceFactor, 1, &maxDistanceFactor);

	if (uNear != -1) 
		glUniform1fv(uNear, 1, &pnear);
	if (uFar != -1) 
		glUniform1fv(uFar, 1, &pfar);

	//Apartado4
	if (umaskfactor != -1) 
		glUniform1fv(umaskfactor, 1, &maskFactor);
	if (umask != -1) 
		glUniform1fv(umask, 25, mask);
	//end-apartado4

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	/*glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);*/
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA);
	glBlendColor(blurParams.r, blurParams.g, blurParams.b, blurParams.a);
	glBlendEquation(GL_FUNC_ADD);


	if (uColorTexPP != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
		glUniform1i(uColorTexPP, 0);
	}

	if (uVertexTexPP != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
		glUniform1i(uVertexTexPP, 1);
	}
	if (uDepthTexPP != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
		glUniform1i(uDepthTexPP, 2);
	}

	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);


	glutSwapBuffers();
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
		&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
		&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
		&(normal[0][0]));
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
}



void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) /float(height), pnear, pfar);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.02f;
	
	glutPostRedisplay();
}

void resizeFBO(unsigned int w, unsigned int h)
{
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, colorBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthBuffTexId, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, vertexBuffTexId, 0);
	const GLenum buffs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffs);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	std::cout << "Se ha pulsado la tecla " << key << std::endl << std::endl;

	glm::mat4 rotation(1.0f);

	switch (key)
	{
	case 'w':
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, displacement));
		break;
	case 's':
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -displacement));
		break;
	case 'a':
		view = glm::translate(view, glm::vec3(displacement, 0.0f, 0.0f));
		break;
	case 'd':
		view = glm::translate(view, glm::vec3(-displacement, 0.0f, 0.0f));
		break;
	case 'q':
		rotation = glm::rotate(rotation, -yaw_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		view = rotation * view;
		break;
	case 'e':
		rotation = glm::rotate(rotation, yaw_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		view = rotation * view;
		break;
	//Motion blur
	case 'r':
		blurParams.r += modifyBlur;
		break;
	case 'g':
		blurParams.g += modifyBlur;
		break;
	case 'b':
		blurParams.b += modifyBlur;
		break;
	case 'f':
		blurParams.a += modifyBlur;
		break;
	case 't':
		blurParams.r -= modifyBlur;
		break;
	case 'h':
		blurParams.g -= modifyBlur;
		break;
	case 'n':
		blurParams.b -= modifyBlur;
		break;
	case 'y':
		blurParams.a -= modifyBlur;
		break;
	//DOF
	case 'z':
		focalDistance += modifyDist;
		break;
	case 'x':
		focalDistance -= modifyDist;
		break;
	case 'c':
		maxDistanceFactor *= modDistFactor;
		break;
	case 'v':
		maxDistanceFactor /= modDistFactor;
		break;
	case '1':
		//enfocar
		maskFactor = float(1.0);
		mask = new float[25]
		{
			0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 0.0f * maskFactor, -1.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, -1.0f * maskFactor, 5.0f * maskFactor, -1.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 0.0f * maskFactor, -1.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor 
		};
		break;
	case '2':
		//detectar bordes
		maskFactor = float(1.0);
		mask = new float[25]
		{
			0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 0.0f * maskFactor, 1.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 1.0f * maskFactor, -4.0f * maskFactor, 1.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 0.0f * maskFactor, 1.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor,
			0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor, 0.0f * maskFactor 
		};
		break;
	case '0':
		maskFactor = float(1.0 / 65.0);
		mask = new float[MASK_SIZE]
		{
			1.0f * maskFactor, 2.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor, 1.0f * maskFactor,
			2.0f * maskFactor, 3.0f * maskFactor, 4.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor,
			3.0f * maskFactor, 4.0f * maskFactor, 5.0f * maskFactor, 4.0f * maskFactor, 3.0f * maskFactor,
			2.0f * maskFactor, 3.0f * maskFactor, 4.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor,
			1.0f * maskFactor, 2.0f * maskFactor, 3.0f * maskFactor, 2.0f * maskFactor, 1.0f * maskFactor
		};
		break;
	default:
		break;
	}

	glutPostRedisplay();

}

void mouseFunc(int button, int state, int x, int y)
{
	if (state == 0)
		std::cout << "Se ha pulsado el boton ";
	else
		std::cout << "Se ha soltado el boton ";

	if (button == 0) std::cout << "de la izquierda del raton " << std::endl;
	if (button == 1) std::cout << "central del raton " << std::endl;
	if (button == 2) std::cout << "de la derecha del raton " << std::endl;

	std::cout << "en la posicion " << x << " " << y << std::endl << std::endl;
}

void mouseMotionFunc(int x, int y)
{

	float xOffset = (float)x - lastX;
	float yOffset = (float)y - lastY;

	lastX = (float)x;
	lastY = (float)y;

	desplX += xOffset;
	desplY += yOffset;

	glm::mat4 rotation(1.0f);

	view = glm::rotate(view, orbitAngle, glm::vec3(desplY, desplX, 0.0));

	glutPostRedisplay();
}