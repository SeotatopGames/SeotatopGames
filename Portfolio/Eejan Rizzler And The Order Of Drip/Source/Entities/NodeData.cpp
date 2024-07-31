#include "NodeData.h"
#include "../Components/Model_Data.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool EROD::NodeData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	Level_Data& _data,
	Level_Data::LEVEL_MODEL& _model,
	Level_Data::MODEL_INSTANCES& _instance,
	int _entityType
	)
{
	for (int i = 0; i < _instance.transformCount; i++)
	{
		auto entity = _game->entity();
		entity.set<ModelTransform>({ &_data.levelTransforms[_instance.transformStart],_instance.modelIndex });
		if (_entityType != 8)
		{
			entity.set<TravelNode>({ _data.levelTransforms[_instance.transformStart+i].row4.x, _data.levelTransforms[_instance.transformStart + i].row4.z, _entityType });
		}

		std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
		switch (_entityType)
		{
		case 6:
		{
			entity.add<PowerPelletSpawn>();
			break;
		}
		case 7:
		{
			entity.add<SpecialSpawn>();
			break;
		}
		case 8:
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
				.add<Wall>()
				.add<Collidable>();
			entity.add<GhostOnly>();
			break;
		}
		case 9:
		{
			entity.add<InkyNode>();
			break;
		}
		case 10:
		{
			entity.add<BlinkyNode>();
			break;
		}
		case 11:
		{
			entity.add<PinkyNode>();
			break;
		}
		case 12:
		{
			entity.add<ClydeNode>();
			break;
		}
		case 13:
		{
			entity.add<PlayerSpawn>();
			break;
		}
		case 17:
		{
			entity.set<ModelBoundary>({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart].row4.x,
									_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart].row4.y,
									_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart].row4.z,
									_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart].row4.w,
									_data.levelColliders[_model.colliderIndex].extent,
									_data.levelColliders[_model.colliderIndex].rotation
				});
			entity.add<ExitLeft>();
			entity.add<Collidable>();
			break;
		}
		case 18:
		{
			entity.set<ModelBoundary>({ _data.levelColliders[_model.colliderIndex].center.x + _data.levelTransforms[_instance.transformStart].row4.x,
											_data.levelColliders[_model.colliderIndex].center.y + _data.levelTransforms[_instance.transformStart].row4.y,
											_data.levelColliders[_model.colliderIndex].center.z + _data.levelTransforms[_instance.transformStart].row4.z,
											_data.levelColliders[_model.colliderIndex].center.w + _data.levelTransforms[_instance.transformStart].row4.w,
											_data.levelColliders[_model.colliderIndex].extent,
											_data.levelColliders[_model.colliderIndex].rotation
				});
			entity.add<ExitRight>();
			entity.add<Collidable>();
			break;
		}
		case 19:
		{
			entity.add<TravelOnly>();
			break;
		}
		case 20:
		{
			entity.add<PelletSpawn>();
			break;
		}
		}
	}

	return true;
}

bool EROD::NodeData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, TravelNode&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	return true;
}