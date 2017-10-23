#include "Vehicle.h"
#include "FollowerAgents.h"
#include "2d/C2DMatrix.h"
#include "2d/Geometry.h"
#include "SteeringBehaviors.h"
#include "2d/Transformations.h"
#include "GameWorld.h"
#include "misc/CellSpacePartition.h"
#include "misc/cgdi.h"

FollowerAgents::FollowerAgents(GameWorld* world,
	Vector2D position,
	double    rotation,
	Vector2D velocity,
	double    mass,
	double    max_force,
	double    max_speed,
	double    max_turn_rate,
	double    scale,
	Vehicle* leaderf):
	Vehicle(world,
		position,
		rotation,
		velocity,
		mass,
		max_force,
		max_speed,
		max_turn_rate,
		scale)
{
	this->leader = leaderf;
	//Designe le comportement
	//this->Steering()->ArriveOn();
	this->Steering()->OffsetPursuitOn(this->leader, Vector2D(2, 0));
	this->Steering()->SeparationOn();
	this->Steering()->Calculate();

}


FollowerAgents::~FollowerAgents()
{
}
