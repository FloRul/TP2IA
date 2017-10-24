#pragma once
#include "Vehicle.h"
#include "MovingEntity.h"
#include "2d/Vector2D.h"
#include "misc/Smoother.h"

#include <vector>
#include <list>
#include <string>

class GameWorld;
class SteeringBehavior;

class LeaderAgent : public Vehicle
{

public:

	LeaderAgent(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale);
	~LeaderAgent();

	void        Render();

};

