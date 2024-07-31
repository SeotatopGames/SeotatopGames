#include "PowerPelletData.h"
#include "../Components/Model_Data.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool EROD::PowerPelletData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	Level_Data& _data,
	Level_Data::LEVEL_MODEL& _model,
	Level_Data::MODEL_INSTANCES& _instance
)
{
	auto entity = _game->entity();
	entity.set<MeshInstance>({ _model.filename, _model.meshCount,_instance.transformCount,_model.vertexStart,
	_model.indexStart,_model.meshStart,_model.materialStart,_instance.transformStart,_model,_instance }).add<PowerPelletTag>();
	for(int i = 0; i < _instance.transformCount; ++i)
	{
		auto entity = _game->entity()
			.set<ModelTransform>({ &_data.levelTransforms[_instance.transformStart + i],_instance.transformStart + i })
			.set<ModelBoundary>({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart + i].row4.x,
										_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart + i].row4.y,
										_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart + i].row4.z,
										_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart + i].row4.w,
										_data.levelColliders[_model.colliderIndex].extent,
										_data.levelColliders[_model.colliderIndex].rotation, _instance.transformStart + i
				})
			.set<Collectable>({ CollectableTypes::POWERPELLET })
			.add<PowerPelletTagEntity>()
		.add<Collidable>();

	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	}

	return true;
}

bool EROD::PowerPelletData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, PowerPelletTagEntity&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	return true;
}