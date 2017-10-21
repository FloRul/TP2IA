#pragma once
#include "MovingEntity.h"
#include "GameWorld.h"

class Bird : public MovingEntity
{
private:
	//a pointer to the world data. So a vehicle can access any obstacle,
	//path, wall or agent data
	GameWorld*            m_pWorld;

	//the steering behavior class
	SteeringBehavior*     m_pSteering;

	//buffer for the bird shape
	std::vector<Vector2D> m_vecBirdVB;

	//fills the buffer with vertex data
	void InitializeBuffer();

	//disallow the copying of Vehicle types
	Bird(const Bird&);
	Bird& operator=(const Bird&);
};
