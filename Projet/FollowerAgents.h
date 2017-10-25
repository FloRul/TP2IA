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

class FollowerAgents :
	public Vehicle
{
private : 
	Vehicle* leader;
public:
	FollowerAgents(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale,
		Vehicle* leaderf);
	~FollowerAgents();
	Vehicle* GetLeader();
	void SetLeader(Vehicle* _leader);

	void        Render();

};

