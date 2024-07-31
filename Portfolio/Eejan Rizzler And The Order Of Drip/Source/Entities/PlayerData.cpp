#include "PlayerData.h"
#include "../Components/Model_Data.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "Prefabs.h"

bool EROD::PlayerData::Load( std::shared_ptr<flecs::world> _game, 
                            std::weak_ptr<const GameConfig> _gameConfig, 
	                        Level_Data& _data,
							Level_Data::LEVEL_MODEL& _model,
							Level_Data::MODEL_INSTANCES& _instance,
                            XTime& timer)
{
	std::vector<GW::MATH::GOBBF> boundaries;
	boundaries.clear();
	for (int i = 0; i < (int)_instance.transformCount; i++)
	{
		boundaries.push_back({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart + i].row4.x,
								_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart + i].row4.y,
								_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart + i].row4.z,
								_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart + i].row4.w,
								0.95f, 0.95f, 0.95f, _data.levelColliders[_model.colliderIndex].extent.w,
								_data.levelColliders[_model.colliderIndex].rotation
			});
	}
	auto entity = _game->entity(_model.filename);
	entity.set<MeshInstance>({ _model.filename, _model.meshCount,_instance.transformCount,_model.vertexStart,
	_model.indexStart,_model.meshStart,_model.materialStart,_instance.transformStart,_model,_instance }).add<RenderingTime>();
	entity.set<ModelTransform>({ &_data.levelTransforms[_instance.transformStart],_instance.transformStart });
	entity.set<ModelBoundary>({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart].row4.x,
									_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart].row4.y,
									_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart].row4.z,
									_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart].row4.w,
									1.5f, 1.5f, 1.5f, _data.levelColliders[_model.colliderIndex].extent.w,
									_data.levelColliders[_model.colliderIndex].rotation
		});
	entity.set<PowerUp>({ false, &timer });
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// color

	// Create Player One
	entity.set<OrigPlayerPos>({ &_data.levelTransforms[_instance.transformStart] });
	entity.set<ControllerID>({ 0 });
	entity.add<Player>(); // Tag this entity as a Player
	entity.add<Collidable>();

	return true;
}

bool EROD::PlayerData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
		_game->each([](flecs::entity e, Player&) {
			e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

    return true;
}
