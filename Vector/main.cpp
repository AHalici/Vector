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
#define numVBOs 20

glm::vec3 vector3(float x, float y, float z);

GLuint renderingProgram1, renderingProgram2;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// white light
float globalAmbient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightAmbient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
float lightDiffuse[4] = { 1.2f, 1.2f, 1.2f, 1.0f };
float lightSpecular[4] = { 0.5f, 0.5f, 0.5f, 0.0f };

// gold material
float* gMatAmb = Utils::goldAmbient();
float* gMatDif = Utils::goldDiffuse();
float* gMatSpe = Utils::goldSpecular();
float gMatShi = Utils::goldShininess();

// silver material
float* sMatAmb = Utils::silverAmbient();
float* sMatDif = Utils::silverDiffuse();
float* sMatSpe = Utils::silverSpecular();
float sMatShi = Utils::silverShininess();

// bronze material
float* bMatAmb = Utils::bronzeAmbient();
float* bMatDif = Utils::bronzeDiffuse();
float* bMatSpe = Utils::bronzeSpecular();
float bMatShi = Utils::bronzeShininess();

float thisAmb[4], thisDif[4], thisSpe[4], matAmb[4], matDif[4], matSpe[4];
float thisShi, matShi;

// Display variables
GLuint mvLoc, mLoc, vLoc, pLoc, nLoc, sLoc;
int width, height;
float aspect;
glm::mat4 pmMat, vlmMat, hlmMat, amMat, cmMat, vMat, pMat, mvMat, invTrMat, cameraTMat, cameraRMat;
glm::vec3 currentLightPos;
float lightPos[3];
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
float yBounds, xBounds;
float timePassed;
float previousTime;
//glm::vec3 cubeLoc = vector3(2.0f * 0.0f, yBounds * 2.0f, 2.0f * 2.2f);
glm::vec3 lightLoc = vector3(0.0f, 0.0f, 10.0f); // If I make the light closer to the plane with the z value, the fov of the lightPMatrix isn't wide enough and the shadow has artifacts
//glm::vec3 lightLoc = glm::vec3(0.0f, 30.0f, 5.0f);
glm::vec3 cubeLoc, cubeLoc2, cubeSpawnLocation;// = vector3(2.0f * 0.0f, yBounds * 4.0f, 2.0f * 2.2f);

// Camera
glm::vec3 negativeCameraPosition;
CameraController cameraController;

// Object Locations
float planeLocX, planeLocY, planeLocZ;

// Object number of vertices
unsigned int planeNumVertices;
unsigned int lineNumVertices;
unsigned int cubeNumVertices;

// Tool variables
bool isLine = false;
bool isRow = false;
bool isAxes = false;
GLuint isLineLoc, isRowLoc, isAxesLoc;

// Cube motion
bool goingUp = true;
bool goingRight = true;
float cubeSpeed = 8.0f;

// Shadow things
int scSizeX, scSizeY;
GLuint shadowTex, shadowBuffer;
glm::mat4 lightVMatrix;
glm::mat4 lightPMatrix;
glm::mat4 shadowMVP1;
glm::mat4 shadowMVP2;
glm::mat4 b;

// Mouse things
double mouseX, mouseY;
bool isLeftClick = false;
GLuint leftClick = 0;

float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
void setupVertices();
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
glm::vec3 convert(glm::vec3 ogVec);
void setupCamera();
void updateCamera();

void installLights(int renderingProgram);
void setupShadowBuffers(GLFWwindow* window);

void passOne(double time);
void passTwo(double time);

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

void spawnCube();

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	//{
	//	double xpos, ypos;
	//	glfwGetCursorPos(window, &xpos, &ypos);
	//	glfwGetFramebufferSize(window, &width, &height);

	//	float mouseX = (2.0f * xpos) / width - 1.0f;
	//	float mouseY = 1.0f - (2.0f * ypos) / height;

	//	std::cout << "=== DEBUG MOUSE CLICK ===" << std::endl;
	//	std::cout << "Screen: (" << xpos << ", " << ypos << ")" << std::endl;
	//	std::cout << "NDC: (" << mouseX << ", " << mouseY << ")" << std::endl;

	//	// Print camera info
	//	glm::vec3 camPos = cameraController.getPosition();
	//	std::cout << "Camera Pos (your coord): (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")" << std::endl;

	//	// Print camera vectors
	//	std::cout << "Camera U: (" << cameraController.getU().x << ", " << cameraController.getU().y << ", " << cameraController.getU().z << ")" << std::endl;
	//	std::cout << "Camera V: (" << cameraController.getV().x << ", " << cameraController.getV().y << ", " << cameraController.getV().z << ")" << std::endl;
	//	std::cout << "Camera N: (" << cameraController.getN().x << ", " << cameraController.getN().y << ", " << cameraController.getN().z << ")" << std::endl;

	//	// Test: What should happen for center screen click?
	//	if (abs(mouseX) < 0.1f && abs(mouseY) < 0.1f) {
	//		std::cout << "CENTER CLICK - Should spawn at (0,0,0)!" << std::endl;
	//	}
	//}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetFramebufferSize(window, &width, &height);

		// Convert to NDC
		float mouseX = (2.0f * xpos) / width - 1.0f;
		float mouseY = 1.0f - (2.0f * ypos) / height;

		// Create points at near and far planes: CLIP SPACE???
		glm::vec4 nearPoint = glm::vec4(mouseX, mouseY, -1.0f, 1.0f);
		glm::vec4 farPoint = glm::vec4(mouseX, mouseY, 1.0f, 1.0f);


		//glm::mat4 invPV = glm::inverse(pMat * vMat); 
			// Can be done in one step like this. 
				// ASK WHY IT'S NOT vMat * pMat if we read from right to left since pMat should be resolved first
		
		// Inverse Perspective Matrix
		glm::vec4 nearWorld4 = glm::inverse(pMat) * nearPoint;
		glm::vec4 farWorld4 = glm::inverse(pMat) * farPoint;

		// Perform perspective divide
			// Can be done after inverse of vMat as well
		nearWorld4 /= nearWorld4.w;
		farWorld4 /= farWorld4.w;

		// Convert from View Space to World Space (Inverse View Matrix)
		glm::vec4 nearWorld4W = glm::inverse(vMat) * nearWorld4;
		glm::vec4 farWorld4W = glm::inverse(vMat) * farWorld4;

		// ------------------------------------- Inverse Perspective and View Matrices at the same time --------------------
		// -----------------------------------------------------------------------------------------------------------------
		//// Unproject both points in one step
		//glm::mat4 invPV = glm::inverse(pMat * vMat);
		//glm::vec4 nearWorld4 = invPV * nearPoint;
		//glm::vec4 farWorld4 = invPV * farPoint;

		//// Perform perspective divide
		//nearWorld4 /= nearWorld4.w;
		//farWorld4 /= farWorld4.w;

		// -----------------------------------------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------------------------------------
		
		// Convert to vec3
		glm::vec3 nearWorld(nearWorld4W);
		glm::vec3 farWorld(farWorld4W);

		// Create ray
		glm::vec3 rayDir = glm::normalize(farWorld - nearWorld);

		// Get camera position
		glm::vec3 cameraPos = cameraController.getPosition();

		// Calculate plane intersection
		float t = -cameraPos.y / rayDir.y;

		// Compute intersection
		glm::vec3 intersection = cameraPos + t * rayDir;

		// Set spawn location
		cubeSpawnLocation = intersection;
		cubeSpawnLocation.y += 0.5f;

		std::cout << "NDC: (" << mouseX << ", " << mouseY << ")  World: ("
			<< intersection.x << ", " << intersection.y << ", "
			<< intersection.z << ")" << std::endl;

		spawnCube();
	}
}

void spawnCube()
{
	//leftClick++;
	isLeftClick = true;


}

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

	glfwSetMouseButtonCallback(window, mouse_button_callback);

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
	renderingProgram1 = Utils::createShaderProgram("vertShader1.glsl", "fragShader1.glsl");
	renderingProgram2 = Utils::createShaderProgram("vertShader2.glsl", "fragShader2.glsl");

	// Set up frustum and perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(toRadians(60.0f), aspect, 0.1f, 1000.0f); // 1.0472 is about 60 degrees in radians

	/*cameraController.setPosition(0.0f, 20.0f, 10.0f);
	cameraController.setOrientation(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));*/
	//cameraLoc = glm::vec3(0.0f, 20.0f, 10.0f);
	planeLocX = 0.0f; planeLocY = 0.0f; planeLocZ = 0.0f;


	// TODO: changed to renderingProgram2
	/*mLoc = glGetUniformLocation(renderingProgram2, "m_matrix");
	vLoc = glGetUniformLocation(renderingProgram2, "v_matrix");
	mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	pLoc = glGetUniformLocation(renderingProgram2, "p_matrix");
	isLineLoc = glGetUniformLocation(renderingProgram2, "isLine");
	isRowLoc = glGetUniformLocation(renderingProgram2, "isRow");
	isAxesLoc = glGetUniformLocation(renderingProgram2, "isAxes");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgram2, "shadowMVP");*/

	setupVertices();
	setupCamera();
	//setupVertices();
	setupShadowBuffers(window);

	// Bias Matrix = converts from light projection space [-1,1] to texture coordinates [0,1]
	b = glm::mat4
	(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
	);
}

void setupVertices()
{
	float planeVertexPositions[18] =
	{
	   -10.0f, 0.0f, -10.0f,
		10.0f, 0.0f, 10.0f,
	   -10.0f, 0.0f, 10.0f,

	   -10.0f, 0.0f, -10.0f,
		10.0f, 0.0f, -10.0f,
		10.0f, 0.0f, 10.0f
	};

	float lineVertexPositions[6] =
	{
		0.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 0.0f
	};

	float xlineVertexPositions[6] =
	{
		-10.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 0.0f
	};

	float ylineVertexPositions[6] =
	{
		0.0f, -10.0f, 0.0f,
		0.0f, 10.0f, 0.0f
	};

	float zlineVertexPositions[6] =
	{
		0.0f, 0.0f, -10.0f,
		0.0f, 0.0f, 10.0f
	};

	float cubeVertexPositions[108] = {
		-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
	};

	float cubeNormals[108] = {
		// Front face (negative z) - 6 vertices, all with normal (0,0,-1)
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,

		// Right face (positive x) - 6 vertices, all with normal (1,0,0)
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		// Back face (positive z) - 6 vertices, all with normal (0,0,1)
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

		// Left face (negative x) - 6 vertices, all with normal (-1,0,0)
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

		// Bottom face (negative y) - 6 vertices, all with normal (0,-1,0)
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,

		// Top face (positive y) - 6 vertices, all with normal (0,1,0)
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
	};

	float planeNormals[18] =
	{
		0.0f, 1.0f, 0.0f,  // Normal for vertex (-10.0, 0.0, 0.0)
		0.0f, 1.0f, 0.0f,  // Normal for vertex (10.0, 0.0, 10.0)
		0.0f, 1.0f, 0.0f,  // Normal for vertex (-10.0, 0.0, 10.0)
		0.0f, 1.0f, 0.0f,  // Normal for vertex (-10.0, 0.0, 0.0)
		0.0f, 1.0f, 0.0f,  // Normal for vertex (10.0, 0.0, 0.0)
		0.0f, 1.0f, 0.0f   // Normal for vertex (10.0, 0.0, 10.0)
	};

	float lineNormals[6] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	planeNumVertices = 6;
	lineNumVertices = 2;
	cubeNumVertices = 36;

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

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeNormals), planeNormals, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineNormals), lineNormals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	currentLightPos = lightLoc;


	timePassed = currentTime - previousTime;
	previousTime = currentTime;

	if (cubeLoc.z > 9.0f)
		goingUp = false;
	else if (cubeLoc.z < -9.0f)
		goingUp = true;

	if (cubeLoc2.x > 9.0f)
		goingRight = false;
	else if (cubeLoc2.x < -9.0f)
		goingRight = true;
		

	if (goingUp)
		yBounds += timePassed;
	else if (!goingUp)
		yBounds -= timePassed;

	if (goingRight)
		xBounds += timePassed;
	else if (!goingRight)
		xBounds -= timePassed;

	/*cout << "This is yBounds: " << yBounds << endl;
	cout << "This is xBounds: " << xBounds << endl;
	cout << "This is timePassed: " << timePassed << endl;*/

	cubeLoc = vector3(2.0f, yBounds * cubeSpeed, 3.0f);
	cubeLoc2 = vector3(xBounds * cubeSpeed, -2.0f, 3.0f);

	// Sets the origin of the lightVMatrix to the higher of the two (9.0f or cubeLoc.z)
	float highestPoint = std::max(10.0f, cubeLoc.y);
	glm::vec3 lightTarget = vector3(0.0f, highestPoint, 0.0f);

	lightVMatrix = glm::lookAt(currentLightPos, vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));
	
	// Changed to Orthographic Projection because it's better for directional lighting, but not for positional or spotlight
		// Using Orthographic projection doesn't allow shadow to change angles when the object is moved or the light moves
	//float orthoSize = 8.0f;
	//lightPMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize + 5.0f, 1.0f, 50.0f);
	lightPMatrix = glm::perspective(toRadians(90.0f), aspect, 0.1f, 1000.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL); //--------------------- JUST REMOVED ---------------
	glPolygonOffset(2.0f, 4.0f); // ------------------- JUST REMOVED -------------

	passOne(currentTime);

	glDisable(GL_POLYGON_OFFSET_FILL); //-------------- JUST REMOVED -------------

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glDrawBuffer(GL_FRONT);

	passTwo(currentTime);

	// -----------------------------------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------------------------------

	//glUseProgram(renderingProgram1);


	///*cameraTMat = glm::mat4(
	//	1.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, 1.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 0.0f,
	//	cameraController.getPosition().x, cameraController.getPosition().y, cameraController.getPosition().z, 1.0f
	//);

	//negativeCameraPosition = -cameraController.getPosition();
	//cameraTMat = glm::translate(glm::mat4(1.0f), negativeCameraPosition);

	//cameraRMat = glm::mat4(
	//	cameraController.getU().x, cameraController.getV().x, -cameraController.getN().x, 0.0f,
	//	cameraController.getU().y, cameraController.getV().y, -cameraController.getN().y, 0.0f,
	//	cameraController.getU().z, cameraController.getV().z, -cameraController.getN().z, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f
	//);*/



	//// View, Model, and Model-View matrices
	////vMat = glm::translate(glm::mat4(1.0f), -cameraLoc);
	////vMat = glm::rotate(vMat, toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	////vMat = glm::lookAt(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(planeLocX, planeLocY, planeLocZ), glm::vec3(0.0f, 0.0f, -1.0f));
	//


	///*vMat = glm::rotate(glm::mat4(1.0f), toRadians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//vMat = glm::rotate(vMat, toRadians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//vMat = glm::translate(vMat, -cameraLoc);*/

	//updateCamera();
	//vMat = cameraRMat * cameraTMat;

	///*vMat = glm::lookAt(
	//	glm::vec3(0.0f, 20.0f, 0.0f),
	//	glm::vec3(0.0f, 0.0f, 0.0f),
	//	glm::vec3(0.0f, 0.0f, -1.0f)
	//);*/
	//

	//// ----------------------------------------- Plane -----------------------------------------
	//pmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 2.0f, 1.0f));
	//pmMat = glm::translate(pmMat, vector3(planeLocX, planeLocY, planeLocZ));
	//mvMat = vMat * pmMat;

	//isLine = false;
	//isRow = false;
	//glUniform1i(isRowLoc, isRow);
	//glUniform1i(isLineLoc, isLine);

	//// Model-View matrix pointer sent to uniforms
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	//glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	//// Bind vertex attribute to vbo[0] values and enable vertex attribute
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	//glDrawArrays(GL_TRIANGLES, 0, planeNumVertices);


	//// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------
	//vlmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 4.0f, 1.0f));
	////vlmMat = glm::rotate(vlmMat, toRadians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//vlmMat = glm::translate(vlmMat, vector3(10.0f, 0.0f, 0.0f));
	////vlmMat = glm::rotate(vlmMat, toRadians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//mvMat = vMat * vlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//isLine = true;
	//glUniform1i(isLineLoc, isLine);

	//// Bind vertex attribute to vbo[1] values and enable vertex attribute
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	//isLine = false;
	//glUniform1i(isLineLoc, isLine);

	//// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------
	//isRow = true;
	//glUniform1i(isRowLoc, isRow);

	//hlmMat = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 2.0f, 1.0f));
	//hlmMat = glm::translate(hlmMat, vector3(2.5f, 20.0f, 0.0f));
	//mvMat = vMat * hlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
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

void passOne(double time)
{
	glUseProgram(renderingProgram1);

	//yBounds = (float)time;

	//cubeLoc = vector3(2.0f * 0.0f, yBounds * 4.0f, 2.0f * 2.2f);
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));
	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f)); // When we scale by 0.5, our translation vectors are also scaled, so 10.0f from before becomes 5.0f
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));
	//cmMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, 10.0f, 2.0f));


	shadowMVP1 = lightPMatrix * lightVMatrix * cmMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[7] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);

	//cmMat = glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc2) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));

	shadowMVP1 = lightPMatrix * lightVMatrix * cmMat;
	//sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[7] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	////glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);

	if (isLeftClick)// && leftClick != 0)
	{
		//cmMat = glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
		cmMat = glm::translate(glm::mat4(1.0f), cubeSpawnLocation) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
		//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));

		shadowMVP1 = lightPMatrix * lightVMatrix * cmMat;
		//sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
		glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

		// Bind vertex attribute to vbo[7] values and enable vertex attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);

		////glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);
	}

	//// ----------------------------------------- Plane -----------------------------------------

	//pmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 2.0f, 1.0f));
	pmMat = glm::translate(glm::mat4(1.0f), vector3(planeLocX, planeLocY, planeLocZ));

	shadowMVP1 = lightPMatrix * lightVMatrix * pmMat;
	//sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[0] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	//glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, planeNumVertices);

	
	// -----------------------------------------------------------------------------------------
	// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------

	isLine = true;
	glUniform1i(isLineLoc, isLine);

	//vlmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 4.0f, 1.0f));
	vlmMat = glm::translate(glm::mat4(1.0f), vector3(-10.0f, 0.0f, 0.0f));
	//mvMat = vMat * vlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	shadowMVP1 = lightPMatrix * lightVMatrix * vlmMat;
	//sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[0] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	//glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	isLine = false;
	glUniform1i(isLineLoc, isLine);


	// -----------------------------------------------------------------------------------------
	// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------

	isRow = true;
	glUniform1i(isRowLoc, isRow);

	//hlmMat = glm::scale(glm::mat4(1.0f), vector3(4.0f, 1.0f, 1.0f));
	hlmMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, -10.0f, 0.0f));

	/*hlmMat = glm::scale(glm::mat4(1.0f), vector3(4.0f, 2.0f, 1.0f));
	hlmMat = glm::translate(hlmMat, vector3(-2.5f, -5.0f, 0.0f));*/


	//mvMat = vMat * vlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	shadowMVP1 = lightPMatrix * lightVMatrix * hlmMat;
	//sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[0] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	//glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	isRow = false;
	glUniform1i(isRowLoc, isRow);

}

void passTwo(double time)
{
	glUseProgram(renderingProgram2);

	//yBounds = (float)time;

	mLoc = glGetUniformLocation(renderingProgram2, "m_matrix");
	vLoc = glGetUniformLocation(renderingProgram2, "v_matrix");
	mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	pLoc = glGetUniformLocation(renderingProgram2, "p_matrix");
	isLineLoc = glGetUniformLocation(renderingProgram2, "isLine");
	isRowLoc = glGetUniformLocation(renderingProgram2, "isRow");
	isAxesLoc = glGetUniformLocation(renderingProgram2, "isAxes");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgram2, "shadowMVP");

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	// -----------------------------------------------------------------------------------------
	// ----------------------------------------- Cube -----------------------------------------
	
	thisAmb[0] = bMatAmb[0]; thisAmb[1] = bMatAmb[1]; thisAmb[2] = bMatAmb[2];  // bronze
	thisDif[0] = bMatDif[0]; thisDif[1] = bMatDif[1]; thisDif[2] = bMatDif[2];
	thisSpe[0] = bMatSpe[0]; thisSpe[1] = bMatSpe[1]; thisSpe[2] = bMatSpe[2];
	thisShi = bMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	//cubeLoc = vector3(2.0f * 0.0f, yBounds * 4.0f, 2.0f * 2.2f);

	//cmMat = glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f)); // When we scale by 0.5, our translation vectors are also scaled, so 10.0f from before becomes 5.0f
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));
	//cmMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, 10.0f, 2.0f));

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	invTrMat = glm::transpose(glm::inverse(cmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * cmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(cmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	// Cube VERTICES
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	// Cube NORMALS
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);



	thisAmb[0] = bMatAmb[0]; thisAmb[1] = bMatAmb[1]; thisAmb[2] = bMatAmb[2];  // bronze
	thisDif[0] = bMatDif[0]; thisDif[1] = bMatDif[1]; thisDif[2] = bMatDif[2];
	thisSpe[0] = bMatSpe[0]; thisSpe[1] = bMatSpe[1]; thisSpe[2] = bMatSpe[2];
	thisShi = bMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	//cmMat = glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc2) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f)); // When we scale by 0.5, our translation vectors are also scaled, so 10.0f from before becomes 5.0f
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));
	//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	invTrMat = glm::transpose(glm::inverse(cmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * cmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(cmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	// Cube VERTICES
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	// Cube NORMALS
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);


	if (isLeftClick)
	{
		thisAmb[0] = bMatAmb[0]; thisAmb[1] = bMatAmb[1]; thisAmb[2] = bMatAmb[2];  // bronze
		thisDif[0] = bMatDif[0]; thisDif[1] = bMatDif[1]; thisDif[2] = bMatDif[2];
		thisSpe[0] = bMatSpe[0]; thisSpe[1] = bMatSpe[1]; thisSpe[2] = bMatSpe[2];
		thisShi = bMatShi;

		// Set VIEW matrix
		updateCamera();
		vMat = cameraRMat * cameraTMat;

		//cmMat = glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
		cmMat = glm::translate(glm::mat4(1.0f), cubeSpawnLocation) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f)); // When we scale by 0.5, our translation vectors are also scaled, so 10.0f from before becomes 5.0f
		//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));
		//cmMat = glm::scale(cmMat, vector3(0.5f, 0.5f, 0.5f));

		currentLightPos = lightLoc;
		installLights(renderingProgram2);

		invTrMat = glm::transpose(glm::inverse(cmMat));
		shadowMVP2 = b * lightPMatrix * lightVMatrix * cmMat;

		glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(cmMat));
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
		//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
		glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

		// Cube VERTICES
		glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);

		// Cube NORMALS
		glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);
		//isLeftClick = false;
	}
	// ----------------------------------------- Plane -----------------------------------------

	thisAmb[0] = sMatAmb[0]; thisAmb[1] = sMatAmb[1]; thisAmb[2] = sMatAmb[2];  // silver
	thisDif[0] = sMatDif[0]; thisDif[1] = sMatDif[1]; thisDif[2] = sMatDif[2];
	thisSpe[0] = sMatSpe[0]; thisSpe[1] = sMatSpe[1]; thisSpe[2] = sMatSpe[2];
	thisShi = sMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	//pmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 2.0f, 1.0f));
	pmMat = glm::translate(glm::mat4(1.0f), vector3(planeLocX, planeLocY, planeLocZ));

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	invTrMat = glm::transpose(glm::inverse(pmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * pmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(pmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	// Plane VERTICES
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	// Plane NORMALS
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	//glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, planeNumVertices);

	// -----------------------------------------------------------------------------------------
	// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------

	thisAmb[0] = gMatAmb[0]; thisAmb[1] = gMatAmb[1]; thisAmb[2] = gMatAmb[2];  // gold
	thisDif[0] = gMatDif[0]; thisDif[1] = gMatDif[1]; thisDif[2] = gMatDif[2];
	thisSpe[0] = gMatSpe[0]; thisSpe[1] = gMatSpe[1]; thisSpe[2] = gMatSpe[2];
	thisShi = gMatShi;

	//vlmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 4.0f, 1.0f));
	vlmMat = glm::translate(glm::mat4(1.0f), vector3(-10.0f, 0.0f, 0.0f));
	//mvMat = vMat * vlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	invTrMat = glm::transpose(glm::inverse(vlmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * vlmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(vlmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));
	
	isLine = true;
	glUniform1i(isLineLoc, isLine);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	isLine = false;
	glUniform1i(isLineLoc, isLine);

	// -----------------------------------------------------------------------------------------
	// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------
	
	thisAmb[0] = gMatAmb[0]; thisAmb[1] = gMatAmb[1]; thisAmb[2] = gMatAmb[2];  // gold
	thisDif[0] = gMatDif[0]; thisDif[1] = gMatDif[1]; thisDif[2] = gMatDif[2];
	thisSpe[0] = gMatSpe[0]; thisSpe[1] = gMatSpe[1]; thisSpe[2] = gMatSpe[2];
	thisShi = gMatShi;
	
	isRow = true;
	glUniform1i(isRowLoc, isRow);

	//hlmMat = glm::scale(glm::mat4(1.0f), vector3(4.0f, 1.0f, 1.0f));
	hlmMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, -10.0f, 0.0f)); // The position is different than the vertical lines because of the 4.0f scaling on the x axis
	//mvMat = vMat * hlmMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	invTrMat = glm::transpose(glm::inverse(hlmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * hlmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(hlmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

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
	////mvMat = vMat * amMat;

	//currentLightPos = lightLoc;
	//installLights(renderingProgram2);

	//invTrMat = glm::transpose(glm::inverse(amMat));
	//shadowMVP2 = b * lightPMatrix * lightVMatrix * amMat;

	//glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(amMat));
	//glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	//glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	//glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

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
	///*mvMat = vMat * amMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));*/

	//currentLightPos = lightLoc;
	//installLights(renderingProgram2);

	//invTrMat = glm::transpose(glm::inverse(amMat));
	//shadowMVP2 = b * lightPMatrix * lightVMatrix * amMat;

	//glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(amMat));
	//glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	//glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	//glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

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
	///*mvMat = vMat * amMat;

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));*/


	//currentLightPos = lightLoc;
	//installLights(renderingProgram2);

	//invTrMat = glm::transpose(glm::inverse(amMat));
	//shadowMVP2 = b * lightPMatrix * lightVMatrix * amMat;

	//glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(amMat));
	//glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	//glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	//glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINES, 0, lineNumVertices);
	//isLine = false;
	//glUniform1i(isLineLoc, isLine);
}

void setupCamera()
{
	cameraController.setPosition(0.0f, 20.0f, 0.0f);
	//cameraController.setOrientation(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	cameraController.setOrientation(vector3(1.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f));

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

	// FIX: Camera basis vectors should be ROWS, not columns
	/*cameraRMat = glm::mat4(
		cameraController.getU().x, cameraController.getU().y, cameraController.getU().z, 0.0f,
		cameraController.getV().x, cameraController.getV().y, cameraController.getV().z, 0.0f,
		-cameraController.getN().x, -cameraController.getN().y, -cameraController.getN().z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);*/
}

glm::vec3 convert(glm::vec3 ogVec)
{
	return glm::vec3(-ogVec.x, ogVec.z, ogVec.y);
}

glm::vec3 vector3(float x, float y, float z)
{
	return glm::vec3(x, z, y);
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

void installLights(int renderingProgram)
{
	lightPos[0] = currentLightPos.x;
	lightPos[1] = currentLightPos.y;
	lightPos[2] = currentLightPos.z;

	matAmb[0] = thisAmb[0]; matAmb[1] = thisAmb[1]; matAmb[2] = thisAmb[2]; matAmb[3] = thisAmb[3];
	matDif[0] = thisDif[0]; matDif[1] = thisDif[1]; matDif[2] = thisDif[2]; matDif[3] = thisDif[3];
	matSpe[0] = thisSpe[0]; matSpe[1] = thisSpe[1]; matSpe[2] = thisSpe[2]; matSpe[3] = thisSpe[3];
	matShi = thisShi;

	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "light.position");
	mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

	//  set the uniform light and material values in the shader
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
	glProgramUniform4fv(renderingProgram, mambLoc, 1, matAmb);
	glProgramUniform4fv(renderingProgram, mdiffLoc, 1, matDif);
	glProgramUniform4fv(renderingProgram, mspecLoc, 1, matSpe);
	glProgramUniform1f(renderingProgram, mshiLoc, matShi);
}

void setupShadowBuffers(GLFWwindow* window)
{
	glfwGetFramebufferSize(window, &width, &height);
	scSizeX = width;
	scSizeY = height;

	glGenFramebuffers(1, &shadowBuffer);

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, scSizeX, scSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// may reduce shadow border artifacts
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}