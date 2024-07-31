#include "PelletData.h"
#include "Prefabs.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"

bool EROD::PelletData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	Level_Data& _data,
	Level_Data::LEVEL_MODEL& _model,
	Level_Data::MODEL_INSTANCES& _instance
	)
{
	auto entity = _game->entity(/*increment.c_str()*/)
		.set<MeshInstance>({ _model.filename, _model.meshCount,_instance.transformCount,_model.vertexStart,
		_model.indexStart,_model.meshStart,_model.materialStart,_instance.transformStart,_model,_instance }).add<PelletTag>().add<RenderingTime>();
	for (int i = 0; i < (int) _instance.transformCount; i++)
	{
		auto entity = _game->entity()
			.set<ModelTransform>({ &_data.levelTransforms[_instance.transformStart + i],_instance.transformStart + i })
			.set<ModelBoundary>({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart + i].row4.x,
											_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart + i].row4.y,
											_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart + i].row4.z,
											_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart + i].row4.w,
											_data.levelColliders[_model.colliderIndex].extent,
											_data.levelColliders[_model.colliderIndex].rotation, _instance.transformStart + i
				}) //Moves the center of the OBB to the world position of the current instance
			.set<Collectable>({ CollectableTypes::PELLET })
			//.add<PelletTag>()
		.add<Collidable>();
		// Grab init settings for players
		std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	}

	return true;
}

bool EROD::PelletData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, PelletTag&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	return true;
}