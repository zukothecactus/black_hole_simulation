#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <string>
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include<cerrno>

class Shader
{
public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);

	void Activate();
	void Delete();

private:
	void CompileErrors(unsigned int shader, const char* type);
};

std::string get_file_contents(const char* filename);

#endif
