#include "LeaderAgent.h"
#include "SteeringBehaviors.h"
#include "GameWorld.h"
#include "misc/cgdi.h"

LeaderAgent::LeaderAgent(GameWorld* world,
						Vector2D position,
						double    rotation,
						Vector2D velocity,
						double    mass,
						double    max_force,
						double    max_speed,
						double    max_turn_rate,
						double    scale) : Vehicle(world,
													position,
													rotation,
													velocity,
													mass,
													max_force,
													max_speed,
													max_turn_rate,
													scale)
{ }

LeaderAgent::~LeaderAgent()
{ }

void LeaderAgent::StartBehavior()
{
	//Ajouter le steering behavior
	this->Steering()->WanderOn();
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void LeaderAgent::Render()
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
		gdi->RedPen();
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
