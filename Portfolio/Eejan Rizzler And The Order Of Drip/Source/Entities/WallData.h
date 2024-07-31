#ifndef WALLDATA_H
#define WALLDATA_H
// Contains our global game settings
#include "../GameConfig.h"
namespace EROD
{
	class WallData
	{
	public:
		// Load required entities and/or prefabs into the ECS 
		bool Load(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			Level_Data& _data,
			Level_Data::LEVEL_MODEL& _model,
			Level_Data::MODEL_INSTANCES& _instance
			);

		// Unload the entities/prefabs from the ECS
		bool Unload(std::shared_ptr<flecs::world> _game);
	};
}
#endif