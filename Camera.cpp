#include "Camera.h"

Camera::Camera(glm::vec3 position, float windowHeight, float windowWidth)
{
	lastX = windowWidth / 2.0f;
	lastY = windowHeight / 2.0f;
    // Calculate initial distance based on the provided position
    Distance = glm::length(position - glm::vec3(0.0f, 0.0f, 0.0f));
	updateCameraVectors();
}

void Camera::updateCameraVectors()
{
	//calculating the viewing direction based on yaw and pitch angles
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);

    //orbital logic
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f); //black hole
    Position = target - Front * Distance;

	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::Inputs(GLFWwindow* window)
{
    // W and S control orbital zoom
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Distance -= Speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Distance += Speed;

	//limit the distance to avoid going inside the black hole
	if (Distance < 0.2f) Distance = 0.2f;

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

void Camera::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->Distance -= static_cast<float>(yoffset) * camera->Speed;
        if (camera->Distance < 0.2f) camera->Distance = 0.2f; // Limit the distance
        camera->updateCameraVectors();
    }
}