#pragma once


#include "../GameConfig.h"
#include "../Entities/GhostData.h"
#include "VulkanRendererLogic.h"
#include "../Components/Identification.h"


namespace EROD 
{
	class GhostLogic
	{
		std::shared_ptr<flecs::world> game;
		std::weak_ptr<const GameConfig> gameConfig;
		flecs::query<Ghost> ghostHolder;
		flecs::system ghostSystem;
		flecs::system ghostCollisionSystem;
		flecs::system ghostHitsPlayer;
		flecs::system ghostMovement;
		flecs::entity playerSpawn;
		flecs::entity playerEntity;
		EROD::D3D11RendererLogic* renderer;
		GW::SYSTEM::GLog gLog;
		GW::AUDIO::GAudio audioEngine;
		GW::AUDIO::GSound currentTrack;
		GW::AUDIO::GMusic* currentSong;
		Level_Data* level_Data;
		std::vector<flecs::entity> travelNodeCache;
		flecs::query<TravelNode, ModelTransform> travelNodeQuery;
		std::vector<flecs::entity> ghostCache;
		flecs::query<Ghost, ModelTransform> ghostQuery;
		bool* pause;
		
	public:
		bool Init(std::shared_ptr<flecs::world> _game, 
			std::weak_ptr<const GameConfig> _gameConfig, 
			EROD::D3D11RendererLogic& _renderer, 
			GW::SYSTEM::GLog _log, 
			GW::AUDIO::GAudio _audioEngine, 
			Level_Data& _levelData,GW::AUDIO::GMusic& _currentSong,
			bool* _pause);
		bool Activate(bool runSystem);
		void ResetGhost();
		bool Shutdown();
	};
}