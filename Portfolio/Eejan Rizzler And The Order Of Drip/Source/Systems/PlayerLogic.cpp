#include "PlayerLogic.h"
#include "../Components/Identification.h"
#include "../Components/Model_Data.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"

using namespace EROD; // Example Space Game
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool EROD::PlayerLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::INPUT::GInput _immediateInput,
	GW::INPUT::GBufferedInput _bufferedInput,
	GW::INPUT::GController _controllerInput,
	GW::AUDIO::GAudio _audioEngine,
	GW::CORE::GEventGenerator _eventPusher,
	bool* _pauseCheck,GW::AUDIO::GMusic& _currentSong)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput = _controllerInput;
	audioEngine = _audioEngine;
	currentKey = 0;
	oneTime = false;
	pauseCheck = _pauseCheck;
	currentSong = &_currentSong;
	pause = 0.0f;
	// Init any helper systems required for this task
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	float speed = (*readCfg).at("Player1").at("speed").as<float>();
	// add logic for updating players
	playerSystem = game->system<Player, ModelTransform, ControllerID, ModelBoundary>("Player System")
		.iter([this, speed](flecs::iter it, Player* p, ModelTransform* t, ControllerID* c, ModelBoundary* b) {

			for (auto i : it) 
			{
				// Use the controller/keyboard to move the player around the screen			
				if (c[i].index == 0) { // enable keyboard controls for player 1
					immediateInput.GetState(G_KEY_SPACE, pause);
					if (pause > 0.0f && oneTime == false)
					{
						*pauseCheck = !*pauseCheck;
						oneTime = true;
					}
					if (pause <= 0.0f)
					{
						oneTime = false;
					}

					if (*pauseCheck == false)
					{
						float xaxis = 0, yaxis = 0, input = 0;
						if (p->leftSideAllowed && (currentKey == 1 || currentKey == 0))
						{
							immediateInput.GetState(G_KEY_LEFT, input); xaxis -= input;
							immediateInput.GetState(G_KEY_A, input); xaxis -= input;
							//controllerInput.GetState(G_DPAD_LEFT_BTN, input); xaxis -= input;
							currentKey = 1;
							if (xaxis >= -0.1f)
							{
								currentKey = 0;
							}
						}
						if (!p->leftSideAllowed && currentKey == 1)
						{
							currentKey = 0;
						}
						if (p->rightSideAllowed && (currentKey == 2 || currentKey == 0))
						{
							immediateInput.GetState(G_KEY_RIGHT, input); xaxis += input;
							immediateInput.GetState(G_KEY_D, input); xaxis += input;
							//controllerInput.GetState(G_DPAD_RIGHT_BTN, input); xaxis += input;
							currentKey = 2;
							if (xaxis <= 0.1f)
							{
								currentKey = 0;
							}
						}
						if (!p->rightSideAllowed && currentKey == 2)
						{
							currentKey = 0;
						}
						if (p->upAllowed && (currentKey == 3 || currentKey == 0))
						{
							immediateInput.GetState(G_KEY_UP, input); yaxis += input;
							immediateInput.GetState(G_KEY_W, input); yaxis += input;
							//controllerInput.GetState(G_DPAD_UP_BTN, input); yaxis += input;
							currentKey = 3;
							if (yaxis <= 0.1f)
							{
								currentKey = 0;
							}
						}
						if (!p->upAllowed && currentKey == 3)
						{
							currentKey = 0;
						}
						if (p->downAllowed && (currentKey == 4 || currentKey == 0))
						{
							immediateInput.GetState(G_KEY_DOWN, input); yaxis -= input;
							immediateInput.GetState(G_KEY_S, input); yaxis -= input;
							//controllerInput.GetState(G_DPAD_DOWN_BTN, input); yaxis -= input;
							currentKey = 4;
							if (yaxis >= -0.1f)
							{
								currentKey = 0;
							}
						}
						if (!p->downAllowed && currentKey == 4)
						{
							currentKey = 0;
						}
						xaxis = G_LARGER(xaxis, -1);// cap right motion
						yaxis = G_LARGER(yaxis, -1);// cap up motion
						xaxis = G_SMALLER(xaxis, 1);// cap left motion
						yaxis = G_SMALLER(yaxis, 1);// cap down motion

						if (xaxis)
						{
							t->modelTransform->row4.x += xaxis * it.delta_time() * speed;
							b->boundary.center.x += xaxis * it.delta_time() * speed;
						}
						if (yaxis)
						{
							t->modelTransform->row4.z += yaxis * it.delta_time() * speed;
							b->boundary.center.z += yaxis * it.delta_time() * speed;
						}
					}
				}
			}
		});

	playerSystem = game->system<PowerUp>("Power-Up Timer")
		.each([this](PowerUp& p)
		{ 
		 if (p.poweredUp == true && !pauseCheck) 
	     { 
			 time += p.timer->TotalTime() * 600;
		   if (time >= 1)
		   {
			   p.poweredUp = false;
			   p.currentTime = 0;
			   time = 0;
			   currentSong->Stop();
		   }
	     }
	   });

	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame

	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	return true;
}

// Free any resources used to run this system
bool EROD::PlayerLogic::Shutdown()
{
	game->entity("Player System").destruct();
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool EROD::PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) {
		(runSystem) ?
			playerSystem.enable()
			: playerSystem.disable();
		return true;
	}
	return false;
}