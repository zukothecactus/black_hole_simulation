#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderClass.h"

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float Yaw = -90.0f;
	float Pitch = 0.0f;
	float Speed = 0.05f;
	float Sensitivity = 0.1f;

	bool firstMouse = true;
	float lastX, lastY;

	Camera(glm::vec3 position, float windowWidth, float windowHeight);

	void updateMatrix(float FOVdeg, float aspectRation, float nearPlane, float farPlane, Shader& shader);
	void Inputs(GLFWwindow* window);

private:
	void updateCameraVectors();
};

#endif // CAMERA_H