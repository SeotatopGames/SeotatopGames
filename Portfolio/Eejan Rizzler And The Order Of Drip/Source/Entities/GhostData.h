#include "../GameConfig.h"

#pragma once
namespace EROD {
	class GhostData
	{
	public:
		// Load required entities and/or prefabs into the ECS 
		bool Load(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			Level_Data& _data,
			Level_Data::LEVEL_MODEL& _model,
			Level_Data::MODEL_INSTANCES& _instance,
			int _entityType
		);

		// Unload the entities/prefabs from the ECS
		bool Unload(std::shared_ptr<flecs::world> _game);
	};
}
