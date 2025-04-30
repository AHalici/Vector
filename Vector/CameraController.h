#pragma once

#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <iostream>

class CameraController
{
public:
	CameraController();

	glm::vec3 getPosition();
	glm::vec3 getOrientation();

	glm::vec3 getU();
	glm::vec3 getV();
	glm::vec3 getN();

	float getRotationAngle();

	void setU(glm::vec3 u);
	void setV(glm::vec3 v);
	void setN(glm::vec3 n);

	void setPosition(float x, float y, float z);
	void setOrientation(glm::vec3 u, glm::vec3 v, glm::vec3 n);

	void moveForward();
	void moveBackward();
	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();

	void rotateRight(glm::mat4 &rMat);
	void rotateLeft(glm::mat4 &rMat);

private:
	//float X, Y, Z;
	float movementDistance = 0.1f;
	float rotationAngle = glm::radians(1.0f);
	glm::vec3 U, V, N, position, orientation;
};

