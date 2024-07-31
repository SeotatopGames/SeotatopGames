// define all ECS components related to gameplay
#ifndef GAMEPLAY_H
#define GAMEPLAY_H

// example space game (avoid name collisions)
namespace EROD
{
	struct Damage { int value; };
	struct Health { int value; };
	struct Firerate { float value; };
	struct Cooldown { float value; };
	
	// gameplay tags (states)
	struct Firing {};
	struct Charging {};

	// powerups
	struct ChargedShot { int max_destroy; };

	struct PowerUp { bool poweredUp; XTime* timer; double currentTime = 0; };
};

#endif