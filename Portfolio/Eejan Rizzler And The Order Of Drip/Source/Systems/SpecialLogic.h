// The Enemy system is responsible for enemy behaviors
#ifndef SPECIALLOGIC_H
#define SPECIALLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/SpecialData.h"
#include "../Components/Identification.h"

// example space game (avoid name collisions)
namespace EROD
{
	class SpecialLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to events
		GW::CORE::GEventGenerator eventPusher;
		flecs::system specialMovementSystem;
		flecs::entity playerEntity;
		GW::SYSTEM::GLog gLog;
		GW::AUDIO::GAudio audioEngine;
		GW::AUDIO::GSound currentTrack;
		GW::AUDIO::GMusic* currentSong;
		flecs::query<TravelNode, ModelTransform> travelNodeQuery;
		TravelNode* specialSpawnNode;
		float offScreenX;
		float offScreenY;
		bool* pause;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher,
			bool* _pause);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif