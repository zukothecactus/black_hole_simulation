#include "Camera.h"

Camera::Camera(glm::vec3 position, float windowHeight, float windowWidth)
{
	Position = position;
	lastX = windowWidth / 2.0f;
	lastY = windowHeight / 2.0f;
	updateCameraVectors();
}

void Camera::updateCameraVectors()
{
	//calculating the new front vector using euler angles
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::Inputs(GLFWwindow* window)
{
    // Keyboard movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += Speed * Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= Speed * Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::normalize(glm::cross(Front, Up)) * Speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::normalize(glm::cross(Front, Up)) * Speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Position += Speed * WorldUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        Position -= Speed * WorldUp;

    // Mouse look (held down left click)
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        xoffset *= Sensitivity;
        yoffset *= Sensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Constrain pitch to avoid screen flipping
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

        updateCameraVectors();
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

void Camera::updateMatrix(float FOVdeg, float aspectRatio, float nearPlane, float farPlane, Shader& shader)
{
    glm::mat4 view = glm::lookAt(Position, Position + Front, Up);
    glm::mat4 proj = glm::perspective(glm::radians(FOVdeg), aspectRatio, nearPlane, farPlane);

    // Sends the matrices to the vertex shader
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
}