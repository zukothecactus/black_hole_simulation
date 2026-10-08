#include "VBO.h"

VBO::VBO(GLfloat* vertices, GLsizeiptr size)
{
	//binding objects means that that object becomes the current active object, and all calls after that will be applied to it, until another object of the same type is bound
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	//for performance
	//STATIC - verticies will be modified once and used many many times
	//DYNAMIC - verticies will be modified repeatedly and used many many times
	//STREAM - verticies will be modified once and used at most a few times
}

void VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete()
{
	glDeleteBuffers(1, &ID);
}