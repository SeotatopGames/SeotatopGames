// The Enemy system is responsible for enemy behaviors
#ifndef POWERPELLETLOGIC_H
#define POWERPELLETLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/PowerPelletData.h"
#include "VulkanRendererLogic.h"

// example space game (avoid name collisions)
namespace EROD
{
	class PowerPelletLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to events
		GW::CORE::GEventGenerator* eventPusher;
		EROD::D3D11RendererLogic* renderer;
		Level_Data* levelData;
		flecs::entity playerEntity;
		float playerZOrig = 0;
		float playerZPos = 0;
		float playerXOrig = 0;
		float playerXPos = 0;
		bool colliding = false;
		unsigned collisionCount = 0;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher, EROD::D3D11RendererLogic& _renderer, 
			Level_Data& _levelData);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif