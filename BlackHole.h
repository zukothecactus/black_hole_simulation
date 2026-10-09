#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include <glm/glm.hpp>
#include <vector>

const float PI = 3.14159265358979323846f;

class BlackHole
{
public:
	glm::vec3 position;
	double mass;
	double radius; // Schwarzschild radius == event horizon radius, for non rotating black holes

	BlackHole(glm::vec3 pos, double m);
	std::vector<float> GenerateVertices();
};

#endif