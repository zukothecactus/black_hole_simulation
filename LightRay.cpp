#include "LightRay.h"

const float PI = 3.14159265358979323846f;
const float c = 299792458.0f; // Speed of light in meters per second
const double G = 6.67430e-11; // Gravitational constant in m^3 kg^-1 s^-2
const float c_visual = 0.001f; // Visual representation of the speed of light for scaling purposes

LightRay::LightRay(float startY, float startZ)
{
	initialY = startY;
	initialZ = startZ;
	position = glm::vec3(-1.0f, initialY, initialZ); //Start on the left (visible)
	velocity = glm::vec3(c_visual, 0.0f, 0.0f); //Initial velocity to the right
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
		vertices.push_back(history[i].z); // z-coordinate for 2D rendering

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

void LightRay::travel(glm::vec3 bhPosition, double bhMass)
{

	if (isSwallowed || isOffScreen)
	{
		if (!history.empty())
		{
			history.pop_back(); // Remove the oldest position from history
		}
		return;
	}
	

	//distance vector
	glm::vec3 r_vec = bhPosition - position;
	//magnitude of the distance vector
	float r = glm::length(r_vec);

	//event horizon check
	float rs = static_cast<float>((2.0 * G * bhMass) / (c * c)); // Schwarzschild radius
	if (r <= rs)
	{
		// Stop the ray if it crosses the event horizon
		velocity = glm::vec3(0.0f, 0.0f, 0.0f); 
		isSwallowed = true;
		return;
	}

	/*
	float GM_visual = (rs * c_visual * c_visual) / 2.0f; // Visual representation of GM for scaling purposes

	//gravitational acceleration
	//newtonian formula: a = G * M / r^2
	//General relativity photon deflection a = 2* (G * M / r^2)
	glm::vec2 r_hat = glm::normalize(r_vec);
	double acceleration_magnitude = 2.0 * GM_visual / (r * r);

	//conversion to float for OpenGL
	glm::vec2 acceleration = r_hat * static_cast<float>(acceleration_magnitude);
	*/
	//implicit Euler integration
	float dt = 3.0f; //Time step per frame (slow down/speed up the simulation)

	//lambda function to calculate acceleration based on current position and velocity
	auto getAcceleration = [&](glm::vec3 pos, glm::vec3 vel) -> glm::vec3
		{
			glm::vec3 r_vec_current = pos - bhPosition;
			float r_current = glm::length(r_vec_current); //magnitude of the distance vector

			//specific angular momentum squared: h^2 = |r x v|^2
			glm::vec3 cross_product = glm::cross(r_vec_current, vel);
			//dot() function -> h^2 = scalar multiplication of the cross product with itself
			float h2 = glm::dot(cross_product, cross_product);

			float coefficient = -1.5f * rs * h2 / std::pow(r_current, 5.0f);
			return coefficient * r_vec_current;
		};

	//Range-Kutta 4th order method for better accuracy
	glm::vec3 k1_v = getAcceleration(position, velocity);
	glm::vec3 k1_p = velocity;

	glm::vec3 k2_v = getAcceleration(position+k1_p*(dt*0.5f), velocity + k1_v*(dt*0.5f));
	glm::vec3 k2_p = velocity + k1_v * (dt * 0.5f);

	glm::vec3 k3_v = getAcceleration(position + k2_p * (dt * 0.5f), velocity + k2_v * (dt * 0.5f));
	glm::vec3 k3_p = velocity + k2_v * (dt * 0.5f);

	glm::vec3 k4_v = getAcceleration(position + k3_p * dt, velocity + k3_v * dt);
	glm::vec3 k4_p = velocity + k3_v * dt;

	velocity += (dt / 6.0f) * (k1_v + 2.0f * k2_v + 2.0f * k3_v + k4_v);
	position += (dt / 6.0f) * (k1_p + 2.0f * k2_p + 2.0f * k3_p + k4_p);

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