// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H


#include "Model_Data.h"
// example space game (avoid name collisions)
namespace EROD
{
	enum Ghosttype{	INKY = 0, BLINKY, PINKY, CLYDE };
	enum Direction 
	{
		UP = 0,
		LEFT,
		DOWN,
		RIGHT,
		START
	};
	enum Behavior 
	{
		IDLE = 0,
		CHASE,
		SCATTER,
		FRIGHTENED,
		EATEN
	};
	
	enum CollectableTypes { PELLET = 0, POWERPELLET, SPECIAL };
	struct TravelNode
	{
		float x, y;
		int type;
		TravelNode* up;
		TravelNode* down;
		TravelNode* left;
		TravelNode* right;
	};
	struct Ghost {
		float x, y;
		int identity;
		int behavior;
		float timer;
		float speed;
		float startupTimer;
		TravelNode* currentNode = nullptr;
		TravelNode* spawnNode = nullptr;
		TravelNode* targetNode = nullptr;
		TravelNode* scatterNode = nullptr;
		TravelNode* playerNode = nullptr;
		TravelNode* lastNode = nullptr;
	};
	struct Player { int score = 0; 
	TravelNode* currentNode = nullptr; 
	bool upAllowed = true;
	bool downAllowed = true;
	bool leftSideAllowed = true;
	bool rightSideAllowed = true;
	int lives = 2;
	unsigned gameLevel = 1;
	};
	struct ControllerID { unsigned index = 0; };
	struct Collectable { CollectableTypes type; };
	struct Bullet {};
	struct Enemy {};
	struct Lives {};
	struct GhostOnly {};
	struct Wall {};
	struct PelletTag {};
	struct PowerPelletTag {};
	struct PowerPelletTagEntity {};
	struct SpecialTag 
	{
		float speed;
		float timer;
		bool alive;
		TravelNode* spawnNode = nullptr;
		TravelNode* currentNode = nullptr;
		TravelNode* targetNode = nullptr;
		TravelNode* lastNode = nullptr;
	};
	struct PelletSpawn {};
	struct InkyNode {};
	struct BlinkyNode {};
	struct PinkyNode {};
	struct ClydeNode {};
	struct PowerPelletSpawn {};
	struct SpecialSpawn {};
	struct PlayerSpawn {};
	struct ExitLeft {};
	struct ExitRight {};
	struct TravelOnly {};
	struct RenderingTime {};
};

#endif