// The Enemy system is responsible for enemy behaviors
#ifndef TRAVELNODELOGIC_H
#define TRAVELLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/NodeData.h"
#include "../Components/Model_Data.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"

// example space game (avoid name collisions)
namespace EROD
{
	class TravelNodeLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to events
		GW::CORE::GEventGenerator eventPusher;
		flecs::query<TravelNode, ModelTransform> queryCache;
		std::vector<flecs::entity> testCache;
		TravelNode* specialSpawn;
		TravelNode* exitLeft;
		TravelNode* exitRight;
		flecs::system PlayerHitsLRNodes;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher,
			Level_Data _levelData);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
		bool GenerateTravelMap();
		bool LinkNodes();
		bool LinkSpecials();
	};
};

#endif