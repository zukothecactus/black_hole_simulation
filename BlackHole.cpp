#include "BlackHole.h"

BlackHole:: BlackHole(glm::vec2 pos, double m)
{
	position = pos;
	mass = m;
	//without .0 a buffer overload occurs
	radius = 2 * 6.67430e-11 * mass / (299792458.0 * 299792458.0); // Schwarzschild radius formula
}

std::vector<float> BlackHole::GenerateVertices()
{
	std::vector<float> vertices;
	vertices.push_back(position.x);
	vertices.push_back(position.y);
	vertices.push_back(0.0f); // z-coordinate for 2D rendering

	//rgb values for black hole color (black)
	vertices.push_back(0.0f); 
	vertices.push_back(0.0f); 
	vertices.push_back(0.0f);

	for (int i = 0; i <= 100;i++)
	{
		float angle = 2.0f * PI * float(i) / float(100);
		float x = position.x + radius * cos(angle);
		float y = position.y + radius * sin(angle);
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(0.0f); // z-coordinate for 2D rendering
		//rgb values for black hole color (black)
		vertices.push_back(0.0f); 
		vertices.push_back(0.0f); 
		vertices.push_back(0.0f);
	}
	return vertices;
}