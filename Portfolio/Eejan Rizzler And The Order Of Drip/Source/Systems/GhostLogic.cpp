#include "GhostLogic.h"
#include "../Components/Identification.h"
#include "../Components/visuals.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Components/Gameplay.h"


bool EROD::GhostLogic::Init(std::shared_ptr<flecs::world> _game, 
	std::weak_ptr<const GameConfig> _gameConfig, 
	EROD::D3D11RendererLogic& _renderer, 
	GW::SYSTEM::GLog _log, 
	GW::AUDIO::GAudio _audioEngine, 
	Level_Data& _levelData,GW::AUDIO::GMusic& _currentSong,
	bool* _pause)
{
	//Setup
	game = _game;
	gameConfig = _gameConfig;
	renderer = &_renderer;
	gLog = _log;
	level_Data = &_levelData;
	audioEngine = _audioEngine;
	currentSong = &_currentSong;
	pause = _pause;
	ghostHolder = game->query<Ghost>();
	game->system<PlayerSpawn>()
		.each([this](flecs::entity& it, PlayerSpawn) {
		playerSpawn = it;
			});
	game->system<Player>()
		.each([this](flecs::entity& it, Player) {
		playerEntity = it;
			});
  
		ghostHitsPlayer = game->system<Ghost>("Ghost Hit Player System")
			.each([this](flecs::entity& e, Ghost c)
			{
				e.each<CollidedWith>([&e, c, this](flecs::entity hit)
				{
					if (hit.has<Player>() && e.has<Ghost>())
					{
						flecs::ref<Player> p = hit.get_ref<Player>();
						if (!hit.get_mut<PowerUp>()->poweredUp)
						{
							auto integerX = playerSpawn.get_mut<TravelNode>()->x;
							auto integerY = playerSpawn.get_mut<TravelNode>()->y;
							gLog.LogCategorized("MESSAGE", "GHOST HAS HIT PLAYER");
							if (p->lives != 0)
							{
								currentTrack.Create("../SoundFX/mixkit-arcade-retro-game-over-213.wav", audioEngine, 0.5f);
								currentTrack.Play();
								p->lives -= 1;
							}
							else
							{
								currentSong->Stop();
								currentSong->Create("../SoundFX/mixkit-retro-game-over-1947.wav", audioEngine, 0.5f);
								currentSong->Play(false);
								renderer->YouLose();
							}
							renderer->UpdateLivesText(p->lives);
							hit.get_mut<ModelTransform>()->modelTransform->row4.x = integerX;
							hit.get_mut<ModelTransform>()->modelTransform->row4.z = integerY;
							hit.get_mut<ModelBoundary>()->boundary.center.x = integerX;
							hit.get_mut<ModelBoundary>()->boundary.center.z = integerY;
							e.remove<CollidedWith>();
							ResetGhost();
						}
						else
						{
							p->score += 300;
							renderer->UpdateScoreText(p->score);
							auto ghostSpawnNode = e.get_mut<Ghost>();
							e.get_mut<ModelTransform>()->modelTransform->row4.x = ghostSpawnNode->spawnNode->x;
							e.get_mut<ModelTransform>()->modelTransform->row4.z = ghostSpawnNode->spawnNode->y;
							e.get_mut<ModelBoundary>()->boundary.center.x = ghostSpawnNode->spawnNode->x;
							e.get_mut<ModelBoundary>()->boundary.center.z = ghostSpawnNode->spawnNode->y;
							ghostSpawnNode->currentNode = ghostSpawnNode->spawnNode;
							ghostSpawnNode->targetNode = ghostSpawnNode->spawnNode;
						}
					}
				});
			});

		travelNodeQuery = game->query<TravelNode, ModelTransform>();
		travelNodeQuery.each([this](flecs::entity e, TravelNode t, ModelTransform m)
			{
				travelNodeCache.push_back(e);
			});
		
		ghostQuery = game->query<Ghost, ModelTransform>();
		ghostQuery.each([this](flecs::entity e, Ghost g, ModelTransform m)
			{
				e.get_mut<Ghost>()->playerNode = nullptr;
				e.get_mut<Ghost>()->scatterNode = nullptr;
				e.get_mut<Ghost>()->spawnNode = nullptr;
				e.get_mut<Ghost>()->targetNode = nullptr;
				e.get_mut<Ghost>()->currentNode = nullptr;
				e.get_mut<Ghost>()->lastNode = nullptr;
			});

		travelNodeQuery.destruct();
		ghostQuery.destruct();

		ghostMovement = game->system<Ghost, ModelTransform, ModelBoundary>("Ghost Movement System")
			.each([this](flecs::entity& e, Ghost c, ModelTransform mT, ModelBoundary b)
				{
					if (*pause)
						return;
					//if (sqrt(pow((-c.x), 2.0f) + pow((jY - iY), 2.0f)) //distance check for future use
          
					//If ghost homenode == nullptr //This should initialize the system once
					//	Collision check for home node 
					//	Distance check Ghost->TravelNode
					//	If distance less than 0.1f set TravelNode to Ghost->HomeNode
					//	set current node to home node
					//	set target node to home node

					float xG = e.get<ModelTransform>()->modelTransform->row4.x;
					float yG = e.get<ModelTransform>()->modelTransform->row4.z;
					float tolerance = 0.1f;
					if (e.get<Ghost>()->spawnNode == nullptr)
					{
						float xT, yT;
						for (int i = 0; i < travelNodeCache.size(); ++i)
						{
							xT = travelNodeCache[i].get<TravelNode>()->x;
							yT = travelNodeCache[i].get<TravelNode>()->y;
							if (sqrt(pow((xT - xG), 2.0f) + pow((yT - yG), 2.0f) <= tolerance))
							{
								e.get_mut<Ghost>()->spawnNode = travelNodeCache[i].get_mut<TravelNode>();
								e.get_mut<Ghost>()->currentNode = travelNodeCache[i].get_mut<TravelNode>();
								e.get_mut<Ghost>()->targetNode = travelNodeCache[i].get_mut<TravelNode>();
							}
						}
					}			

					//every frame
					//distance check check for travelnode
					//if distanceof ghost->target < 0.1f
					//	set current node to target
					float xT = e.get<Ghost>()->targetNode->x;
					float yT = e.get<Ghost>()->targetNode->y;
					if (e.get<Ghost>()->targetNode != nullptr)
					{
						if (sqrt(pow((xT - xG), 2.0f) + pow((yT - yG), 2.0f) <= tolerance))
						{
							e.get_mut<Ghost>()->lastNode = e.get_mut<Ghost>()->currentNode;
							e.get_mut<Ghost>()->currentNode = e.get_mut<Ghost>()->targetNode;
						}
					}

					//if current node == target node 
					//	targetnode = nullptr
					//	while (target node == nullptr)
					//		rand int 0->3 (0 == up, etc...)
					//		if rand == 0 and currentnode->up != nullptr
					//			set target node to up //do for each direction
					
					if (e.get<Ghost>()->targetNode == e.get<Ghost>()->currentNode)
					{
						e.get_mut<Ghost>()->targetNode = nullptr;
						while (e.get<Ghost>()->targetNode == nullptr)
						{
							int random = rand() % 4;
							if (random == 0 && e.get<Ghost>()->currentNode->up != nullptr && e.get<Ghost>()->currentNode->up != e.get<Ghost>()->lastNode)
							{
								e.get_mut<Ghost>()->targetNode = e.get_mut<Ghost>()->currentNode->up;
								break;
							}
							if (random == 1 && e.get<Ghost>()->currentNode->down != nullptr && e.get<Ghost>()->currentNode->down != e.get<Ghost>()->lastNode)
							{
								e.get_mut<Ghost>()->targetNode = e.get_mut<Ghost>()->currentNode->down;
								break;
							}
							if (random == 2 && e.get<Ghost>()->currentNode->left != nullptr && e.get<Ghost>()->currentNode->left != e.get<Ghost>()->lastNode)
							{
								e.get_mut<Ghost>()->targetNode = e.get_mut<Ghost>()->currentNode->left;
								break;
							}
							if (random == 3 && e.get<Ghost>()->currentNode->right != nullptr && e.get<Ghost>()->currentNode->right != e.get<Ghost>()->lastNode)
							{
								e.get_mut<Ghost>()->targetNode = e.get_mut<Ghost>()->currentNode->right;
								break;
							}
						}
					}

					//if target node != nullptr
					//	lerp ghost to target node using r = ghost -> speed

					if (e.get<Ghost>()->targetNode != nullptr)
					{
						 //replace this with the actual current level once an entity or reference to it is available
						unsigned currentLevel = playerEntity.get_mut<Player>()->gameLevel;
						xG = xG + (e.get_mut<Ghost>()->targetNode->x - e.get_mut<Ghost>()->currentNode->x)
							* e.get_mut<Ghost>()->speed * e.delta_time() * ((currentLevel + 1.0f) * 0.1f);
						yG = yG + (e.get_mut<Ghost>()->targetNode->y - e.get_mut<Ghost>()->currentNode->y)
							* e.get_mut<Ghost>()->speed * e.delta_time() * ((currentLevel + 1.0f) * 0.1f);

						//if (abs(xG) > abs(e.get<Ghost>()->targetNode->x) + tolerance && sqrt(pow((xT - e.get<ModelTransform>()->modelTransform->row4.x), 2.0f) + pow((yT - e.get<ModelTransform>()->modelTransform->row4.z), 2.0f) > tolerance))
						//{
						//	xG = e.get<Ghost>()->targetNode->x;
						//	gLog.Log("ghost overshoot prevented X");
						//}
						//if (abs(yG) > abs(e.get<Ghost>()->targetNode->y) + tolerance && sqrt(pow((xT - e.get<ModelTransform>()->modelTransform->row4.x), 2.0f) + pow((yT - e.get<ModelTransform>()->modelTransform->row4.z), 2.0f) > tolerance))
						//{
						//	yG = e.get<Ghost>()->targetNode->y;
						//	gLog.Log("ghost overshoot prevented Y");
						//} //failed attempt at overshoot prevention

						if (e.get<Ghost>()->targetNode->type == 17) //ghost hits exit right so its target is exits left
						{
							xG = e.get<Ghost>()->targetNode->right->x;
							yG = e.get<Ghost>()->targetNode->right->y;
							e.get_mut<Ghost>()->lastNode = e.get_mut<Ghost>()->targetNode; //exit left
							e.get_mut<Ghost>()->currentNode = e.get_mut<Ghost>()->targetNode->right; //right of exit left
							e.get_mut<Ghost>()->targetNode = e.get_mut<Ghost>()->currentNode->right; //right, right of exit left
						}

						if (e.get<Ghost>()->targetNode->type == 18)
						{
							xG = e.get<Ghost>()->targetNode->left->x;
							yG = e.get<Ghost>()->targetNode->left->y;
							e.get_mut<Ghost>()->lastNode = e.get_mut<Ghost>()->targetNode; //exit right
							e.get_mut<Ghost>()->currentNode = e.get_mut<Ghost>()->targetNode->left; //left of exit right
							e.get_mut<Ghost>()->targetNode = e.get_mut<Ghost>()->currentNode->left; //left, left of exit right
						}

						e.get_mut<ModelTransform>()->modelTransform->row4.x = xG;
						e.get_mut<ModelTransform>()->modelTransform->row4.z = yG;
						e.get_mut<ModelBoundary>()->boundary.center.x = xG;
						e.get_mut<ModelBoundary>()->boundary.center.z = yG;
					}
				});

	return true;
}

bool EROD::GhostLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Ghost System").enable();
	}
	else {
		game->entity("Ghost System").disable();
	}
	return false;
}

bool EROD::GhostLogic::Shutdown()
{
	game->entity("Ghost System").destruct();
	game->entity("Ghost Hit Player System").destruct();
	game->entity("Ghost Movement System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}
void EROD::GhostLogic::ResetGhost()
{
	ghostHolder.each([this](flecs::entity& e, Ghost)
		{
			auto ghostSpawnNode = e.get_mut<Ghost>();
			e.get_mut<ModelTransform>()->modelTransform->row4.x = ghostSpawnNode->spawnNode->x;
			e.get_mut<ModelTransform>()->modelTransform->row4.z = ghostSpawnNode->spawnNode->y;
			e.get_mut<ModelBoundary>()->boundary.center.x = ghostSpawnNode->spawnNode->x;
			e.get_mut<ModelBoundary>()->boundary.center.z = ghostSpawnNode->spawnNode->y;
			ghostSpawnNode->currentNode = ghostSpawnNode->spawnNode;
			ghostSpawnNode->targetNode = ghostSpawnNode->spawnNode;
		});
	//game->entity("Ghost Reset").destruct();
}
