#ifndef ENGINE_H
#define ENGINE_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Engine
{
public:
	GLFWwindow* window;
	int WIDTH = 800;
	int HEIGHT = 800;
	float width = 1e11;
	float height = 1e11;

	Engine();
	void Update();
};

#endif