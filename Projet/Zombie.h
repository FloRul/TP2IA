#pragma once
#include "MovingEntity.h"
#include "Vehicle.h"
#include "GameWorld.h"

class Zombie : public Vehicle
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
	Zombie(const Zombie&);
	Zombie& operator=(const Zombie&);

public :

	Zombie(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale);

	~Zombie();

	//updates the zombie's position and orientation
	void        Update(double time_elapsed);

	void        Render();

	//-------------------------------------------accessor methods
	SteeringBehavior*const  Steering()const { return m_pSteering; }
	GameWorld*const         World()const { return m_pWorld; }

};
