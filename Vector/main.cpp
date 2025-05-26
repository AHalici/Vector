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
float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

GLuint renderingProgram1, renderingProgram2;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Camera
glm::mat4 cameraTMat, cameraRMat;
glm::vec3 negativeCameraPosition;
CameraController cameraController;

// Display
int width, height;
float aspect;

// Lighting (White Light)
float globalAmbient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.2f, 1.2f, 1.2f, 1.0f };
float lightSpecular[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float lightDirection[3] = { 0.0f, 0.0f, -1.0f };

GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, directionLoc, cutoffLoc, exponentLoc;
glm::vec3 currentLightPos;
glm::vec3 lightLoc = vector3(0.0f, 0.0f, 0.1f); // If I make the light closer to the plane with the z value, the fov of the lightPMatrix isn't wide enough and the shadow has artifacts
float lightPos[3];
float spotlightCutoff = 30.0f;
float spotLightExponent = 4.0f;

// Shadows
int scSizeX, scSizeY;
GLuint sLoc1, sLoc2;
GLuint shadowTex, shadowBuffer;
glm::mat4 lightVMatrix;
glm::mat4 lightPMatrix;
glm::mat4 shadowMVP1;
glm::mat4 shadowMVP2;
glm::mat4 b;


// Gold
float* gMatAmb = Utils::goldAmbient();
float* gMatDif = Utils::goldDiffuse();
float* gMatSpe = Utils::goldSpecular();
float gMatShi = Utils::goldShininess();

// Silver
float* sMatAmb = Utils::silverAmbient();
float* sMatDif = Utils::silverDiffuse();
float* sMatSpe = Utils::silverSpecular();
float sMatShi = Utils::silverShininess();

// Bronze
float* bMatAmb = Utils::bronzeAmbient();
float* bMatDif = Utils::bronzeDiffuse();
float* bMatSpe = Utils::bronzeSpecular();
float bMatShi = Utils::bronzeShininess();

// Light Green
float* lgMatAmb = Utils::lightGreenAmbient();
float* lgMatDif = Utils::lightGreenDiffuse();
float* lgMatSpe = Utils::lightGreenSpecular();
float lgMatShi = Utils::lightGreenShininess();

float thisAmb[4], thisDif[4], thisSpe[4], matAmb[4], matDif[4], matSpe[4];
float thisShi, matShi;
GLuint mambLoc, mdiffLoc, mspecLoc, mshiLoc;

// Object Locations
float planeLocX, planeLocY, planeLocZ;
glm::vec3 cubeLoc, cubeLoc2, cubeSpawnLocation;

// Object number of vertices
unsigned int planeNumVertices;
unsigned int lineNumVertices;
unsigned int cubeNumVertices;
unsigned int tracklineNumVertices;

// Transformations
GLuint mLoc, vLoc, pLoc, nLoc, mvLoc;
glm::mat4 pmMat, vlmMat, hlmMat, tlmMat, amMat, cmMat, vMat, pMat, mvMat, invTrMat;

// Cube motion
bool goingUp = true;
bool goingRight = true;
float cubeSpeed = 8.0f;

// Tools
float yBounds, xBounds;
float timePassed;
float previousTime;
bool isLine = false;
bool isRow = false;
bool isAxes = false;
GLuint isLineLoc1, isRowLoc1, isAxesLoc1, isLineLoc2, isRowLoc2, isAxesLoc2;

// Mouse
double mouseX, mouseY;
bool spawnInstancedCube = false;
bool leftMouseHeld = false;
bool rightMouseHeld = false;
bool middleMouseHeld = false;


void setupVertices();
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
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
void reloadShaders();

glm::vec3 getMouseWorldIntersection(GLFWwindow* window);
void moveCubeToCursor(GLFWwindow* window);
void directLightToCursor(GLFWwindow* window);
void moveLightToCursor(GLFWwindow* window);

bool rKeyPressed = false;

//void handleAllMouseActions(GLFWwindow* window)
//{
//
//	// Check current mouse states
//	leftMouseHeld = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
//	rightMouseHeld = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
//	middleMouseHeld = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
//
//	// Track state changes
//	wasleftMouseHeld = false;
//	wasrightMouseHeld = false;
//	wasmiddleMouseHeld = false;
//
//	// Only calculate intersection if any button is pressed
//	glm::vec3 intersection;
//	bool needIntersection = leftMouseHeld || rightMouseHeld || middleMouseHeld;
//	if (needIntersection) {
//		intersection = getMouseWorldIntersection(window);
//	}
//
//	// Handle left mouse
//	if (leftMouseHeld && !wasleftMouseHeld) {
//		// Just pressed - spawn cube
//		cubeSpawnLocation = intersection;
//		cubeSpawnLocation.y += 0.5f;
//		spawnCube();
//	}
//
//	if (rightMouseHeld)
//	{
//		// Point light toward intersection using your coordinate system
//			// We have to create a vector from our light to our target location. 
//			// We then normalize that vector because if it wasn't, it would cause different lighting behaviors based on different lengths
//		glm::vec3 currentLight = lightLoc;
//		intersection.y = lightLoc.y; // Fixed at lightLoc.y so that the angle of the light doesn't change and stays at the same height as the light position.
//		glm::vec3 target = intersection;
//		glm::vec3 direction = glm::normalize(target - currentLight);
//
//		/*if (direction.x > direction.z)
//		{
//			direction.x = 0.7f;
//		}
//		else if (direction.z > direction.x)
//		{
//			direction.z = 0.7f;
//		}*/
//		cout << "This is the direction: x: " << direction.x << "  y: " << direction.y << "  z: " << direction.z << endl;
//		lightDirection[0] = direction.x;
//		lightDirection[1] = direction.y;
//		lightDirection[2] = direction.z;
//		installLights(renderingProgram2);
//	}
//	
//	// Handle middle mouse
//	if (middleMouseHeld) {
//		lightLoc = vector3(intersection.x, intersection.z, lightLoc.y);
//		currentLightPos = lightLoc;
//
//		//lightVMatrix = glm::lookAt(currentLightPos, vector3(lightDirection[0], lightDirection[2], lightDirection[1]), vector3(0.0f, 1.0f, 0.0f));
//
//
//		installLights(renderingProgram2);
//	}
//
//	// Update previous states
//	wasleftMouseHeld = leftMouseHeld;
//	wasrightMouseHeld = rightMouseHeld;
//	wasmiddleMouseHeld = middleMouseHeld;
//
//}

int main(void)
{
	// GLFW init
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	
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
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			// Call the function continuously while left mouse is held down
			moveCubeToCursor(window);
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			// Call the function continuously while right mouse is held down
			directLightToCursor(window);
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
			// Call the function continuously while middle mouse is held down
			moveLightToCursor(window);
		}
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			// Ensures reload shaders once per press instead of continuously calling it while R is held
			if (!rKeyPressed) {
				reloadShaders();
				rKeyPressed = true;
			}
		} else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
			rKeyPressed = false;
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

	// DEBUG: Check if shaders compiled successfully
	std::cout << "renderingProgram1 ID: " << renderingProgram1 << std::endl;
	std::cout << "renderingProgram2 ID: " << renderingProgram2 << std::endl;

	// Check for OpenGL errors after shader creation
	if (Utils::checkOpenGLError()) {
		std::cout << "OpenGL error after shader creation!" << std::endl;
	}


	// Set up frustum and perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(toRadians(60.0f), aspect, 0.1f, 1000.0f); // 1.0472 is about 60 degrees in radians

	// Object Location Init
	planeLocX = 0.0f; planeLocY = 0.0f; planeLocZ = 0.0f;

	mLoc = glGetUniformLocation(renderingProgram2, "m_matrix");
	vLoc = glGetUniformLocation(renderingProgram2, "v_matrix");
	//mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	pLoc = glGetUniformLocation(renderingProgram2, "p_matrix");
	isLineLoc2 = glGetUniformLocation(renderingProgram2, "isLine");
	isRowLoc2 = glGetUniformLocation(renderingProgram2, "isRow");
	isAxesLoc2 = glGetUniformLocation(renderingProgram2, "isAxes");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	sLoc2 = glGetUniformLocation(renderingProgram2, "shadowMVP");

	isLineLoc1 = glGetUniformLocation(renderingProgram1, "isLine");
	isRowLoc1 = glGetUniformLocation(renderingProgram1, "isRow");
	isAxesLoc1 = glGetUniformLocation(renderingProgram1, "isAxes");
	sLoc1 = glGetUniformLocation(renderingProgram1, "shadowMVP");


	setupVertices();
	setupCamera();
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

	float tracklineVertexPositions[18] =
	{
		0.0f, 0.0f, 0.0f,
		0.2f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		
		0.0f, 0.0f, 0.0f,
		0.2f, 0.0f, 0.0f,
		0.2f, 0.0f, 1.0f,
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
	tracklineNumVertices = 6;

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

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tracklineVertexPositions), tracklineVertexPositions, GL_STATIC_DRAW);
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

	cubeLoc = vector3(2.0f, yBounds * cubeSpeed, 3.0f);
	cubeLoc2 = vector3(xBounds * cubeSpeed, -2.0f, 3.0f);

	// Calculate target position from light position + direction
	glm::vec3 lightTarget = currentLightPos + vector3(lightDirection[0], lightDirection[2], lightDirection[1]);

	// Check if light direction is parallel to up vector
	glm::vec3 lightDir = glm::normalize(lightTarget - currentLightPos);
	glm::vec3 upVector = vector3(0.0f, 1.0f, 0.0f);

	// If pointing straight up or down, use a different up vector
	if (abs(glm::dot(lightDir, upVector)) > 0.99f) {
		upVector = vector3(1.0f, 0.0f, 0.0f);  // Use X-axis as up instead
	}


	lightVMatrix = glm::lookAt(currentLightPos, lightTarget, upVector);
	lightPMatrix = glm::perspective(toRadians(spotlightCutoff * 2.0f), aspect, 0.1f, 1000.0f);
	

	// Changed to Orthographic Projection because it's better for directional lighting, but not for positional or spotlight
		// Using Orthographic projection doesn't allow shadow to change angles when the object is moved or the light moves
	//float orthoSize = 8.0f;
	//lightPMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize + 5.0f, 1.0f, 50.0f);


	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);

	passOne(currentTime);

	glDisable(GL_POLYGON_OFFSET_FILL);

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

	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f)); // When we scale by 0.5, our translation vectors are also scaled, so 10.0f from before becomes 5.0f
	shadowMVP1 = lightPMatrix * lightVMatrix * cmMat;
	sLoc1 = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

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

	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc2) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
	shadowMVP1 = lightPMatrix * lightVMatrix * cmMat;
	glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[7] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);

	if (spawnInstancedCube)
	{
		cmMat = glm::translate(glm::mat4(1.0f), cubeSpawnLocation) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
		shadowMVP1 = lightPMatrix * lightVMatrix * cmMat;
		glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

		// Bind vertex attribute to vbo[7] values and enable vertex attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, cubeNumVertices);
	}

	//// ----------------------------------------- Plane -----------------------------------------

	pmMat = glm::translate(glm::mat4(1.0f), vector3(planeLocX, planeLocY, planeLocZ));
	shadowMVP1 = lightPMatrix * lightVMatrix * pmMat;
	glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[0] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, planeNumVertices);










	tlmMat = glm::translate(glm::mat4(1.0f), vector3(1.0f, 0.0f, 1.0f));
	shadowMVP1 = lightPMatrix * lightVMatrix * tlmMat;
	glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// Bind vertex attribute to vbo[0] values and enable vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, tracklineNumVertices);


	// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------

	//isLine = true;
	//glUniform1i(isLineLoc1, isLine);

	////vlmMat = glm::scale(glm::mat4(1.0f), vector3(1.0f, 4.0f, 1.0f));
	//vlmMat = glm::translate(glm::mat4(1.0f), vector3(-10.0f, 0.0f, 0.0f));
	////mvMat = vMat * vlmMat;

	////glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//shadowMVP1 = lightPMatrix * lightVMatrix * vlmMat;
	////sLoc1 = glGetUniformLocation(renderingProgram1, "shadowMVP");
	//glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	//// Bind vertex attribute to vbo[0] values and enable vertex attribute
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	////glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	//glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	//isLine = false;
	//glUniform1i(isLineLoc1, isLine);


	//// -----------------------------------------------------------------------------------------
	//// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------

	//isRow = true;
	//glUniform1i(isRowLoc1, isRow);

	////hlmMat = glm::scale(glm::mat4(1.0f), vector3(4.0f, 1.0f, 1.0f));
	//hlmMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, -10.0f, 0.0f));

	///*hlmMat = glm::scale(glm::mat4(1.0f), vector3(4.0f, 2.0f, 1.0f));
	//hlmMat = glm::translate(hlmMat, vector3(-2.5f, -5.0f, 0.0f));*/


	////mvMat = vMat * vlmMat;

	////glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//shadowMVP1 = lightPMatrix * lightVMatrix * hlmMat;
	////sLoc1 = glGetUniformLocation(renderingProgram1, "shadowMVP");
	//glUniformMatrix4fv(sLoc1, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	//// Bind vertex attribute to vbo[0] values and enable vertex attribute
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	////glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	//glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);
	//isRow = false;
	//glUniform1i(isRowLoc1, isRow);

}

void passTwo(double time)
{
	glUseProgram(renderingProgram2);

	// ----------------------------------------- Cube -----------------------------------------
	
	thisAmb[0] = bMatAmb[0]; thisAmb[1] = bMatAmb[1]; thisAmb[2] = bMatAmb[2];  // bronze
	thisDif[0] = bMatDif[0]; thisDif[1] = bMatDif[1]; thisDif[2] = bMatDif[2];
	thisSpe[0] = bMatSpe[0]; thisSpe[1] = bMatSpe[1]; thisSpe[2] = bMatSpe[2];
	thisShi = bMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));

	invTrMat = glm::transpose(glm::inverse(cmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * cmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(cmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

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

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	cmMat = glm::translate(glm::mat4(1.0f), cubeLoc2) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));
	
	invTrMat = glm::transpose(glm::inverse(cmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * cmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(cmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

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


	if (spawnInstancedCube)
	{
		thisAmb[0] = bMatAmb[0]; thisAmb[1] = bMatAmb[1]; thisAmb[2] = bMatAmb[2];  // bronze
		thisDif[0] = bMatDif[0]; thisDif[1] = bMatDif[1]; thisDif[2] = bMatDif[2];
		thisSpe[0] = bMatSpe[0]; thisSpe[1] = bMatSpe[1]; thisSpe[2] = bMatSpe[2];
		thisShi = bMatShi;

		// Set VIEW matrix
		updateCamera();
		vMat = cameraRMat * cameraTMat;

		currentLightPos = lightLoc;
		installLights(renderingProgram2);

		cmMat = glm::translate(glm::mat4(1.0f), cubeSpawnLocation) * glm::scale(glm::mat4(1.0f), vector3(0.5f, 0.5f, 0.5f));

		invTrMat = glm::transpose(glm::inverse(cmMat));
		shadowMVP2 = b * lightPMatrix * lightVMatrix * cmMat;

		glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(cmMat));
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
		//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
		glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

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
	}

	// ----------------------------------------- Plane -----------------------------------------

	thisAmb[0] = sMatAmb[0]; thisAmb[1] = sMatAmb[1]; thisAmb[2] = sMatAmb[2];  // silver
	thisDif[0] = sMatDif[0]; thisDif[1] = sMatDif[1]; thisDif[2] = sMatDif[2];
	thisSpe[0] = sMatSpe[0]; thisSpe[1] = sMatSpe[1]; thisSpe[2] = sMatSpe[2];
	thisShi = sMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	pmMat = glm::translate(glm::mat4(1.0f), vector3(planeLocX, planeLocY, planeLocZ));

	invTrMat = glm::transpose(glm::inverse(pmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * pmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(pmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	// Plane VERTICES
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	// Plane NORMALS
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, planeNumVertices);

	// ----------------------------------------- Vertical Lines (Blue) -----------------------------------------

	isLine = true;
	glUniform1i(isLineLoc2, isLine);

	thisAmb[0] = gMatAmb[0]; thisAmb[1] = gMatAmb[1]; thisAmb[2] = gMatAmb[2];  // gold
	thisDif[0] = gMatDif[0]; thisDif[1] = gMatDif[1]; thisDif[2] = gMatDif[2];
	thisSpe[0] = gMatSpe[0]; thisSpe[1] = gMatSpe[1]; thisSpe[2] = gMatSpe[2];
	thisShi = gMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	vlmMat = glm::translate(glm::mat4(1.0f), vector3(-10.0f, 0.0f, 0.0f));

	invTrMat = glm::transpose(glm::inverse(vlmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * vlmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(vlmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);

	isLine = false;
	glUniform1i(isLineLoc2, isLine);


	// ----------------------------------------- Track Lines (Green) -----------------------------------------

	thisAmb[0] = lgMatAmb[0]; thisAmb[1] = lgMatAmb[1]; thisAmb[2] = lgMatAmb[2];  // light green
	thisDif[0] = lgMatDif[0]; thisDif[1] = lgMatDif[1]; thisDif[2] = lgMatDif[2];
	thisSpe[0] = lgMatSpe[0]; thisSpe[1] = lgMatSpe[1]; thisSpe[2] = lgMatSpe[2];
	thisShi = lgMatShi;

	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	tlmMat = glm::translate(glm::mat4(1.0f), vector3(1.0f, 0.0f, 0.01f)) * glm::scale(glm::mat4(1.0f), vector3(1.0f, 5.0f, 1.0f));

	invTrMat = glm::transpose(glm::inverse(tlmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * tlmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(tlmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);


	/*glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);*/

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, tracklineNumVertices);

	// ----------------------------------------- Horizontal Lines (Green) -----------------------------------------
	
	isRow = true;
	glUniform1i(isRowLoc2, isRow);

	thisAmb[0] = gMatAmb[0]; thisAmb[1] = gMatAmb[1]; thisAmb[2] = gMatAmb[2];  // gold
	thisDif[0] = gMatDif[0]; thisDif[1] = gMatDif[1]; thisDif[2] = gMatDif[2];
	thisSpe[0] = gMatSpe[0]; thisSpe[1] = gMatSpe[1]; thisSpe[2] = gMatSpe[2];
	thisShi = gMatShi;
	
	// Set VIEW matrix
	updateCamera();
	vMat = cameraRMat * cameraTMat;

	currentLightPos = lightLoc;
	installLights(renderingProgram2);

	hlmMat = glm::translate(glm::mat4(1.0f), vector3(0.0f, -10.0f, 0.0f));

	invTrMat = glm::transpose(glm::inverse(hlmMat));
	shadowMVP2 = b * lightPMatrix * lightVMatrix * hlmMat;

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(hlmMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArraysInstanced(GL_LINES, 0, lineNumVertices, 21);

	isRow = false;
	glUniform1i(isRowLoc2, isRow);


	//// ----------------------------------------- Axes -----------------------------------------
	//// ----------------------------------------- X-axis (Red)
	//isAxes = true;
	//glUniform1i(isAxesLoc2, isAxes);

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
	//glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINES, 0, lineNumVertices);
	//isAxes = false;
	//glUniform1i(isAxesLoc2, isAxes);

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
	//glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

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
	//glUniformMatrix4fv(sLoc2, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	//glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINES, 0, lineNumVertices);
	//isLine = false;
	//glUniform1i(isLineLoc, isLine);
}

void setupCamera()
{
	cameraController.setPosition(0.0f, 0.0f, 20.0f);
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
	cameraController.rotateRight(cameraRMat);
}

void onQKeyPressed() 
{
	std::cout << "Q key pressed" << std::endl;

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


	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "light.position");
	directionLoc = glGetUniformLocation(renderingProgram, "light.direction");
	cutoffLoc = glGetUniformLocation(renderingProgram, "light.cutoffAngle");
	exponentLoc = glGetUniformLocation(renderingProgram, "light.exponent");

	mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");


	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
	glProgramUniform3fv(renderingProgram, directionLoc, 1, lightDirection);
	glProgramUniform1f(renderingProgram, cutoffLoc, spotlightCutoff);
	glProgramUniform1f(renderingProgram, exponentLoc, spotLightExponent);

	glProgramUniform4fv(renderingProgram, mambLoc, 1, matAmb);
	glProgramUniform4fv(renderingProgram, mdiffLoc, 1, matDif);
	glProgramUniform4fv(renderingProgram, mspecLoc, 1, matSpe);
	glProgramUniform1f(renderingProgram, mshiLoc, matShi);

	if (Utils::checkOpenGLError()) {
		std::cout << "OpenGL error in installLights!" << std::endl;
	}
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

void reloadShaders() {
	std::cout << "Reloading shaders..." << std::endl;

	// Delete old programs
	if (renderingProgram1 != 0) glDeleteProgram(renderingProgram1);
	if (renderingProgram2 != 0) glDeleteProgram(renderingProgram2);

	// Recreate programs
	renderingProgram1 = Utils::createShaderProgram("vertShader1.glsl", "fragShader1.glsl");
	renderingProgram2 = Utils::createShaderProgram("vertShader2.glsl", "fragShader2.glsl");

	std::cout << "Shaders reloaded. Program1: " << renderingProgram1 << ", Program2: " << renderingProgram2 << std::endl;

	// Check for compilation errors
	Utils::checkOpenGLError();
}

glm::vec3 getMouseWorldIntersection(GLFWwindow* window)
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
			// The multiplication order for creating the combined matrix is the reverse of the application order, which is usually applying the perspective matrix then the view matrix

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

	return intersection;
}

void moveCubeToCursor(GLFWwindow* window)
{
	glm::vec3 intersection = getMouseWorldIntersection(window);
	cubeSpawnLocation = intersection;
	cubeSpawnLocation.y += lightLoc.y; // Spawns the cube half way into the plane
	spawnCube();
}

void directLightToCursor(GLFWwindow* window)
{
	glm::vec3 intersection = getMouseWorldIntersection(window);
	// Point light toward intersection using your coordinate system
			// We have to create a vector from our light to our target location. 
			// We then normalize that vector because if it wasn't, it would cause different lighting behaviors based on different lengths
	glm::vec3 currentLight = lightLoc;
	intersection.y = lightLoc.y; // Fixed at lightLoc.y so that the angle of the light doesn't change and stays at the same height as the light position.
	glm::vec3 target = intersection;
	glm::vec3 direction = glm::normalize(target - currentLight);

	cout << "This is the direction: x: " << direction.x << "  y: " << direction.y << "  z: " << direction.z << endl;

	lightDirection[0] = direction.x;
	lightDirection[1] = direction.y;
	lightDirection[2] = direction.z;
	installLights(renderingProgram2);
}

void moveLightToCursor(GLFWwindow* window)
{
	glm::vec3 intersection = getMouseWorldIntersection(window);

	lightLoc = vector3(intersection.x, intersection.z, lightLoc.y);
	currentLightPos = lightLoc;

	installLights(renderingProgram2);
}

void spawnCube()
{
	spawnInstancedCube = true;
}
