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
#include "Camera.h"

struct GPURay
{
	glm::vec4 position; //xyz = position, w = isSwallowed
	glm::vec4 velocity; //xyz = velocity, w = isOffScreen
};

int main()
{
	Engine engine;

	Shader shaderProgram("default.vert", "default.frag");
	Shader computeShader("raytrace.comp");


	BlackHole blackHole(glm::vec3(0.0f, 0.0f, 0.0f), 1.0e26);
	std::vector<float> blackHoleVertices = blackHole.GenerateVertices();

	VAO VAO1;
	VAO1.Bind();


	VBO VBO1(blackHoleVertices.data(), blackHoleVertices.size() * sizeof(float));

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO1.Unbind();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	GLint isPointLoc = glGetUniformLocation(shaderProgram.ID, "isPoint");

	//SSBO initialization
	const int RAYS_PER_SIDE = 20; // 20x20 grid = 400 total rays
	const int NUM_RAYS = RAYS_PER_SIDE * RAYS_PER_SIDE;
	const int TRAIL_LENGTH = 100;
	const int TOTAL_POINTS = NUM_RAYS * TRAIL_LENGTH;

	std::vector<GPURay> initialRays(TOTAL_POINTS);
	for (int i = 0; i < RAYS_PER_SIDE; ++i)
	{
		for (int j = 0; j < RAYS_PER_SIDE; ++j)
		{
			int headIdx = (i * RAYS_PER_SIDE + j) * TRAIL_LENGTH;

			// Distribute rays in a grid from -1.0 to 3.0 on both Y and Z axes
			float y = -0.5f + 1.0f * (float)i / (RAYS_PER_SIDE - 1);
			float z = -0.5f + 1.0f * (float)j / (RAYS_PER_SIDE - 1);

			// Head of the ray
			initialRays[headIdx].position = glm::vec4(-1.0f, y, z, 0.0f);
			initialRays[headIdx].velocity = glm::vec4(0.001f, 0.0f, 0.0f, 0.0f);

			// Initialize the trail history underneath the starting point
			for (int k = 1; k < TRAIL_LENGTH; k++) {
				initialRays[headIdx + k].position = glm::vec4(-1.5f, y, z, 1.0f);
				initialRays[headIdx + k].velocity = glm::vec4(0.0f);
			}
		}
	}
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, initialRays.size() * sizeof(GPURay), initialRays.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Add this right after configuring your SSBO, before the camera setup
	VAO rayVAO;
	rayVAO.Bind();
	glBindBuffer(GL_ARRAY_BUFFER, ssbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GPURay), (void*)0);
	rayVAO.Unbind();

	//camera activation
	Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), 800.0f, 600.0f);
	glfwSetWindowUserPointer(engine.window, &camera);
	glfwSetScrollCallback(engine.window, Camera::scrollCallback);
	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Update();
		shaderProgram.Activate();
		glUniform1i(isPointLoc, GL_TRUE);
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "pointSize"), 10.0f);

		camera.Inputs(engine.window);
		camera.updateMatrix(45.0f, 800.0f / 600.0f, 0.1f, 100.0f, shaderProgram);

		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// 1. Draw Black Hole
		glUniform1i(isPointLoc, GL_FALSE);
		VAO1.Bind();
		glDrawArrays(GL_TRIANGLES, 0, blackHoleVertices.size() / 6);
		VAO1.Unbind();
		// 2. Draw Rays
		// 1. Dispatch Compute Shader
		computeShader.Activate();
		glUniform3fv(glGetUniformLocation(computeShader.ID, "bhPosition"), 1, glm::value_ptr(blackHole.position));
		glUniform1f(glGetUniformLocation(computeShader.ID, "bhMass"), (float)blackHole.mass);
		glUniform1f(glGetUniformLocation(computeShader.ID, "dt"), 5.0f);
		glUniform1i(glGetUniformLocation(computeShader.ID, "trailLength"), TRAIL_LENGTH);
		GLuint workgroups = (NUM_RAYS + 127) / 128;
		glDispatchCompute(workgroups, 1, 1);

		// 2. Memory Barrier (Ensure compute writes finish before vertex reads)
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

		// 3. Render the updated SSBO as points
		shaderProgram.Activate();
		glUniform1i(isPointLoc, GL_TRUE);
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "pointSize"), 2.0f);

		rayVAO.Bind();
		glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f); // Make sure color is set here too if needed
		glDrawArrays(GL_POINTS, 0, TOTAL_POINTS);
		rayVAO.Unbind();

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	VAO1.Delete();
	VBO1.Delete();
	glfwDestroyWindow(engine.window);
	glfwTerminate();
	return 0;
}