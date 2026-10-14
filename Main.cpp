#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine.h"
#include "shaderClass.h"
#include "BlackHole.h"
#include "VAO.h"
#include "VBO.h"
#include "Camera.h"

float quadVertices[]
{
	// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};


int main()
{
	Engine engine;

	Shader shaderProgram("default.vert", "default.frag");
	Shader computeShader("raytrace.comp");

	BlackHole blackHole(glm::vec3(0.0f, 0.0f, 0.0f), 1.0e26);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//quadtexture for compute shader output
	VAO quadVAO;
	quadVAO.Bind();
	VBO quadVBO(quadVertices, sizeof(quadVertices));
	// Position attribute (location 0)
	quadVAO.LinkAttrib(quadVBO, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
	// Texture coordinate attribute (location 1)
	quadVAO.LinkAttrib(quadVBO, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	quadVAO.Unbind();

	const unsigned int TEXTURE_WIDTH = 600;
	const unsigned int TEXTURE_HEIGHT = 400;

	GLuint renderTexture;
	glGenTextures(1, &renderTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// GL_RGBA32F is crucial for compute shader imageStore operations
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	// Bind it as an image unit (binding = 0) so the compute shader can write to it
	glBindImageTexture(0, renderTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	//camera activation
	Camera camera(glm::vec3(0.0f, 0.0f, 15.0f), 800.0f, 600.0f);
	glfwSetWindowUserPointer(engine.window, &camera);
	glfwSetScrollCallback(engine.window, Camera::scrollCallback);


	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Update();

		camera.Inputs(engine.window);

		// 1. Dispatch Compute Shader (Now a 2D grid based on screen resolution)
		computeShader.Activate();
		glUniform3fv(glGetUniformLocation(computeShader.ID, "bhPosition"), 1, glm::value_ptr(blackHole.position));
		glUniform1f(glGetUniformLocation(computeShader.ID, "bhMass"), (float)blackHole.mass);
		//TODO: leaving this causes ray tunneling, but removing it causes massive performance strain.
		//glUniform1f(glGetUniformLocation(computeShader.ID, "dt"), 5.0f);

		// Assuming an 8x8 local workgroup size in the new compute shader
		GLuint workgroupsX = (TEXTURE_WIDTH + 7) / 8;
		GLuint workgroupsY = (TEXTURE_HEIGHT + 7) / 8;

		// 1. Dispatch Compute Shader
		computeShader.Activate();

		glUniform3fv(glGetUniformLocation(computeShader.ID, "camPos"), 1, glm::value_ptr(camera.Position));
		glUniform3fv(glGetUniformLocation(computeShader.ID, "camFront"), 1, glm::value_ptr(camera.Front));
		glUniform3fv(glGetUniformLocation(computeShader.ID, "camUp"), 1, glm::value_ptr(camera.Up));
		glUniform3fv(glGetUniformLocation(computeShader.ID, "camRight"), 1, glm::value_ptr(camera.Right));

		glUniform3fv(glGetUniformLocation(computeShader.ID, "bhPosition"), 1, glm::value_ptr(blackHole.position));

		glDispatchCompute(workgroupsX, workgroupsY, 1);

		// 2. Memory Barrier (Ensure compute shader is done writing to the texture)
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// 3. Render the Quad with the output texture
		shaderProgram.Activate();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderTexture);
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "screenTexture"), 0);

		quadVAO.Bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quadVAO.Unbind();

		// Re-enable depth test for the next frame if you plan to draw rasterized 3D geometry later
		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	quadVAO.Delete();
	glfwDestroyWindow(engine.window);
	glfwTerminate();
	return 0;
}