#include "VAO.h"

VAO::VAO()
{
	glGenVertexArrays(1, &ID);
}

void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
	VBO.Bind();
	//vertex attribute is a way to comminicate with the vertex shader from the outside
	//(position of the vertex attribute, number of values per vertex, type of values we have, amount of data between each vertex, offset (pointer to where our vertices begin in the array))
	//layout - the location of the vertex attribute in the shader (0 for coordinates, 1 for colors...)
	//numComponents - how many components per vertex (3 for coordinates, 3 for colors...)
	//type - the type of data (GL_FLOAT for float, GL_INT for int...)
	//stride - the amount of data between each vertex (we have both the coordinates and the color strides are 6 * sizeof(float) because we have 6 floats per vertex)
	//offset - the initial offset of layout in bytes (0 for coordinates, 3 * sizeof(float) for colors...)
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

void VAO::Bind()
{
	glBindVertexArray(ID);
}

void VAO::Unbind()
{
	glBindVertexArray(0);
}

void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}