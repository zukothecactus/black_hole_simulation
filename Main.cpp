#include <iostream>

#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine.h"
#include "shaderClass.h"
#include "BlackHole.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

int main()
{
	Engine engine;

	Shader shaderProgram("default.vert", "default.frag");

	BlackHole blackHole(glm::vec2(0.0f, 0.0f), 1.0e26);
	std::vector<float> blackHoleVertices = blackHole.GenerateVertices();

	VAO VAO1;
	VAO1.Bind();


	VBO VBO1(blackHoleVertices.data(), blackHoleVertices.size() * sizeof(float));

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	VAO1.Unbind();



	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Update();
		shaderProgram.Activate();

		//Calculate a projection matrix large enough to view the black hole
		//glm::mat4 projection = glm::ortho(-2000.0f, 2000.0f, -2000.0f, 2000.0f, -1.0f, 1.0f);
		//int camLoc = glGetUniformLocation(shaderProgram.ID, "camMatrix");
		//glUniformMatrix4fv(camLoc, 1, GL_FALSE, glm::value_ptr(projection));


		VAO1.Bind();
		glDrawArrays(GL_TRIANGLE_FAN, 0, blackHoleVertices.size() / 3);
		VAO1.Unbind();
		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	VAO1.Delete();
	VBO1.Delete();
	glfwDestroyWindow(engine.window);
	glfwTerminate();
	return 0;
}