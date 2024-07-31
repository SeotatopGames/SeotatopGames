#include "SpecialLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"

using namespace EROD; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool EROD::SpecialLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher,
	bool* _pause)
{
	game = _game;
	pause = _pause;

	game->system<Player>()
		.each([this](flecs::entity& it, Player) {
		playerEntity = it;
			});

	travelNodeQuery = game->query<TravelNode, ModelTransform>();
	travelNodeQuery.each([this](flecs::entity e, TravelNode t, ModelTransform m)
		{
			if (e.get<TravelNode>()->type == 7)
			{
				specialSpawnNode = e.get_mut<TravelNode>();
			}
		});
	travelNodeQuery.destruct();

	specialMovementSystem = game->system<SpecialTag, ModelTransform, ModelBoundary>("Special Movement System")
		.each([this](flecs::entity& e, SpecialTag c, ModelTransform mT, ModelBoundary b)
			{
				if (*pause)
					return;
				//Sets Special's spawn node to its spawn node once and for use by the rest of the script
				if (e.get<SpecialTag>()->spawnNode == nullptr)
				{
					e.get_mut<SpecialTag>()->spawnNode = specialSpawnNode;
				}

				//if the special is dead accumulate time, if the special is alive move around below
				if (e.get<SpecialTag>()->alive == false)
				{
					e.get_mut<SpecialTag>()->timer += e.delta_time();
					if (e.get<SpecialTag>()->timer >= 15.0f)
					{
						float xT = e.get<SpecialTag>()->spawnNode->x;
						float yT = e.get<SpecialTag>()->spawnNode->y;
						e.get_mut<SpecialTag>()->currentNode = specialSpawnNode;
						e.get_mut<SpecialTag>()->targetNode = specialSpawnNode;
						e.get_mut<ModelTransform>()->modelTransform->row4.x = xT;
						e.get_mut<ModelTransform>()->modelTransform->row4.z = yT;
						e.get_mut<ModelBoundary>()->boundary.center.x = xT;
						e.get_mut<ModelBoundary>()->boundary.center.z = yT;
						e.get_mut<SpecialTag>()->alive = true;
						e.get_mut<SpecialTag>()->timer = 0.0f;
					}
					else
					{
						return;
					}
				}

				float xG = e.get<ModelTransform>()->modelTransform->row4.x;
				float yG = e.get<ModelTransform>()->modelTransform->row4.z;

				//every frame
				//distance check check for travelnode
				//if distanceof ghost->target < 0.001f
				//	set current node to target

				if (e.get<SpecialTag>()->targetNode != nullptr)
				{
					float xT = e.get<SpecialTag>()->targetNode->x;
					float yT = e.get<SpecialTag>()->targetNode->y;
					if (sqrt(pow((xT - xG), 2.0f) + pow((yT - yG), 2.0f) <= 0.1f))
					{
						e.get_mut<SpecialTag>()->lastNode = e.get_mut<SpecialTag>()->currentNode;
						e.get_mut<SpecialTag>()->currentNode = e.get_mut<SpecialTag>()->targetNode;
					}
				}

				//if current node == target node 
				//	targetnode = nullptr
				//	while (target node == nullptr)
				//		rand int 0->3 (0 == up, etc...)
				//		if rand == 0 and currentnode->up != nullptr
				//			set target node to up //do for each direction

				if (e.get<SpecialTag>()->targetNode == e.get<SpecialTag>()->currentNode)
				{
					e.get_mut<SpecialTag>()->targetNode = nullptr;
					while (e.get<SpecialTag>()->targetNode == nullptr)
					{
						int random = rand() % 4;
						if (random == 0 && e.get<SpecialTag>()->currentNode->up != nullptr && e.get<SpecialTag>()->currentNode->up != e.get<SpecialTag>()->lastNode)
						{
							e.get_mut<SpecialTag>()->targetNode = e.get_mut<SpecialTag>()->currentNode->up;
							break;
						}
						if (random == 1 && e.get<SpecialTag>()->currentNode->down != nullptr && e.get<SpecialTag>()->currentNode->down != e.get<SpecialTag>()->lastNode)
						{
							e.get_mut<SpecialTag>()->targetNode = e.get_mut<SpecialTag>()->currentNode->down;
							break;
						}
						if (random == 2 && e.get<SpecialTag>()->currentNode->left != nullptr && e.get<SpecialTag>()->currentNode->left != e.get<SpecialTag>()->lastNode)
						{
							e.get_mut<SpecialTag>()->targetNode = e.get_mut<SpecialTag>()->currentNode->left;
							break;
						}
						if (random == 3 && e.get<SpecialTag>()->currentNode->right != nullptr && e.get<SpecialTag>()->currentNode->right != e.get<SpecialTag>()->lastNode)
						{
							e.get_mut<SpecialTag>()->targetNode = e.get_mut<SpecialTag>()->currentNode->right;
							break;
						}
					}
				}

				//if target node != nullptr
				//	lerp ghost to target node using r = ghost -> speed

				if (e.get<SpecialTag>()->targetNode != nullptr)
				{
					float currentLevel = playerEntity.get_mut<Player>()->gameLevel; //replace this with the actual current level once an entity or reference to it is available
					xG = xG + (e.get_mut<SpecialTag>()->targetNode->x - e.get_mut<SpecialTag>()->currentNode->x)
						* e.get_mut<SpecialTag>()->speed * e.delta_time() * ((currentLevel + 1.0f) * 0.1f);
					yG = yG + (e.get_mut<SpecialTag>()->targetNode->y - e.get_mut<SpecialTag>()->currentNode->y)
						* e.get_mut<SpecialTag>()->speed * e.delta_time() * ((currentLevel + 1.0f) * 0.1f);

					if (e.get<SpecialTag>()->targetNode->type == 17) //ghost hits exit right so its target is exits left
					{
						xG = e.get<SpecialTag>()->targetNode->right->x;
						yG = e.get<SpecialTag>()->targetNode->right->y;
						e.get_mut<SpecialTag>()->lastNode = e.get_mut<SpecialTag>()->targetNode; //exit left
						e.get_mut<SpecialTag>()->currentNode = e.get_mut<SpecialTag>()->targetNode->right; //right of exit left
						e.get_mut<SpecialTag>()->targetNode = e.get_mut<SpecialTag>()->currentNode->right; //right, right of exit left
					}

					if (e.get<SpecialTag>()->targetNode->type == 18)
					{
						xG = e.get<SpecialTag>()->targetNode->left->x;
						yG = e.get<SpecialTag>()->targetNode->left->y;
						e.get_mut<SpecialTag>()->lastNode = e.get_mut<SpecialTag>()->targetNode; //exit right
						e.get_mut<SpecialTag>()->currentNode = e.get_mut<SpecialTag>()->targetNode->left; //left of exit right
						e.get_mut<SpecialTag>()->targetNode = e.get_mut<SpecialTag>()->currentNode->left; //left, left of exit right
					}

					e.get_mut<ModelTransform>()->modelTransform->row4.x = xG;
					e.get_mut<ModelTransform>()->modelTransform->row4.z = yG;
					e.get_mut<ModelBoundary>()->boundary.center.x = xG;
					e.get_mut<ModelBoundary>()->boundary.center.z = yG;
				}
			});

	return true;
}

// Free any resources used to run this system
bool EROD::SpecialLogic::Shutdown()
{
	specialMovementSystem.destruct();
	return true;
}

// Toggle if a system's Logic is actively running
bool EROD::SpecialLogic::Activate(bool runSystem)
{

	return false;
}