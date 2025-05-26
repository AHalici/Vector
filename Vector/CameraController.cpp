#include "CameraController.h"

CameraController::CameraController() 
{
	position = glm::vec3(0.0f, 0.0f, 0.0f);

	U = glm::vec3(1.0f, 0.0f, 0.0f); // Example: set to default basis vectors
	V = glm::vec3(0.0f, 0.0f, 1.0f);
	N = glm::vec3(0.0f, -1.0f, 0.0f);
}

glm::vec3 CameraController::getPosition()
{
	return position;
}

glm::vec3 CameraController::getU()
{
	return U;
}

glm::vec3 CameraController::getV()
{
	return V;
}

glm::vec3 CameraController::getN()
{
	return N;
}

float CameraController::getRotationAngle()
{
	return rotationAngle;
}

void CameraController::setU(glm::vec3 u) 
{
	U = u;
}

void CameraController::setV(glm::vec3 v)
{
	V = v;
}

void CameraController::setN(glm::vec3 n)
{
	N = n;
}

void CameraController::setPosition(float x, float y, float z)
{
	position = glm::vec3(x, z, y);
}

void CameraController::setOrientation(glm::vec3 u, glm::vec3 v, glm::vec3 n)
{
	this->U = u;
	this->V = v;
	this->N = n;
}

void CameraController::moveForward()
{
	position += (movementDistance * N);
}

void CameraController::moveBackward()
{
	position -= (movementDistance * N);
}

void CameraController::moveRight()
{
	position += (movementDistance * U);
}

void CameraController::moveLeft()
{
	position -= (movementDistance * U);
}

void CameraController::moveUp()
{
	position += (movementDistance * V);
}

void CameraController::moveDown()
{
	position -= (movementDistance * V);
}

void CameraController::rotateRight(glm::mat4 &rMat)
{
	rMat = glm::rotate(glm::mat4(1.0f), rotationAngle, V);
	U = glm::vec3(rMat * glm::vec4(U, 0.0f));
	N = glm::vec3(rMat * glm::vec4(N, 0.0f));
}

void CameraController::rotateLeft(glm::mat4 &rMat)
{
	rMat = glm::rotate(glm::mat4(1.0f), -rotationAngle, V);
	U = rMat * glm::vec4(U, 0.0f);
	N = rMat * glm::vec4(N, 0.0f);
}



