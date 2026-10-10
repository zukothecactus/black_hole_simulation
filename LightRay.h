#ifndef LIGHTRAY_H
#define LIGHTRAY_H

#include <glm/glm.hpp>
#include <vector>
#include <deque>

class LightRay
{
public:
	glm::vec3 position;
	glm::vec3 velocity;
	std::deque<glm::vec3> history;
	int maxTrailLength = 1000;
	float initialY;
	float initialZ;

	bool isSwallowed;
	bool isOffScreen;
	LightRay(float startY, float startZ);
	std::vector<float> GenerateVertices();

	//takes in the black hole parameters
	void travel(glm::vec3 bhPosition, double bhMass);
	
};


#endif