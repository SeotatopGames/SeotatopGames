#include "PhysicsLogic.h"
#include "../Components/Physics.h"
#include "../Components/Model_Data.h"
#include"../Components/Identification.h"

bool EROD::PhysicsLogic::Init(	std::shared_ptr<flecs::world> _game, 
								std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	// **** MOVEMENT ****
	game->system<Player>().each([this](flecs::entity& e, Player) {
		player = e;
		playerBound = e.get_mut<ModelBoundary>()->boundary;
		});
	// update velocity by acceleration
	game->system<Velocity, const Acceleration>("Acceleration System")
		.each([](flecs::entity e, Velocity& v, const Acceleration &a) 
		{
			GW::MATH2D::GVECTOR2F accel;
			GW::MATH2D::GVector2D::Scale2F(a.value, e.delta_time(), accel);
			GW::MATH2D::GVector2D::Add2F(accel, v.value, v.value);
		});
	// update position by velocity
	game->system<Position, const Velocity>("Translation System")
		.each([](flecs::entity e, Position& p, const Velocity &v) 
		{
			GW::MATH2D::GVECTOR2F speed;
			GW::MATH2D::GVector2D::Scale2F(v.value, e.delta_time(), speed);
			// adding is simple but doesn't account for orientation
			GW::MATH2D::GVector2D::Add2F(speed, p.value, p.value);
		});
	// **** CLEANUP ****
	// clean up any objects that end up offscreen
	game->system<const Position>("Cleanup System")
		.each([](flecs::entity e, const Position& p) 
		{
			if (p.value.x > 1.5f || p.value.x < -1.5f ||
			p.value.y > 1.5f || p.value.y < -1.5f) 
			{
				e.destruct();
			}
		});
	// **** COLLISIONS ****
	// due to wanting to loop through all collidables at once, we do this in two steps:
	// 1. A System will gather all collidables into a shared std::vector
	// 2. A second system will run after, testing/resolving all collidables against each other
	queryCache = game->query<Collidable, ModelTransform, ModelBoundary>();
	// only happens once per frame at the very start of the frame
	struct CollisionSystem {}; // local definition so we control iteration count (singular)
	game->entity("Detect-Collisions").add<CollisionSystem>();
	game->system<CollisionSystem>()
		.each([this](CollisionSystem& s) 
		{
			// collect any and all collidable objects
			queryCache.each([this](flecs::entity e, Collidable& c, ModelTransform& t, ModelBoundary& b) 
			{
				SHAPE polygon; // compute buffer for this objects polygon
				// This is critical, if you want to store an entity handle it must be mutable
				polygon.owner = e; // allows later changes
				polygon.ob = b.boundary;
				testCache.push_back(polygon);
			});
			// loop through the testCahe resolving all collisions
			// the inner loop starts at the entity after you so you don't double check collisions
				for (int j = 0; j < testCache.size(); ++j) 
				{
				// test the two world space polygons for collision
				// possibly make this cheaper by leaving one of them local and using an inverse matrix
				GW::MATH::GCollision::GCollisionCheck result;
				if (abs(playerBound.center.x - testCache[j].ob.center.x) <= playerBound.extent.x && abs(playerBound.center.z - testCache[j].ob.center.z) <= playerBound.extent.z)
				{
					GW::MATH::GCollision::TestOBBToOBBF(playerBound, testCache[j].ob, result);
					if (result == GW::MATH::GCollision::GCollisionCheck::COLLISION)
					{
						// Create an ECS relationship between the colliders
						// Each system can decide how to respond to this info independently
						testCache[j].owner.add<CollidedWith>(player);
						player.add<CollidedWith>(testCache[j].owner);
					}
					else
					{
						testCache[j].owner.remove<CollidedWith>(player);
						player.remove<CollidedWith>(testCache[j].owner);
					}
				}
				else
				{
					testCache[j].owner.remove<CollidedWith>(player);
					player.remove<CollidedWith>(testCache[j].owner);
				}
				}
			// wipe the test cache for the next frame (keeps capacity intact)
			testCache.clear();
		});
	return true;
}

bool EROD::PhysicsLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Acceleration System").enable();
		game->entity("Translation System").enable();
		game->entity("Cleanup System").enable();
	}
	else {
		game->entity("Acceleration System").disable();
		game->entity("Translation System").disable();
		game->entity("Cleanup System").disable();
	}
	return true;
}

bool EROD::PhysicsLogic::Shutdown()
{
	queryCache.destruct(); // fixes crash on shutdown
	game->entity("Acceleration System").destruct();
	game->entity("Translation System").destruct();
	game->entity("Cleanup System").destruct();
	game->entity("Detect-Collisions").destruct();
	return true;
}

GW::MATH::GOBBF EROD::PhysicsLogic::MatrixXOBB(GW::MATH::GOBBF ob, GW::MATH::GMATRIXF mat)
{
	GW::MATH::GOBBF result;
	GW::MATH::GMatrix::VectorXMatrixF(mat, ob.center, result.center);
	for (int i = 0; i < 3; ++i)
		GW::MATH::GMatrix::VectorXMatrixF(mat, ob.data[i], result.data[i]);
	GW::MATH::GMatrix::VectorXMatrixF(mat, ob.extent, result.extent);
	result.rotation = ob.rotation;
	return result;
}