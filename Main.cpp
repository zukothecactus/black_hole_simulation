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
#include "LightRay.h"

int main()
{
	Engine engine;

	Shader shaderProgram("default.vert", "default.frag");

	BlackHole blackHole(glm::vec2(0.0f, 0.0f), 1.0e26);
	std::vector<float> blackHoleVertices = blackHole.GenerateVertices();

	VAO VAO1;
	VAO1.Bind();


	VBO VBO1(blackHoleVertices.data(), blackHoleVertices.size() * sizeof(float));

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO1.Unbind();

	std::vector<LightRay> rays;
	for (float y = 0.05f; y <= 0.95f; y += 0.1f)
	{
		rays.push_back(LightRay(y));
		rays.push_back(LightRay(-y));
	}
	VAO rayVAO;
	rayVAO.Bind();

	//initialize the VBO with nullptr and the size of the buffer
	VBO rayVBO(nullptr, rays[0].maxTrailLength * 6 * sizeof(float)); // 6 floats per vertex (x, y, z, r, g, b)

	rayVAO.LinkAttrib(rayVBO, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0); // Position attribute (layout 0)
	rayVAO.LinkAttrib(rayVBO, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Color attribute (layout 1)
	rayVAO.Unbind();


	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLint isPointLoc = glGetUniformLocation(shaderProgram.ID, "isPoint");

	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Update();
		shaderProgram.Activate();

		// 1. Draw Black Hole
		glUniform1i(isPointLoc, GL_FALSE);
		VAO1.Bind();
		glDrawArrays(GL_TRIANGLE_FAN, 0, blackHoleVertices.size() / 3);
		VAO1.Unbind();
		// 2. Draw Rays
		rayVAO.Bind();
		rayVBO.Bind();

		for (auto& ray : rays)
		{
			ray.travel(blackHole.position, blackHole.mass);
			std::vector<float> rayVertices = ray.GenerateVertices();

			if (!rayVertices.empty())
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, rayVertices.size() * sizeof(float), rayVertices.data());

				// Draw line strip
				glUniform1i(isPointLoc, GL_FALSE);
				glLineWidth(2.0f);
				glDrawArrays(GL_LINE_STRIP, 0, rayVertices.size() / 6);

				// Draw head point
				glUniform1i(isPointLoc, GL_TRUE);
				glPointSize(8.0f);
				glDrawArrays(GL_POINTS, 0, 1);
			}
		}

		rayVAO.Unbind();

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	VAO1.Delete();
	VBO1.Delete();
	rayVAO.Delete();
	rayVBO.Delete();
	glfwDestroyWindow(engine.window);
	glfwTerminate();
	return 0;
}