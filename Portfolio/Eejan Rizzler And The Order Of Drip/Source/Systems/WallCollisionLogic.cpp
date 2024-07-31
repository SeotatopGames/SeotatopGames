#include "WallCollisionLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"

using namespace EROD; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool EROD::PowerPelletLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher, EROD::D3D11RendererLogic& _renderer, Level_Data& _levelData)
{
	game = _game;
	gameConfig = _gameConfig;
	renderer = &_renderer;
	levelData = &_levelData;
	game->system<Player>()
		.each([this](flecs::entity& e, Player c) {
		playerEntity = e;
			});
game->system<Wall>("Wall System")
	.each([this](flecs::entity& e, Wall c)
		{
			collisionCount = 0;
			//be collected by the player
			e.each<CollidedWith>([&e, c, this](flecs::entity hit)
				{
					//playerEntity.get_mut<Player>()->downAllowed = true;
					//playerEntity.get_mut<Player>()->downAllowed = true;
					if (hit.has<Player>() && e.has<Wall>())
					{
						collisionCount++;
						colliding = true;
						flecs::ref<Player> player = hit.get_ref<Player>();
						auto transform = hit.get_mut<ModelTransform>()->modelTransform;
						playerZPos = hit.get_mut<ModelBoundary>()->boundary.center.z;
						float wallZ = e.get_mut<ModelBoundary>()->boundary.center.z;
						playerXPos = hit.get_mut<ModelBoundary>()->boundary.center.x;
						float wallX = e.get_mut<ModelBoundary>()->boundary.center.x;
						float distance = wallX - playerXPos;
						if (wallZ > playerZPos)
						{
							playerZOrig = playerZPos;
							player->upAllowed = false;
							player->downAllowed = true;
							player->leftSideAllowed = false;
							player->rightSideAllowed = false;
						}
						if (wallZ < playerZPos)
						{
							playerZOrig = playerZPos;
							player->downAllowed = false;
							player->upAllowed = true;
							player->leftSideAllowed = false;
							player->rightSideAllowed = false;
						}
						if (wallX < playerXPos)
						{
							player->leftSideAllowed = false;
							player->rightSideAllowed = true;
						}
						if(wallX > playerXPos)
						{
							player->rightSideAllowed = false;
							player->leftSideAllowed = true;
						}
						//e.remove<CollidedWith>();
						//transform->row4.x -= 2;
						//hit.get_mut<ModelTransform>()->modelTransform = transform;
						//auto boundary = hit.get_mut<ModelBoundary>()->boundary;
						//boundary.center.x -= 2;
						//hit.get_mut<ModelBoundary>()->boundary = boundary;
						////e.destruct();
						//e.remove<CollidedWith>();
						//hit.remove< CollidedWith>();

						//draw = true;
						//unsigned number = instanceEntity.get_mut<meshInstance>()->instanceTransformCount;
						//instanceEntity.get_mut<meshInstance>()->instanceTransformCount = number - 1;
						/*int index = e.get_mut<ModelBoundary>()->transformIndex;
						auto iter = levelData->levelTransforms.begin() + index;
						levelData->levelTransforms.erase(iter);*/
						//std::string name = e.get_mut<meshInstance>()->filename;
						//unsigned count = e.get_mut<meshInstance>()->instanceTransformCount;
						//e.get_mut<meshInstance>()->instanceTransformCount--;
						//count = e.get_mut<meshInstance>()->instanceTransformCount;
						//pelletData->ChangePelletTransform(e.get_mut<ModelBoundary>()->transformIndex);
						//e.get_mut<meshInstance>()->instanceTransformCount--;
						//GW::MATH::GMatrix proxy;
						//proxy.Create();
						//GW::MATH::GVECTORF translation = { 100, 0 , 0, 0 };
						//proxy.TranslateLocalF(e.get<ModelTransform>()->modelTransform, translation);
						//reduce number of remaining pellets
						//draw = false;
					}
				});
			if (collisionCount == 0)
			{
				playerEntity.get_mut<Player>()->downAllowed = true;
				playerEntity.get_mut<Player>()->upAllowed = true;
				playerEntity.get_mut<Player>()->rightSideAllowed = true;
				playerEntity.get_mut<Player>()->leftSideAllowed = true;
			}
		});
	return true;
}

// Free any resources used to run this system
bool EROD::PowerPelletLogic::Shutdown()
{
	game->entity("Wall System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool EROD::PowerPelletLogic::Activate(bool runSystem)
{

	return false;
}