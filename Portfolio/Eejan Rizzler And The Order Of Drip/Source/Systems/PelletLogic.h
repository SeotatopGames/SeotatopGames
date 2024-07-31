// The Enemy system is responsible for enemy behaviors
#ifndef PELLETLOGIC_H
#define PELLETLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/PelletData.h"
#include "VulkanRendererLogic.h"
#include "../Entities/PowerPelletData.h"
#include "../Components/Identification.h"

// example space game (avoid name collisions)
namespace EROD
{
	class PelletLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to events
		GW::CORE::GEventGenerator eventPusher;
		GW::SYSTEM::GLog gLog;
		flecs::entity spawnEntity;
		flecs::entity playerEntity;
		flecs::entity pelletMesh;
		flecs::entity powerPelletMesh;
		EROD::D3D11RendererLogic* renderer;
		Level_Data* levelData;
		std::vector<GW::MATH::GVECTORF> pelletPosHolder;
		flecs::query<Ghost> ghostHolder;
		GW::MATH::GVECTORF pelletPos;
		EROD::PelletData pelletData;
		EROD::PowerPelletData powerPelletData;
		GW::AUDIO::GAudio audioEngine;
		GW::AUDIO::GSound currentTrack;
		GW::AUDIO::GMusic* currentSong;
		int ghostCount = 0;
		/*GW::AUDIO::GMusic currTrack;*/
		float timeStart = 0.0;
		bool collected = false;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher, EROD::D3D11RendererLogic& renderer,
			Level_Data& _levelData, GW::SYSTEM::GLog log, GW::AUDIO::GAudio _audioEngine,
			EROD::PelletData& _pelletData,EROD::PowerPelletData& _powerPelletData,GW::AUDIO::GMusic& _currentSong);
		// control if the system is actively running
		//bool Activate(bool runSystem);
		void EatGhost();
		void ResetGhost();
		void ResetPellets();
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif