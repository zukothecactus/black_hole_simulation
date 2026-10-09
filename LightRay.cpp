#include "LightRay.h"

const float PI = 3.14159265358979323846f;
const float c = 299792458.0f; // Speed of light in meters per second
const double G = 6.67430e-11; // Gravitational constant in m^3 kg^-1 s^-2
const float c_visual = 0.01f; // Visual representation of the speed of light for scaling purposes

LightRay::LightRay(float startY)
{
	initialY = startY;
	position = glm::vec2(-1.0f, initialY); //Start on the left (visible)
	velocity = glm::vec2(c_visual, 0.0f); //Initial velocity to the right
	isSwallowed = false;
	isOffScreen = false;
}

std::vector<float> LightRay::GenerateVertices()
{
	std::vector<float> vertices;
	
	for(size_t i = 0; i < history.size(); ++i)
	{
		vertices.push_back(history[i].x);
		vertices.push_back(history[i].y);
		vertices.push_back(0.0f); // z-coordinate for 2D rendering

		float intensity = 1.0f - ((float)i / history.size()); // Fade effect
		// r, g, b (white/grey fading to black)
		if (i == 0)
		{
			vertices.push_back(3.0f); // Bright R
			vertices.push_back(3.0f); // Bright G
			vertices.push_back(2.0f); // Slightly warmer/whiter B
		}
		else {
			vertices.push_back(intensity);
			vertices.push_back(intensity);
			vertices.push_back(intensity);
		}
	}
	return vertices;
}

void LightRay::travel(glm::vec2 bhPosition, double bhMass)
{

	if (isSwallowed || isOffScreen)
	{
		if (!history.empty())
		{
			history.pop_back(); // Remove the oldest position from history
		}
	}
	

	//distance vector
	glm::vec2 r_vec = bhPosition - position;
	//magnitude of the distance vector
	float r = glm::length(r_vec);

	//event horizon check
	float rs = static_cast<float>((2.0 * G * bhMass) / (c * c)); // Schwarzschild radius
	if (r <= rs)
	{
		// Stop the ray if it crosses the event horizon
		velocity = glm::vec2(0.0f, 0.0f); 
		isSwallowed = true;
		return;
	}

	float GM_visual = (rs * c_visual * c_visual) / 2.0f; // Visual representation of GM for scaling purposes

	//gravitational acceleration
	//newtonian formula: a = G * M / r^2
	//General relativity photon deflection a = 2* (G * M / r^2)
	glm::vec2 r_hat = glm::normalize(r_vec);
	double acceleration_magnitude = 2.0 * GM_visual / (r * r);

	//conversion to float for OpenGL
	glm::vec2 acceleration = r_hat * static_cast<float>(acceleration_magnitude);

	//semi implicit Euler integration
	float dt = 3.0f; //Time step per frame (slow down/speed up the simulation)

	velocity += acceleration * dt;
	position += velocity * dt;


	//off screen check
	if (position.x > 2.0f || position.x < -2.0f || position.y > 2.0f || position.y < -2.0f) {
		isOffScreen = true;
	}

	history.push_front(position);
	if (history.size() > maxTrailLength)
	{
		history.pop_back();
	}
}