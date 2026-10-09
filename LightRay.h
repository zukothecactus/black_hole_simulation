#ifndef LIGHTRAY_H
#define LIGHTRAY_H

#include <glm/glm.hpp>
#include <vector>
#include <deque>

class LightRay
{
public:
	glm::vec2 position;
	glm::vec2 velocity;
	std::deque<glm::vec2> history;
	int maxTrailLength = 50;
	float initialY;

	bool isSwallowed;
	bool isOffScreen;
	LightRay(float startY);
	std::vector<float> GenerateVertices();

	//takes in the black hole parameters
	void travel(glm::vec2 bhPosition, double bhMass);
};


#endif