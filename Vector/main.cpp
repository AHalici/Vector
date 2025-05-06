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
#include "CameraController.h"

using namespace std;

#define numVAOs 1
#define numVBOs 5

GLuint renderingProgram1, renderingProgram2;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Display variables
GLuint mvLoc, pLoc;
int width, height;
float aspect;
glm::mat4 pmMat, vlmMat, hlmMat, amMat, vMat, pMat, mvMat, cameraTMat, cameraRMat;

// Camera
//glm::vec3 cameraLoc;
glm::vec3 negativeCameraPosition;
CameraController cameraController;

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
void setupCamera();
void updateCamera();

// Key press listeners for camera controller
void onWKeyPressed();
void onSKeyPressed();
void onAKeyPressed();
void onDKeyPressed();
void onZKeyPressed();
void onCKeyPressed();
void onEKeyPressed();
void onQKeyPressed();
void onEscKeyPressed();


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

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			// Call the function continuously while W is held down
			onWKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			// Call the function continuously while S is held down
			onSKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			// Call the function continuously while A is held down
			onAKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			// Call the function continuously while D is held down
			onDKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			// Call the function continuously while Z is held down
			onZKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			// Call the function continuously while C is held down
			onCKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			// Call the function continuously while E is held down
			onEKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			// Call the function continuously while Q is held down
			onQKeyPressed();
		}
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			// Call the function continuously while Esc is held down
			onEscKeyPressed();
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void init(GLFWwindow* window)
{
	renderingProgram1 = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	/*cameraController.setPosition(0.0f, 20.0f, 10.0f);
	cameraController.setOrientation(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));*/
	//cameraLoc = glm::vec3(0.0f, 20.0f, 10.0f);
	planeLocX = 0.0f; planeLocY = 0.0f; planeLocZ = 0.0f;

	mvLoc = glGetUniformLocation(renderingProgram1, "mv_matrix");
	pLoc = glGetUniformLocation(renderingProgram1, "p_matrix");
	isLineLoc = glGetUniformLocation(renderingProgram1, "isLine");
	isRowLoc = glGetUniformLocation(renderingProgram1, "isRow");
	isAxesLoc = glGetUniformLocation(renderingProgram1, "isAxes");

	setupCamera();
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

	glUseProgram(renderingProgram1);


	// Set up frustum and perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 is about 60 degrees in radians


	/*cameraTMat = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		cameraController.getPosition().x, cameraController.getPosition().y, cameraController.getPosition().z, 1.0f
	);

	negativeCameraPosition = -cameraController.getPosition();
	cameraTMat = glm::translate(glm::mat4(1.0f), negativeCameraPosition);

	cameraRMat = glm::mat4(
		cameraController.getU().x, cameraController.getV().x, -cameraController.getN().x, 0.0f,
		cameraController.getU().y, cameraController.getV().y, -cameraController.getN().y, 0.0f,
		cameraController.getU().z, cameraController.getV().z, -cameraController.getN().z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);*/



	// View, Model, and Model-View matrices
	//vMat = glm::translate(glm::mat4(1.0f), -cameraLoc);
	//vMat = glm::rotate(vMat, toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//vMat = glm::lookAt(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(planeLocX, planeLocY, planeLocZ), glm::vec3(0.0f, 0.0f, -1.0f));
	


	/*vMat = glm::rotate(glm::mat4(1.0f), toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	vMat = glm::rotate(vMat, toRadians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	vMat = glm::translate(vMat, -cameraLoc);*/

	updateCamera();
	vMat = cameraRMat * cameraTMat;

	/*vMat = glm::lookAt(
		glm::vec3(0.0f, 20.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	);*/
	

	// ----------------------------------------- Plane -----------------------------------------
	pmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 2.0f, 1.0f));
	pmMat = glm::translate(pmMat, vector3(planeLocX, planeLocY, planeLocZ));
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


	// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------
	vlmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 4.0f, 1.0f));
	//vlmMat = glm::rotate(vlmMat, toRadians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	vlmMat = glm::translate(vlmMat, vector3(10.0f, 0.0f, 0.0f));
	//vlmMat = glm::rotate(vlmMat, toRadians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvMat = vMat * vlmMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	isLine = true;
	glUniform1i(isLineLoc, isLine);

	// Bind vertex attribute to vbo[1] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	isLine = false;
	glUniform1i(isLineLoc, isLine);

	// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------
	isRow = true;
	glUniform1i(isRowLoc, isRow);

	hlmMat = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 2.0f, 1.0f));
	hlmMat = glm::translate(hlmMat, vector3(2.5f, 20.0f, 0.0f));
	mvMat = vMat * hlmMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	
	isRow = false;
	glUniform1i(isRowLoc, isRow);

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

void setupCamera()
{
	cameraController.setPosition(0.0f, 20.0f, 10.0f);
	cameraController.setOrientation(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
}

void updateCamera()
{
	cameraTMat = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		cameraController.getPosition().x, cameraController.getPosition().y, cameraController.getPosition().z, 1.0f
	);

	negativeCameraPosition = -cameraController.getPosition();
	cameraTMat = glm::translate(glm::mat4(1.0f), negativeCameraPosition);

	cameraRMat = glm::mat4(
		cameraController.getU().x, cameraController.getV().x, -cameraController.getN().x, 0.0f,
		cameraController.getU().y, cameraController.getV().y, -cameraController.getN().y, 0.0f,
		cameraController.getU().z, cameraController.getV().z, -cameraController.getN().z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

glm::vec3 convert(glm::vec3 ogVec)
{
	return glm::vec3(-ogVec.x, ogVec.z, ogVec.y);
}

glm::vec3 vector3(float x, float y, float z)
{
	return glm::vec3(-x, z, y);
}

void onWKeyPressed() 
{
	std::cout << "W key pressed" << std::endl;
	cameraController.moveForward();
}


void onSKeyPressed() 
{
	std::cout << "S key pressed" << std::endl;
	cameraController.moveBackward();
}

void onAKeyPressed() 
{
	std::cout << "A key pressed" << std::endl;
	cameraController.moveLeft();
}

void onDKeyPressed() 
{
	std::cout << "D key pressed" << std::endl;
	cameraController.moveRight();
}

void onZKeyPressed() 
{
	std::cout << "Z key pressed" << std::endl;
	cameraController.moveUp();
}

void onCKeyPressed() 
{
	std::cout << "C key pressed" << std::endl;
	cameraController.moveDown();
}

void onEKeyPressed() 
{
	std::cout << "E key pressed" << std::endl;
	//cameraRMat = glm::rotate(glm::mat4(1.0f), -cameraController.getRotationAngle(), cameraController.getV());

	/*cameraRMat = glm::mat4(
		cameraController.getU().x, cameraController.getV().x, -cameraController.getN().x, 0.0f,
		cameraController.getU().y, cameraController.getV().y, -cameraController.getN().y, 0.0f,
		cameraController.getU().z, cameraController.getV().z, -cameraController.getN().z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);*/
	//cameraController.setU(cameraRMat * glm::vec4(cameraController.getU(), 0.0f));
	//cameraController.setN(cameraRMat * glm::vec4(cameraController.getN(), 0.0f));
	cameraController.rotateRight(cameraRMat);
	//cameraRMat = glm::rotate(cameraRMat, 0.1f, cameraController.getU());
}

void onQKeyPressed() 
{
	std::cout << "Q key pressed" << std::endl;

	//cameraRMat = glm::rotate(glm::mat4(1.0f), cameraController.getRotationAngle(), cameraController.getV());

	//vMat = cameraRMat * cameraTMat;
	//cameraController.setU(cameraRMat * glm::vec4(cameraController.getU(), 0.0f));
	//cameraController.setN(cameraRMat * glm::vec4(cameraController.getN(), 0.0f));

	// -------------------------- GLM Rotation --------------------------
	cameraController.rotateLeft(cameraRMat);
	// ------------------------------------------------------------------

	//// -------------------------- Manual Rotation (using rotation matrix from scratch) --------------------------
	// Rotation around Y/V axis
	//glm::mat3 rotationMatrix(
	//	cos(-cameraController.getRotationAngle()), 0.0f, sin(-cameraController.getRotationAngle()),
	//	0.0f, 1.0f, 0.0f,
	//	-sin(-cameraController.getRotationAngle()), 0.0f, cos(-cameraController.getRotationAngle())
	//);

	//cameraController.setU(rotationMatrix * cameraController.getU());
	//cameraController.setN(rotationMatrix * cameraController.getN());

	//cameraRMat = glm::mat4(
	//	cameraController.getU().x, cameraController.getV().x, -cameraController.getN().x, 0.0f,
	//	cameraController.getU().y, cameraController.getV().y, -cameraController.getN().y, 0.0f,
	//	cameraController.getU().z, cameraController.getV().z, -cameraController.getN().z, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f
	//);
	// ------------------------------------------------------------------------------------------------------------
}

void onEscKeyPressed()
{
	std::cout << "Q key pressed" << std::endl;
	exit(EXIT_SUCCESS);
}