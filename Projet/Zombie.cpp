#include "Zombie.h"
#include "SteeringBehaviors.h"
#include "GameWorld.h"
#include "misc/cgdi.h"

Zombie::Zombie(GameWorld* world,
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
{
	//Add the behavior
	this->m_pSteering->WanderOn();
}

Zombie::~Zombie()
{ }


void Zombie::Update(double time_elapsed)
{
	// TODO
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Zombie::Render()
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

	gdi->ClosedShape(m_vecVehicleVBTrans);

	//render any visual aids / and or user options
	if (m_pWorld->ViewKeys())
	{
		Steering()->RenderAids();
	}
}
