#include "GameWorld.h"
#include "Vehicle.h"
#include "LeaderAgent.h"
#include "FollowerAgents.h"
#include "Zombie.h"
#include "constants.h"
#include "Obstacle.h"
#include "2d/Geometry.h"
#include "2d/Wall2D.h"
#include "2d/Transformations.h"
#include "SteeringBehaviors.h"
#include "time/PrecisionTimer.h"
#include "misc/Smoother.h"
#include "ParamLoader.h"
#include "misc/WindowUtils.h"
#include "misc/Stream_Utility_Functions.h"
#include "FollowerAgents.h"
#include "LeaderAgent.h"


#include "resource.h"

#include <list>

using std::list;


//------------------------------- ctor -----------------------------------
//------------------------------------------------------------------------
GameWorld::GameWorld(int cx, int cy, int nb_leader, int agent_humain,
	int comportement, int nb_agents, int nb_poursuiveurs_l1, int nb_poursuiveurs_l2,
	int nb_poursuiveurs_h, int offset) :

	m_cxClient(cx),
	m_cyClient(cy),
	m_bPaused(false),
	m_vCrosshair(Vector2D(cxClient() / 2.0, cxClient() / 2.0)),
	m_bShowWalls(false),
	m_bShowObstacles(false),
	m_bShowPath(false),
	m_bShowWanderCircle(false),
	m_bShowSteeringForce(false),
	m_bShowFeelers(false),
	m_bShowDetectionBox(false),
	m_bShowFPS(true),
	m_dAvFrameTime(0),
	m_pPath(NULL),
	m_bRenderNeighbors(false),
	m_bViewKeys(false),
	m_bShowCellSpaceInfo(false)
{
	/*nb_leader = 2;*/
	// Create local variable
	int nb_agents_total = nb_agents + nb_poursuiveurs_l1 + nb_poursuiveurs_l2 + nb_poursuiveurs_h + agent_humain + nb_leader;

	//setup the spatial subdivision class
	m_pCellSpace = new CellSpacePartition<Vehicle*>((double)cx, (double)cy, Prm.NumCellsX, Prm.NumCellsY, nb_agents_total);

	double border = 30;
	m_pPath = new Path(5, border, border, cx - border, cy - border, true);

	std::vector<FollowerAgents*> listeFollowerL1;
	std::vector<FollowerAgents*> listeFollowerL2;



	// setup basic agents
	Vehicle* pVehicle;
	switch (comportement)
	{
		//--------------- Comportement basique ------------------------//
		//-------------------------------------------------------------//
	case 0:
	{
		for (int a = 0; a < nb_agents; ++a)
		{
			Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
				cy / 2.0 + RandomClamped()*cy / 2.0);
			// Create the agent
			pVehicle = new Vehicle(this,
				SpawnPos,                 //initial position
				RandFloat()*TwoPi,        //start rotation
				Vector2D(0, 0),            //velocity
				Prm.VehicleMass,          //mass
				Prm.MaxSteeringForce,     //max force
				Prm.MaxSpeed,             //max velocity
				Prm.MaxTurnRatePerSecond, //max turn rate
				Prm.VehicleScale);        //scale

			if (!m_aHuman & agent_humain == 1) {
				m_aHuman = true;
				m_HumanAgent = pVehicle;
			}
			else {
				// Standard behavior
				pVehicle->Steering()->WanderOn();
			}


			//add dans la liste de vehicule
			m_Vehicles.push_back(pVehicle);
			//add it to the cell subdivision
			m_pCellSpace->AddEntity(pVehicle);
		}
		break;
		//--------------------------------------------------------------//
	}

	//---------------LeaderFollowing behavior ----------------------//
	//--------------------------------------------------------------//
	case 1:
	{

		LeaderAgent* pLeader1;
		switch (nb_leader)
		{
			//---------------- 1 seul leader ---------------------------
			//----------------------------------------------------------
		case 1:
			for (int i = 0; i < nb_leader; i++)
			{
				Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
					cy / 2.0 + RandomClamped()*cy / 2.0);

				pLeader1 = new LeaderAgent(this,
					SpawnPos,                 //initial position
					RandFloat()*TwoPi,        //start rotation
					Vector2D(0, 0),            //velocity
					Prm.VehicleMass,          //mass
					Prm.MaxSteeringForce,     //max force
					Prm.MaxSpeed,             //max velocity
					Prm.MaxTurnRatePerSecond, //max turn rate
					Prm.VehicleScale);        //scale


				if (!m_aHuman & agent_humain==1) {
					m_aHuman = true;
					m_HumanAgent = pLeader1;
				}
				else {
					// Standard behavior
					pLeader1->Steering()->WanderOn();
				}

				
				
				//add dans la liste de vehicule


				m_Leaders.push_back(pLeader1);
				m_Vehicles.push_back(pLeader1);

				//add it to the cell subdivision
				m_pCellSpace->AddEntity(pLeader1);
			}

			//Ajout des followers
			for (int i = 0; i < nb_poursuiveurs_l1; i++)
			{

				Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
					cy / 2.0 + RandomClamped()*cy / 2.0);

				FollowerAgents* pFollow = new FollowerAgents(this,
					SpawnPos,                 //initial position
					RandFloat()*TwoPi,        //start rotation
					Vector2D(0, 0),            //velocity
					Prm.VehicleMass,          //mass
					Prm.MaxSteeringForce,     //max force
					Prm.MaxSpeed,             //max velocity
					Prm.MaxTurnRatePerSecond, //max turn rate
					Prm.VehicleScale);        //scale





											  //add dans la liste des followers
				m_Vehicles.push_back(pFollow);
				//add it to the cell subdivision
				m_pCellSpace->AddEntity(pFollow);
				listeFollowerL1.push_back(pFollow);
			}
			//Relier les agents entre eux puis le dernier au leader
			for (int i = 0; i < (int)listeFollowerL1.size() - 1; i++)
			{
				FollowerAgents* tempFAgent = listeFollowerL1.at(i);
				tempFAgent->SetLeader(listeFollowerL1.at(i + 1));
				tempFAgent->UpdateSteering();
			}

			listeFollowerL1.at(listeFollowerL1.size() - 1)->SetLeader(m_Leaders.at(0));
			listeFollowerL1.at(listeFollowerL1.size() - 1)->UpdateSteering();
			break;

			//----------------------2 Leaders -----------------------------------------
			//-------------------------------------------------------------------------
		case 2:
			//ajout de 2 leader
			for (int i = 0; i < nb_leader; i++)
			{
				Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
					cy / 2.0 + RandomClamped()*cy / 2.0);

				LeaderAgent* pLeader2 = new LeaderAgent(this,
					SpawnPos,                 //initial position
					RandFloat()*TwoPi,        //start rotation
					Vector2D(0, 0),            //velocity
					Prm.VehicleMass,          //mass
					Prm.MaxSteeringForce,     //max force
					Prm.MaxSpeed,             //max velocity
					Prm.MaxTurnRatePerSecond, //max turn rate
					Prm.VehicleScale);        //scale

				if (!m_aHuman & agent_humain == 1) {
					m_aHuman = true;
					m_HumanAgent = pLeader2;
				}

				m_Leaders.push_back(pLeader2);

				m_Vehicles.push_back(pLeader2);
				//add it to the cell subdivision
				m_pCellSpace->AddEntity(pLeader2);
			}

			//Ajout des followers L1
			for (int i = 0; i < nb_poursuiveurs_l1; i++)
			{
				Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
					cy / 2.0 + RandomClamped()*cy / 2.0);
				FollowerAgents* pFollow = new FollowerAgents(this,
					SpawnPos,                 //initial position
					RandFloat()*TwoPi,        //start rotation
					Vector2D(0, 0),            //velocity
					Prm.VehicleMass,          //mass
					Prm.MaxSteeringForce,     //max force
					Prm.MaxSpeed,             //max velocity
					Prm.MaxTurnRatePerSecond, //max turn rate
					Prm.VehicleScale);        //scale


				
				


				listeFollowerL1.push_back(pFollow);
				m_Vehicles.push_back(pFollow);
				//add it to the cell subdivision
				m_pCellSpace->AddEntity(pFollow);
			}
			//Relier les agents entre eux puis le dernier au leader
			for (int i = 0; i < (int)listeFollowerL1.size() - 1; i++)
			{
				FollowerAgents* tempFAgent = listeFollowerL1.at(i);
				tempFAgent->SetLeader(listeFollowerL1.at(i + 1));
				tempFAgent->UpdateSteering();

				m_Vehicles.push_back(tempFAgent);
				//add it to the cell subdivision
				m_pCellSpace->AddEntity(tempFAgent);
			}
			listeFollowerL1.at(listeFollowerL1.size() - 1)->SetLeader(m_Leaders.at(0));
			listeFollowerL1.at(listeFollowerL1.size() - 1)->UpdateSteering();

			//-----------------------------------------------------------------------

			//Ajout des followers L2
			//-----------------------------------------------------------------------
			for (int i = 0; i < nb_poursuiveurs_l2; i++)
			{
				Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
					cy / 2.0 + RandomClamped()*cy / 2.0);

				FollowerAgents* pFollow = new FollowerAgents(this,
					SpawnPos,                 //initial position
					RandFloat()*TwoPi,        //start rotation
					Vector2D(0, 0),            //velocity
					Prm.VehicleMass,          //mass
					Prm.MaxSteeringForce,     //max force
					Prm.MaxSpeed,             //max velocity
					Prm.MaxTurnRatePerSecond, //max turn rate
					Prm.VehicleScale);        //scale


				m_Vehicles.push_back(pFollow);
				//add it to the cell subdivision
				m_pCellSpace->AddEntity(pFollow);
				//add dans la liste des followers
				listeFollowerL2.push_back(pFollow);
			}
			//Relier les agents entre eux puis le dernier au leader
			for (int i = 0; i < (int)listeFollowerL2.size() - 1; i++)
			{
				FollowerAgents* tempFAgent = listeFollowerL2.at(i);
				tempFAgent->SetLeader(listeFollowerL2.at(i + 1));
				tempFAgent->UpdateSteering();
			}
			listeFollowerL2.at(listeFollowerL2.size() - 1)->SetLeader(m_Leaders.at(1));
			listeFollowerL2.at(listeFollowerL2.size() - 1)->UpdateSteering();
			break;
		default:
			break;
		}
		break;
	}
	//--------------------------------------------------------------//


	//-------------- FlockingV behavior ----------------------------//
	//--------------------------------------------------------------//
	case 2:
	{
		for (int a = 0; a < nb_agents; ++a)
		{
			Vector2D SpawnPos = Vector2D(cx / 2.0 + RandomClamped()*cx / 2.0,
				cy / 2.0 + RandomClamped()*cy / 2.0);
			// Create the agent
			pVehicle = new Vehicle(this,
				SpawnPos,                 //initial position
				RandFloat()*TwoPi,        //start rotation
				Vector2D(0, 0),            //velocity
				Prm.VehicleMass,          //mass
				Prm.MaxSteeringForce,     //max force
				Prm.MaxSpeed - 200,             //max velocity
				Prm.MaxTurnRatePerSecond, //max turn rate
				Prm.VehicleScale);        //scale

										  // FlokingV behavior
			pVehicle->Steering()->FlockingVOn();

			if (!m_aHuman & agent_humain == 1) {
				m_aHuman = true;
				m_HumanAgent = pVehicle;
			}

			//add dans la liste de vehicule
			m_Vehicles.push_back(pVehicle);
			//add it to the cell subdivision
			m_pCellSpace->AddEntity(pVehicle);
		}
		break;
	}
	/*default:
	pVehicle->Steering()->WanderOn();*/
	}

}


GameWorld::GameWorld(int cx, int cy):
	GameWorld(cx, cy, 0, 0, 0, Prm.NumAgents, 0, 0, 0, 15)
{}

//-------------------------------- dtor ----------------------------------
//------------------------------------------------------------------------
GameWorld::~GameWorld()
{
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    delete m_Vehicles[a];
  }

  for (unsigned int a = 0; a<m_Leaders.size(); ++a)
  {
	  delete m_Leaders[a];
  }

  for (unsigned int ob=0; ob<m_Obstacles.size(); ++ob)
  {
    delete m_Obstacles[ob];
  }

  delete m_pCellSpace;
  
  delete m_pPath;
}


//----------------------------- Update -----------------------------------
//------------------------------------------------------------------------
void GameWorld::Update(double time_elapsed)
{ 
  if (m_bPaused) return;

  //create a smoother to smooth the framerate
  const int SampleRate = 10;
  static Smoother<double> FrameRateSmoother(SampleRate, 0.0);

  m_dAvFrameTime = FrameRateSmoother.Update(time_elapsed);
  
  // TODO update leaders

  //update the vehicles
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    m_Vehicles[a]->Update(time_elapsed);
  }
}
  

//--------------------------- CreateWalls --------------------------------
//
//  creates some walls that form an enclosure for the steering agents.
//  used to demonstrate several of the steering behaviors
//------------------------------------------------------------------------
void GameWorld::CreateWalls()
{
  //create the walls  
  double bordersize = 20.0;
  double CornerSize = 0.2;
  double vDist = m_cyClient-2*bordersize;
  double hDist = m_cxClient-2*bordersize;
  
  const int NumWallVerts = 8;

  Vector2D walls[NumWallVerts] = {Vector2D(hDist*CornerSize+bordersize, bordersize),
                                   Vector2D(m_cxClient-bordersize-hDist*CornerSize, bordersize),
                                   Vector2D(m_cxClient-bordersize, bordersize+vDist*CornerSize),
                                   Vector2D(m_cxClient-bordersize, m_cyClient-bordersize-vDist*CornerSize),
                                         
                                   Vector2D(m_cxClient-bordersize-hDist*CornerSize, m_cyClient-bordersize),
                                   Vector2D(hDist*CornerSize+bordersize, m_cyClient-bordersize),
                                   Vector2D(bordersize, m_cyClient-bordersize-vDist*CornerSize),
                                   Vector2D(bordersize, bordersize+vDist*CornerSize)};
  
  for (int w=0; w<NumWallVerts-1; ++w)
  {
    m_Walls.push_back(Wall2D(walls[w], walls[w+1]));
  }

  m_Walls.push_back(Wall2D(walls[NumWallVerts-1], walls[0]));
}


//--------------------------- CreateObstacles -----------------------------
//
//  Sets up the vector of obstacles with random positions and sizes. Makes
//  sure the obstacles do not overlap
//------------------------------------------------------------------------
void GameWorld::CreateObstacles()
{
    //create a number of randomly sized tiddlywinks
  for (int o=0; o<Prm.NumObstacles; ++o)
  {   
    bool bOverlapped = true;

    //keep creating tiddlywinks until we find one that doesn't overlap
    //any others.Sometimes this can get into an endless loop because the
    //obstacle has nowhere to fit. We test for this case and exit accordingly

    int NumTrys = 0; int NumAllowableTrys = 2000;

    while (bOverlapped)
    {
      NumTrys++;

      if (NumTrys > NumAllowableTrys) return;
      
      int radius = RandInt((int)Prm.MinObstacleRadius, (int)Prm.MaxObstacleRadius);

      const int border                 = 10;
      const int MinGapBetweenObstacles = 20;

      Obstacle* ob = new Obstacle(RandInt(radius+border, m_cxClient-radius-border),
                                  RandInt(radius+border, m_cyClient-radius-30-border),
                                  radius);

      if (!Overlapped(ob, m_Obstacles, MinGapBetweenObstacles))
      {
        //its not overlapped so we can add it
        m_Obstacles.push_back(ob);

        bOverlapped = false;
      }

      else
      {
        delete ob;
      }
    }
  }
}


//------------------------- Set Crosshair ------------------------------------
//
//  The user can set the position of the crosshair by right clicking the
//  mouse. This method makes sure the click is not inside any enabled
//  Obstacles and sets the position appropriately
//------------------------------------------------------------------------
void GameWorld::SetCrosshair(POINTS p)
{
  Vector2D ProposedPosition((double)p.x, (double)p.y);

  //make sure it's not inside an obstacle
  for (ObIt curOb = m_Obstacles.begin(); curOb != m_Obstacles.end(); ++curOb)
  {
    if (PointInCircle((*curOb)->Pos(), (*curOb)->BRadius(), ProposedPosition))
    {
      return;
    }

  }
  m_vCrosshair.x = (double)p.x;
  m_vCrosshair.y = (double)p.y;
}


//------------------------- HandleKeyPresses -----------------------------
void GameWorld::HandleKeyPresses(WPARAM wParam)
{

  switch(wParam)
  {
  case 'U':
    {
      delete m_pPath;
      double border = 60;
      m_pPath = new Path(RandInt(3, 7), border, border, cxClient()-border, cyClient()-border, true); 
      m_bShowPath = true; 
      for (unsigned int i=0; i<m_Vehicles.size(); ++i)
      {
        m_Vehicles[i]->Steering()->SetPath(m_pPath->GetPath());
      }
    }
    break;

    case 'P':
      
      TogglePause(); break;

    case 'O':

      ToggleRenderNeighbors(); break;

    case 'I':

      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->ToggleSmoothing();
        }

      }

      break;

    case 'Y':

       m_bShowObstacles = !m_bShowObstacles;

        if (!m_bShowObstacles)
        {
          m_Obstacles.clear();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOff();
          }
        }
        else
        {
          CreateObstacles();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOn();
          }
        }
        break;


	case 0x25 : // Left
		if (m_aHuman) {

			
			m_vCrosshair.x = (double)m_HumanAgent->Pos().x - 20;
			m_vCrosshair.y = (double)m_HumanAgent->Pos().y;
			m_HumanAgent->Steering()->SeekOn();

		}
		break;

	case 0x26: // Up
		if (m_aHuman) {

			
			m_vCrosshair.x = (double)m_HumanAgent->Pos().x;
			m_vCrosshair.y = (double)m_HumanAgent->Pos().y - 20;
			m_HumanAgent->Steering()->SeekOn();

		}
		break;

	case 0x27: // right
		if (m_aHuman) {


			m_vCrosshair.x = (double)m_HumanAgent->Pos().x + 20;
			m_vCrosshair.y = (double)m_HumanAgent->Pos().y;
			m_HumanAgent->Steering()->SeekOn();

		}
		break;

	case 0x28: // down
		if (m_aHuman) {


			m_vCrosshair.x = (double)m_HumanAgent->Pos().x;
			m_vCrosshair.y = (double)m_HumanAgent->Pos().y + 20 ;
			m_HumanAgent->Steering()->SeekOn();

		}
		break;

  }//end switch
}



//-------------------------- HandleMenuItems -----------------------------
void GameWorld::HandleMenuItems(WPARAM wParam, HWND hwnd)
{
  switch(wParam)
  {
    case ID_OB_OBSTACLES:

        m_bShowObstacles = !m_bShowObstacles;

        if (!m_bShowObstacles)
        {
          m_Obstacles.clear();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOff();
          }

          //uncheck the menu
         ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_UNCHECKED);
        }
        else
        {
          CreateObstacles();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOn();
          }

          //check the menu
          ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_CHECKED);
        }

       break;

    case ID_OB_WALLS:

      m_bShowWalls = !m_bShowWalls;

      if (m_bShowWalls)
      {
        CreateWalls();

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->WallAvoidanceOn();
        }

        //check the menu
         ChangeMenuState(hwnd, ID_OB_WALLS, MFS_CHECKED);
      }

      else
      {
        m_Walls.clear();

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->WallAvoidanceOff();
        }

        //uncheck the menu
         ChangeMenuState(hwnd, ID_OB_WALLS, MFS_UNCHECKED);
      }

      break;


    case IDR_PARTITIONING:
      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->ToggleSpacePartitioningOnOff();
        }

        //if toggled on, empty the cell space and then re-add all the 
        //vehicles
        if (m_Vehicles[0]->Steering()->isSpacePartitioningOn())
        {
          m_pCellSpace->EmptyCells();
       
          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_pCellSpace->AddEntity(m_Vehicles[i]);
          }

          ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_CHECKED);
        }
        else
        {
          ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_UNCHECKED);
          ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
          m_bShowCellSpaceInfo = false;

        }
      }

      break;

    case IDM_PARTITION_VIEW_NEIGHBORS:
      {
        m_bShowCellSpaceInfo = !m_bShowCellSpaceInfo;
        
        if (m_bShowCellSpaceInfo)
        {
          ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_CHECKED);

          if (!m_Vehicles[0]->Steering()->isSpacePartitioningOn())
          {
            SendMessage(hwnd, WM_COMMAND, IDR_PARTITIONING, NULL);
          }
        }
        else
        {
          ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
        }
      }
      break;
        

    case IDR_WEIGHTED_SUM:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::weighted_average);
        }
      }

      break;

    case IDR_PRIORITIZED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::prioritized);
        }
      }

      break;

    case IDR_DITHERED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_CHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::dithered);
        }
      }

      break;


      case ID_VIEW_KEYS:
      {
        ToggleViewKeys();

        CheckMenuItemAppropriately(hwnd, ID_VIEW_KEYS, m_bViewKeys);
      }

      break;

      case ID_VIEW_FPS:
      {
        ToggleShowFPS();

        CheckMenuItemAppropriately(hwnd, ID_VIEW_FPS, RenderFPS());
      }

      break;

      case ID_MENU_SMOOTHING:
      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->ToggleSmoothing();
        }

        CheckMenuItemAppropriately(hwnd, ID_MENU_SMOOTHING, m_Vehicles[0]->isSmoothingOn());
      }

      break;
      
  }//end switch
}


//------------------------------ Render ----------------------------------
//------------------------------------------------------------------------
void GameWorld::Render()
{
  gdi->TransparentText();

  //render any walls
  gdi->BlackPen();
  for (unsigned int w=0; w<m_Walls.size(); ++w)
  {
    m_Walls[w].Render(true);  //true flag shows normals
  }

  //render any obstacles
  gdi->BlackPen();
  
  for (unsigned int ob=0; ob<m_Obstacles.size(); ++ob)
  {
    gdi->Circle(m_Obstacles[ob]->Pos(), m_Obstacles[ob]->BRadius());
  }

  //render the agents
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    m_Vehicles[a]->Render();  
    
    //render cell partitioning stuff
    if (m_bShowCellSpaceInfo && a==0)
    {
      gdi->HollowBrush();
      InvertedAABBox2D box(m_Vehicles[a]->Pos() - Vector2D(Prm.ViewDistance, Prm.ViewDistance),
                           m_Vehicles[a]->Pos() + Vector2D(Prm.ViewDistance, Prm.ViewDistance));
      box.Render();

      gdi->RedPen();
      CellSpace()->CalculateNeighbors(m_Vehicles[a]->Pos(), Prm.ViewDistance);
      for (BaseGameEntity* pV = CellSpace()->begin();!CellSpace()->end();pV = CellSpace()->next())
      {
        gdi->Circle(pV->Pos(), pV->BRadius());
      }
      
      gdi->GreenPen();
      gdi->Circle(m_Vehicles[a]->Pos(), Prm.ViewDistance);
    }
  }  

//#define CROSSHAIR
#ifdef CROSSHAIR
  //and finally the crosshair
  gdi->RedPen();
  gdi->Circle(m_vCrosshair, 4);
  gdi->Line(m_vCrosshair.x - 8, m_vCrosshair.y, m_vCrosshair.x + 8, m_vCrosshair.y);
  gdi->Line(m_vCrosshair.x, m_vCrosshair.y - 8, m_vCrosshair.x, m_vCrosshair.y + 8);
  gdi->TextAtPos(5, cyClient() - 20, "Click to move crosshair");
#endif


  //gdi->TextAtPos(cxClient() -120, cyClient() - 20, "Press R to reset");

  gdi->TextColor(Cgdi::grey);
  if (RenderPath())
  {
     gdi->TextAtPos((int)(cxClient()/2.0f - 80), cyClient() - 20, "Press 'U' for random path");

     m_pPath->Render();
  }

  if (RenderFPS())
  {
    gdi->TextColor(Cgdi::grey);
    gdi->TextAtPos(5, cyClient() - 20, ttos(1.0 / m_dAvFrameTime));
  } 

  if (m_bShowCellSpaceInfo)
  {
    m_pCellSpace->RenderCells();
  }

}
