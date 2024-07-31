#include "GhostData.h"
#include "../Components/Identification.h"
#include "../Components/visuals.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Components/Gameplay.h"

bool EROD::GhostData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	Level_Data& _data,
	Level_Data::LEVEL_MODEL& _model,
	Level_Data::MODEL_INSTANCES& _instance,
	int _entityType
)
{
	auto entity = _game->entity();
	entity.set<MeshInstance>({ _model.filename, _model.meshCount,_instance.transformCount,_model.vertexStart,
	_model.indexStart,_model.meshStart,_model.materialStart,_instance.transformStart,_model,_instance });
	entity.set<ModelTransform>({ &_data.levelTransforms[_instance.transformStart], _instance.transformStart });
	entity.set<ModelBoundary>({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart].row4.x,
									_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart].row4.y,
									_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart].row4.z,
									_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart].row4.w,
									1.5f, 1.5f, 1.5f, _data.levelColliders[_model.colliderIndex].extent.w,
									_data.levelColliders[_model.colliderIndex].rotation
		});
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	entity.set<Ghost>({ _data.levelTransforms[_instance.transformStart].row4.x, 
		_data.levelTransforms[_instance.transformStart].row4.z,
		0, 0, 0.0, 9.0f, 0.0 });
	entity.add<Collidable>();

	return true;
}

bool EROD::GhostData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Ghost&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	return true;
}
