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
}

FollowerAgents::FollowerAgents(GameWorld* world,
	Vector2D position,
	double    rotation,
	Vector2D velocity,
	double    mass,
	double    max_force,
	double    max_speed,
	double    max_turn_rate,
	double    scale) :
	Vehicle(world,
		position,
		rotation,
		velocity,
		mass,
		max_force,
		max_speed,
		max_turn_rate,
		scale){}

FollowerAgents::~FollowerAgents()
{}

//------------- Setter and Getter for the leader of the agent -----------------
//-----------------------------------------------------------------------------
Vehicle* FollowerAgents::GetLeader() 
{
	return this->leader;
}

void FollowerAgents::SetLeader(Vehicle* _leader) 
{
	this->leader = _leader;
	this->Steering()->OffsetPursuitOn(this->leader, Vector2D(-5, 0));
	this->Steering()->SeparationOn();
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void FollowerAgents::Render()
{
	//a vector to hold the transformed vertices
	static std::vector<Vector2D>  m_vecVehicleVBTrans;

	//render neighboring vehicles in different colors if requested
	if (m_pWorld->RenderNeighbors())
	{
		if (ID() == 0) gdi->RedPen();
		else if (IsTagged()) gdi->GreenPen();
		else gdi->BluePen();
	}

	else
	{
		gdi->BluePen();
	}

	if (Steering()->isInterposeOn())
	{
		gdi->RedPen();
	}

	if (Steering()->isHideOn())
	{
		gdi->GreenPen();
	}

	if (isSmoothingOn())
	{
		m_vecVehicleVBTrans = WorldTransform(m_vecVehicleVB,
			Pos(),
			SmoothedHeading(),
			SmoothedHeading().Perp(),
			Scale());
	}

	else
	{
		m_vecVehicleVBTrans = WorldTransform(m_vecVehicleVB,
			Pos(),
			Heading(),
			Side(),
			Scale());
	}


	gdi->ClosedShape(m_vecVehicleVBTrans);

	//render any visual aids / and or user options
	if (m_pWorld->ViewKeys())
	{
		Steering()->RenderAids();
	}
}
