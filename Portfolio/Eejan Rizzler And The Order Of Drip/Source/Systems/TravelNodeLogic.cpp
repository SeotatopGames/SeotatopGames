#include "TravelNodeLogic.h"
#include "../Events/Playevents.h"
#include "../Components/Model_Data.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"

using namespace EROD; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool EROD::TravelNodeLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher,
	Level_Data _levelData
)
{
	game = _game;
	gameConfig = _gameConfig;

	if (GenerateTravelMap() == false)
	{
		return false;
	}

	PlayerHitsLRNodes = game->system<TravelNode>("Player Hits Left Right Exit System")
		.each([this](flecs::entity& e, TravelNode c)
			{
				e.each<CollidedWith>([&e, c, this](flecs::entity hit)
					{
						auto integerX = 0;
						auto integerY = 0;
						if (hit.has<Player>() && exitLeft && e.get_mut<TravelNode>()->type == 17)
						{
							integerX = exitRight->x - 1.5;
							integerY = exitRight->y;
							flecs::ref<Player> p = hit.get_ref<Player>();
						}
						else if (hit.has<Player>() && exitRight && e.get_mut<TravelNode>()->type == 18)
						{
							integerX = exitLeft->x + 1.5;
							integerY = exitLeft->y;
							flecs::ref<Player> p = hit.get_ref<Player>();
						}
						hit.get_mut<ModelTransform>()->modelTransform->row4.x = integerX;
						hit.get_mut<ModelTransform>()->modelTransform->row4.z = integerY;
						hit.get_mut<ModelBoundary>()->boundary.center.x = integerX;
						hit.get_mut<ModelBoundary>()->boundary.center.z = integerY;
						e.remove<CollidedWith>();

					});
			});

	return true;
}

// Free any resources used to run this system
bool EROD::TravelNodeLogic::Shutdown()
{
	game->entity("Travel Map").destruct();
	return true;
}

// Toggle if a system's Logic is actively running
bool EROD::TravelNodeLogic::Activate(bool runSystem)
{

	return false;
}

bool EROD::TravelNodeLogic::GenerateTravelMap()
{	
	if (LinkNodes() == false)
	{
		return false;
	}
	if (LinkSpecials() == false)
	{
		return false;
	}
	return true;
}

bool EROD::TravelNodeLogic::LinkNodes()
{
	queryCache = game->query<TravelNode, ModelTransform>();
	// only happens once per frame at the very start of the frame
	struct TravelMap {}; // local definition so we control iteration count (singular)
	queryCache.each([this](flecs::entity e, TravelNode t, ModelTransform m)
		{
			e.get_mut<TravelNode>()->up = nullptr;
			e.get_mut<TravelNode>()->down = nullptr;
			e.get_mut<TravelNode>()->left = nullptr;
			e.get_mut<TravelNode>()->right = nullptr;
			switch (e.get<TravelNode>()->type)
			{
			case 7:
			{
				specialSpawn = e.get_mut<TravelNode>();
				break;
			}
			case 17:
			{
				exitLeft = e.get_mut<TravelNode>();
				break;
			}
			case 18:
			{
				exitRight = e.get_mut<TravelNode>();
				break;
			}
			default: { break; }
			}
			testCache.push_back(e);
		});


	float iX, iY, jX, jY, test;
	for (int i = 0; i < testCache.size(); ++i)
	{
		for (int j = 0; j < testCache.size(); ++j)
		{
			if (i != j)
			{
				iX = testCache[i].get<TravelNode>()->x;
				iY = testCache[i].get<TravelNode>()->y;
				jX = testCache[j].get<TravelNode>()->x;
				jY = testCache[j].get<TravelNode>()->y;
				test = sqrt(pow((jX - iX), 2.0f) + pow((jY - iY), 2.0f));
				if (test <= 2.1f)
				{
					float Xs = jX - iX;
					float Ys = jY - iY;
					bool a = abs(Xs) > 0.1;
					bool b = abs(Ys) > 0.1;
					bool c = abs(Ys) < 0.1;
					bool d = abs(Xs) < 0.1;
					if (a && c) //left or right
					{
						if (Xs < 0) //j left of i
						{
							testCache[j].get_mut<TravelNode>()->left = testCache[i].get_mut<TravelNode>();
							testCache[i].get_mut<TravelNode>()->right = testCache[j].get_mut<TravelNode>();
						}
						if (Xs > 0) //j right of i
						{
							testCache[j].get_mut<TravelNode>()->right = testCache[i].get_mut<TravelNode>();
							testCache[i].get_mut<TravelNode>()->left = testCache[j].get_mut<TravelNode>();
						}
					}
					if (b && d)//up or down
					{
						if (Ys > 0) //j up of i
						{
							testCache[j].get_mut<TravelNode>()->up = testCache[i].get_mut<TravelNode>();
							testCache[i].get_mut<TravelNode>()->down = testCache[j].get_mut<TravelNode>();
						}
						if (Ys < 0) //j down of i
						{
							testCache[j].get_mut<TravelNode>()->down = testCache[i].get_mut<TravelNode>();
							testCache[i].get_mut<TravelNode>()->up = testCache[j].get_mut<TravelNode>();
						}
					}
				}
			}
		}
	}
	queryCache.destruct();
	
	return true;
}

bool EROD::TravelNodeLogic::LinkSpecials()
{
	for (int i = 0; i < testCache.size(); ++i)
	{
		
		switch (testCache[i].get<TravelNode>()->type)
		{
		case 9: //ghost spawn
		{
			testCache[i].get_mut<TravelNode>()->up = specialSpawn;
			testCache[i].get_mut<TravelNode>()->left = nullptr;
			testCache[i].get_mut<TravelNode>()->right = nullptr;
			testCache[i].get_mut<TravelNode>()->down = nullptr;
			break;
		}
		case 10: //ghost spawn
		{
			testCache[i].get_mut<TravelNode>()->up = specialSpawn;
			testCache[i].get_mut<TravelNode>()->left = nullptr;
			testCache[i].get_mut<TravelNode>()->right = nullptr;
			testCache[i].get_mut<TravelNode>()->down = nullptr;
			break;
		}
		case 11: //ghost spawn
		{
			testCache[i].get_mut<TravelNode>()->up = specialSpawn;
			testCache[i].get_mut<TravelNode>()->left = nullptr;
			testCache[i].get_mut<TravelNode>()->right = nullptr;
			testCache[i].get_mut<TravelNode>()->down = nullptr;
			break;
		}
		case 12: //ghost spawn
		{
			testCache[i].get_mut<TravelNode>()->up = specialSpawn;
			testCache[i].get_mut<TravelNode>()->left = nullptr;
			testCache[i].get_mut<TravelNode>()->right = nullptr;
			testCache[i].get_mut<TravelNode>()->down = nullptr;
			break;
		}
		case 17: //exit left
		{
			testCache[i].get_mut<TravelNode>()->right = exitRight;
			break;
		}
		case 18: //exit right
		{
			testCache[i].get_mut<TravelNode>()->left = exitLeft;
			break;
		}
		default: { break; }
		}
		
	}

	return true;
}
