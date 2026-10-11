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

float quadVertices[]
{
	// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

GLuint loadCubemap(std::vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// Disable byte-alignment restrictions to prevent black textures on odd-sized images
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		// Force stb_image to load 4 channels (RGBA) regardless of the original image format
		// The '4' at the end is the requested channel count (STBI_rgb_alpha)
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 4);

		if (data)
		{
			// Since we forced 4 channels, we strictly use GL_RGBA for both internal and format
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			// This will tell you exactly WHY it failed (e.g., "can't open file")
			std::cout << "Reason: " << stbi_failure_reason() << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

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

	//quadtexture for compute shader output
	VAO quadVAO;
	quadVAO.Bind();
	VBO quadVBO(quadVertices, sizeof(quadVertices));
	// Position attribute (location 0)
	quadVAO.LinkAttrib(quadVBO, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
	// Texture coordinate attribute (location 1)
	quadVAO.LinkAttrib(quadVBO, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	quadVAO.Unbind();

	const unsigned int TEXTURE_WIDTH = 700;
	const unsigned int TEXTURE_HEIGHT = 500;

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


	// Load the skybox (ensure these files exist in your working directory)
	std::vector<std::string> faces = {
		"C:\\Users\\obren\\OneDrive\\Desktop\\CERN\\openGL\\gravity_simulation\\right.jpg", "C:\\Users\\obren\\OneDrive\\Desktop\\CERN\\openGL\\gravity_simulation\\left.jpg", "C:\\Users\\obren\\OneDrive\\Desktop\\CERN\\openGL\\gravity_simulation\\top.jpg", "C:\\Users\\obren\\OneDrive\\Desktop\\CERN\\openGL\\gravity_simulation\\bottom.jpg", "C:\\Users\\obren\\OneDrive\\Desktop\\CERN\\openGL\\gravity_simulation\\front.jpg", "C:\\Users\\obren\\OneDrive\\Desktop\\CERN\\openGL\\gravity_simulation\\back.jpg"
	};
	GLuint skyboxTexture = loadCubemap(faces);

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

		// 1. Dispatch Compute Shader (Now a 2D grid based on screen resolution)
		computeShader.Activate();
		glUniform3fv(glGetUniformLocation(computeShader.ID, "bhPosition"), 1, glm::value_ptr(blackHole.position));
		glUniform1f(glGetUniformLocation(computeShader.ID, "bhMass"), (float)blackHole.mass);
		//TODO: leaving this causes ray tunneling, but removing it causes massive performance strain.
		glUniform1f(glGetUniformLocation(computeShader.ID, "dt"), 5.0f);

		// Assuming an 8x8 local workgroup size in the new compute shader
		GLuint workgroupsX = (TEXTURE_WIDTH + 7) / 8;
		GLuint workgroupsY = (TEXTURE_HEIGHT + 7) / 8;

		// 1. Dispatch Compute Shader
		computeShader.Activate();

		glUniform3fv(glGetUniformLocation(computeShader.ID, "camPos"), 1, glm::value_ptr(camera.Position));
		glUniform3fv(glGetUniformLocation(computeShader.ID, "camFront"), 1, glm::value_ptr(camera.Front));
		glUniform3fv(glGetUniformLocation(computeShader.ID, "camUp"), 1, glm::value_ptr(camera.Up));
		glUniform3fv(glGetUniformLocation(computeShader.ID, "camRight"), 1, glm::value_ptr(camera.Right));
		// Bind the skybox cubemap to binding = 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glUniform1i(glGetUniformLocation(computeShader.ID, "skybox"), 1);

		glUniform3fv(glGetUniformLocation(computeShader.ID, "bhPosition"), 1, glm::value_ptr(blackHole.position));

		glDispatchCompute(workgroupsX, workgroupsY, 1);

		// 2. Memory Barrier (Ensure compute shader is done writing to the texture)
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// 3. Render the Quad with the output texture
		shaderProgram.Activate();

		// Disable depth testing since we are just slapping a 2D texture over the screen
		glDisable(GL_DEPTH_TEST);

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

	VAO1.Delete();
	VBO1.Delete();
	quadVAO.Delete();
	glfwDestroyWindow(engine.window);
	glfwTerminate();
	return 0;
}