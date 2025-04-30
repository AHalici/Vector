#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/SOIL2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Utils.h"

using namespace std;

#define numVAOs 1
#define numVBOs 5

GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Display variables
GLuint mvLoc, pLoc;
int width, height;
float aspect;
glm::mat4 pmMat, vlmMat, hlmMat, amMat, vMat, pMat, mvMat;

// Camera
glm::vec3 cameraLoc;

// Object Locations
float planeLocX, planeLocY, planeLocZ;

// Object number of vertices
unsigned int planeNumVertices;
unsigned int lineNumVertices;

// Tool variables
bool isLine = false;
bool isRow = false;
bool isAxes = false;
GLuint isLineLoc, isRowLoc, isAxesLoc;



float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
void setupVertices();
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
glm::vec3 convert(glm::vec3 ogVec);
glm::vec3 vector3(float x, float y, float z);


int main(void)
{
	// GLFW init
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

	// Full Screen Monitor Options
	/*GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Vector - Puzzle Game", primaryMonitor, NULL);*/

	GLFWwindow* window = glfwCreateWindow(1600, 1200, "Vector - Puzzle Game", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	init(window);

	while (!glfwWindowShouldClose(window))
	{
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void init(GLFWwindow* window)
{
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	cameraLoc = glm::vec3(0.0f, 20.0f, 10.0f);
	planeLocX = 0.0f; planeLocY = 0.0f; planeLocZ = 0.0f;
	setupVertices();
}

void setupVertices()
{
	float planeVertexPositions[18] =
	{
	   -10.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 10.0f,
	   -10.0f, 0.0f, 10.0f,

	   -10.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 10.0f
	};

	float lineVertexPositions[6] =
	{
		0.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 0.0f
	};

	float xlineVertexPositions[6] =
	{
		0.0f, 0.0f, 0.0f,
		5.0f, 0.0f, 0.0f
	};

	float ylineVertexPositions[6] =
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 5.0f, 0.0f
	};

	float zlineVertexPositions[6] =
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 5.0f
	};

	planeNumVertices = 6;
	lineNumVertices = 2;

	// Init VAO and VBO 
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertexPositions), planeVertexPositions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertexPositions), lineVertexPositions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xlineVertexPositions), xlineVertexPositions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ylineVertexPositions), ylineVertexPositions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(zlineVertexPositions), zlineVertexPositions, GL_STATIC_DRAW);
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	pLoc = glGetUniformLocation(renderingProgram, "p_matrix");
	isLineLoc = glGetUniformLocation(renderingProgram, "isLine");
	isRowLoc = glGetUniformLocation(renderingProgram, "isRow");
	isAxesLoc = glGetUniformLocation(renderingProgram, "isAxes");

	// Set up frustum and perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 is about 60 degrees in radians

	// View, Model, and Model-View matrices
	//vMat = glm::translate(glm::mat4(1.0f), -cameraLoc);
	//vMat = glm::rotate(vMat, toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//vMat = glm::lookAt(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(planeLocX, planeLocY, planeLocZ), glm::vec3(0.0f, 0.0f, -1.0f));
	
	vMat = glm::rotate(glm::mat4(1.0f), toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	vMat = glm::rotate(vMat, toRadians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	vMat = glm::translate(vMat, -cameraLoc);
	/*vMat = glm::lookAt(
		glm::vec3(0.0f, 20.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	);*/
	

	// ----------------------------------------- Plane -----------------------------------------
	//pmMat = glm::rotate(glm::mat4(1.0f), toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//pmMat = glm::translate(glm::mat4(1.0f), glm::vec3(planeLocX, planeLocY, planeLocZ));
	pmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 2.0f, 1.0f));
	pmMat = glm::translate(pmMat, vector3(planeLocX, planeLocY, planeLocZ));//convert(glm::vec3(planeLocX, planeLocY, planeLocZ)));
	//pmMat = glm::rotate(pmMat, toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mvMat = vMat * pmMat;

	isLine = false;
	isRow = false;
	glUniform1i(isRowLoc, isRow);
	glUniform1i(isLineLoc, isLine);

	// Model-View matrix pointer sent to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// Bind vertex attribute to vbo[0] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, planeNumVertices);


	//// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------
	//vlmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 2.0f, 1.0f));
	////vlmMat = glm::rotate(vlmMat, toRadians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//vlmMat = glm::translate(vlmMat, vector3(10.0f, 2.0f, 0.0f));
	////vlmMat = glm::rotate(vlmMat, toRadians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//mvMat = vMat * vlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//isLine = true;
	//glUniform1i(isLineLoc, isLine);

	//// Bind vertex attribute to vbo[1] values and enable vertex attribute
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 201);
	//isLine = false;
	//glUniform1i(isLineLoc, isLine);

	//// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------
	//isRow = true;
	//glUniform1i(isRowLoc, isRow);

	//hlmMat = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 1.0f, 1.0f));
	//hlmMat = glm::translate(hlmMat, vector3(2.5f, 14.0f, 0.0f));
	//mvMat = vMat * hlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 101);
	//
	//isRow = false;
	//glUniform1i(isRowLoc, isRow);

	//// ----------------------------------------- Axes -----------------------------------------
	//// ----------------------------------------- X-axis (Red)
	//isAxes = true;
	//glUniform1i(isAxesLoc, isAxes);

	//amMat = glm::rotate(glm::mat4(1.0f), toRadians(165.0f), vector3(0.0f, 1.0f, 0.0f));
	//amMat = glm::translate(amMat, vector3(0.0f, -5.0f, 0.0f));
	////amMat = glm::rotate(amMat, toRadians(90.0f), xAxis);
	//mvMat = vMat * amMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINES, 0, lineNumVertices);
	//isAxes = false;
	//glUniform1i(isAxesLoc, isAxes);

	//// ----------------------------------------- Y-axis (World) - (Z-axis converted) (Green)
	//isRow = true;
	//glUniform1i(isRowLoc, isRow);

	//amMat = glm::rotate(glm::mat4(1.0f), toRadians(-15.0f), vector3(0.0f, 1.0f, 0.0f));
	//amMat = glm::translate(amMat, vector3(0.0f, -5.0f, 0.0f));
	//mvMat = vMat * amMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINES, 0, lineNumVertices);
	//isRow = false;
	//glUniform1i(isRowLoc, isRow);

	//// ----------------------------------------- Z-axis (World) - (Y-axis converted) (Blue)
	//isLine = true;
	//glUniform1i(isLineLoc, isLine);

	////amMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 2.0f));
	////amMat = glm::rotate(glm::mat4(1.0f), toRadians(45.0f), vector3(0.0f, 0.0f, 0.0f));
	//amMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, -5.0f, 0.0f));
	//
	////amMat = glm::rotate(amMat, toRadians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvMat = vMat * amMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINES, 0, lineNumVertices);
	//isLine = false;
	//glUniform1i(isLineLoc, isLine);
}

glm::vec3 convert(glm::vec3 ogVec)
{
	return glm::vec3(-ogVec.x, ogVec.z, ogVec.y);
}

glm::vec3 vector3(float x, float y, float z)
{
	return glm::vec3(-x, z, y);
}