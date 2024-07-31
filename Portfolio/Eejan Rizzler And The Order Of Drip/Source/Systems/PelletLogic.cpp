#include "PelletLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"

using namespace EROD; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool EROD::PelletLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher, EROD::D3D11RendererLogic& _renderer,
	Level_Data& _levelData, GW::SYSTEM::GLog _log, GW::AUDIO::GAudio _audioEngine, 
	EROD::PelletData& pelletData, EROD::PowerPelletData& _powerPelletData, GW::AUDIO::GMusic& _currentSong)
{
	game = _game;
	gameConfig = _gameConfig;
	renderer = &_renderer;
	levelData = &_levelData;
	gLog = _log;
	audioEngine = _audioEngine;
	powerPelletData = _powerPelletData;
	currentSong = &_currentSong;


	gLog.LogCategorized("MESSAGE", "PELLET TIME");
	game->system<PlayerSpawn>()
		.each([this](flecs::entity& e, PlayerSpawn c){
				spawnEntity = e;
				});
	game->system<PowerPelletTag>()
		.each([this](flecs::entity& e, PowerPelletTag c) {
		powerPelletMesh = e;
			});
	game->system<PelletTag>()
		.each([this](flecs::entity& e, PelletTag c) {
		pelletMesh = e;
			});
	game->system<Player>()
		.each([this](flecs::entity& e, Player c) {
		playerEntity = e;
			});
	ghostHolder = game->query<Ghost>();
	// destroy any bullets that have the CollidedWith relationship
	game->system<Collectable>("Pellet System")
		.each([this](flecs::entity& e, Collectable c)
		{
				bool play;
				currentSong->isPlaying(play);
				if (play == false)
				{
					currentSong->Create("../Music/Monplaisir_-_01_-_Welcome_to_Turbo_Rallye_Clicker_4000_Nitro_Futur_Edition(chosic.com).wav", audioEngine, 0.05f);
					currentSong->Play(true);
				}
				if (!playerEntity.get_mut<PowerUp>()->poweredUp && collected)
				{
					gLog.LogCategorized("MESSAGE", "POWER UP FALSE");
					collected = false;
				}
		//be collected by the player
			e.each<CollidedWith>([&e, c, this](flecs::entity hit)
			{
				if (hit.has<Player>())
				{
					flecs::ref<Player> p = hit.get_ref<Player>();
					if (c.type == CollectableTypes::POWERPELLET)
					{
                        currentTrack.Create("../SoundFX/mixkit-retro-game-notification-212.wav", audioEngine, 0.5f);
					    currentTrack.Play();
						currentSong->Stop();
						currentSong->Create("../Music/Loyalty_Freak_Music_-_04_-_Cant_Stop_My_Feet_(chosic.com).wav", audioEngine, 0.5f);
						currentSong->Play(true);
						EatGhost();
						//unsigned index = e.get_mut<ModelBoundary>()->transformIndex - instanceEntity.get_mut<MeshInstance>()->instanceTransformStart;
						p->score += 50;
						
					}
					if (c.type == CollectableTypes::PELLET)
					{						
						p->score += 10;
						renderer->DecrementPelletCount(1);
                        currentTrack.Create("../SoundFX/mixkit-arcade-game-jump-coin-216.wav", audioEngine, 0.5f);
					    currentTrack.Play();
					}
					if (c.type == CollectableTypes::SPECIAL)
					{
						e.get_mut<ModelTransform>()->modelTransform->row4.x = 300.0f;
						e.get_mut<ModelTransform>()->modelTransform->row4.z = 300.0f;
						e.get_mut<ModelBoundary>()->boundary.center.x = 300.0f;
						e.get_mut<ModelBoundary>()->boundary.center.z = 300.0f;
						e.get_mut<SpecialTag>()->alive = false;
						e.get_mut<SpecialTag>()->timer = 0.0f;
            
						int gameLevel = playerEntity.get_mut<Player>()->gameLevel;
						currentTrack.Create("../SoundFX/mixkit-retro-arcade-casino-notification-211.wav", audioEngine, 0.5f);
						currentTrack.Play();
						switch (p->gameLevel)
						{
						case 1: 
						{
							p->score += 100;
							break;
						}
						case 2:
						{
							p->score += 300;
							break;
						}
						case 3:
						{
							p->score += 500;
							break;
						}
						case 4:
						{
							p->score += 700;
							break;
						}
						case 5:
						{
							p->score += 1000;
							break;
						}
						case 6:
						{
							p->score += 3000;
							break;
						}
						default: //level >= 7
						{
							p->score += 5000;
							break;
						}
						}
					}
					else
					{
						auto index = e.get_mut<ModelBoundary>()->transformIndex;
						auto& pop = levelData->levelTransforms[e.get_mut<ModelBoundary>()->transformIndex];
						pelletPos = { pop.row4.x,pop.row4.y, pop.row4.z, (float)index };
						pelletPosHolder.push_back(pelletPos);
						pop.row4.x += 100;
						levelData->levelTransforms[e.get_mut<ModelBoundary>()->transformIndex] = pop;
						e.destruct();
					}
					renderer->UpdateScoreText(p->score);

					if (renderer->GetPelletCount() <= 0)
					{
						p->gameLevel += 1;
						currentSong->Stop();
						currentSong->Create("../SoundFX/winsquare-6993.wav", audioEngine, 0.5f);
						currentSong->Play(false);
						ResetPellets();
						ResetGhost();
						renderer->UpdateLevel(p->gameLevel);
						auto integerX = spawnEntity.get_mut<TravelNode>()->x;
						auto integerY = spawnEntity.get_mut<TravelNode>()->y;
						bool levelComplete = true;
						hit.get_mut<ModelTransform>()->modelTransform->row4.x = integerX;
						hit.get_mut<ModelTransform>()->modelTransform->row4.z = integerY;
						hit.get_mut<ModelBoundary>()->boundary.center.x = integerX;
						hit.get_mut<ModelBoundary>()->boundary.center.z = integerY;
					}
				}
			});
		});
	return true;
}

// Free any resources used to run this system
bool EROD::PelletLogic::Shutdown()
{
	game->entity("Pellet System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
//bool ESG::PelletLogic::Activate(bool runSystem)
//{
//	return false;
//}

void EROD::PelletLogic::EatGhost()
{
	if (!playerEntity.get_mut<PowerUp>()->poweredUp) 
	{
		gLog.LogCategorized("MESSAGE", "POWER UP TRUE");
		collected = true;
		playerEntity.get_mut<PowerUp>()->poweredUp = true;
		playerEntity.get_mut<PowerUp>()->timer->Restart();
		playerEntity.get_mut<PowerUp>()->timer->Signal();
	}
	//time.enable();
	//auto timePassed = time.delta_time();
}

void EROD::PelletLogic::ResetGhost()
{
	ghostHolder.each([this](flecs::entity& e, Ghost)
		{
			ghostCount++;
			auto ghostSpawnNode = e.get_mut<Ghost>();
			e.get_mut<ModelTransform>()->modelTransform->row4.x = ghostSpawnNode->spawnNode->x;
			e.get_mut<ModelTransform>()->modelTransform->row4.z = ghostSpawnNode->spawnNode->y;
			e.get_mut<ModelBoundary>()->boundary.center.x = ghostSpawnNode->spawnNode->x;
			e.get_mut<ModelBoundary>()->boundary.center.z = ghostSpawnNode->spawnNode->y;
			ghostSpawnNode->currentNode = ghostSpawnNode->spawnNode;
			ghostSpawnNode->targetNode = ghostSpawnNode->spawnNode;
		});
	ghostCount = ghostCount;
	//game->entity("Ghost Reset").destruct();
}

void EROD::PelletLogic::ResetPellets()
{
	gLog.LogCategorized("MESSAGE", "ALL PELLETS COLLECTED REFRESHING");
	auto meshInstance = pelletMesh.get_mut<MeshInstance>();
	auto powerPelletMeshInstance = powerPelletMesh.get_mut<MeshInstance>();
	auto indexStart = meshInstance->instanceTransformStart;
	for (int i = 0; i < pelletPosHolder.size(); ++i)
	{
		levelData->levelTransforms[pelletPosHolder[i].w].row4 = pelletPosHolder[i];
		levelData->levelTransforms[pelletPosHolder[i].w].row4.w = 1;
		
	}
	renderer->ResetPelletCount();
	pelletMesh.destruct();
	powerPelletMesh.destruct();
	pelletData.Load(game, gameConfig, *levelData, meshInstance->model, meshInstance->instanceNumber);
	powerPelletData.Unload(game);
	powerPelletData.Load(game, gameConfig, *levelData, powerPelletMeshInstance->model, powerPelletMeshInstance->instanceNumber);
}

