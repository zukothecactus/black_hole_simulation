#include "BlackHole.h"

BlackHole:: BlackHole(glm::vec3 pos, double m)
{
	position = pos;
	mass = m;
	//without .0 a buffer overload occurs
	radius = 2 * 6.67430e-11 * mass / (299792458.0 * 299792458.0); // Schwarzschild radius formula
}

std::vector<float> BlackHole::GenerateVertices()
{
	std::vector<float> vertices;
	int stacks = 50; // Number of stacks for the circle approximation
	int slices = 50; // Number of slices for the circle approximation

	for (int i = 0; i < stacks; i++)
	{
		float phi1 = PI * float(i) / float(stacks);
		float phi2 = PI * float(i + 1) / float(stacks);

		for (int j = 0; j < slices; ++j)
		{
			float theta1 = 2.0f * PI * float(j) / float(slices);
			float theta2 = 2.0f * PI * float(j + 1) / float(slices);

			auto pushVertex = [&](float p, float t)
				{
				vertices.push_back(position.x + radius * sin(p) * cos(t));
				vertices.push_back(position.y + radius * cos(p));
				vertices.push_back(position.z + radius * sin(p) * sin(t));
				//rgb values for black hole color (black)
				vertices.push_back(0.25f); 
				vertices.push_back(0.25f); 
				vertices.push_back(0.25f);
				};
			//triangle 1
			pushVertex(phi1, theta1);
			pushVertex(phi2, theta1);
			pushVertex(phi1, theta2);


			pushVertex(phi2, theta2);
			pushVertex(phi2, theta2);
			pushVertex(phi1, theta2);
		}
	}
	return vertices;
}