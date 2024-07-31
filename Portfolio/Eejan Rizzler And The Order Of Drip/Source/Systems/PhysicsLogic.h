// The level system is responsible for transitioning the various levels in the game
#ifndef PHYSICSLOGIC_H
#define PHYSICSLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Physics.h"
#include "../Components/Model_Data.h"

// example space game (avoid name collisions)
namespace EROD
{
	class PhysicsLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// used to cache collision queries
		flecs::query<Collidable, ModelTransform, ModelBoundary> queryCache;
		// defines what to be tested
		struct SHAPE {
			GW::MATH::GOBBF ob;
			flecs::entity owner;
		};
		flecs::entity player;
		GW::MATH::GOBBF playerBound;
		// vector used to save/cache all active collidables
		std::vector<SHAPE> testCache;
	public:
		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();

		GW::MATH::GOBBF MatrixXOBB(GW::MATH::GOBBF ob, GW::MATH::GMATRIXF mat);
	};

};

#endif