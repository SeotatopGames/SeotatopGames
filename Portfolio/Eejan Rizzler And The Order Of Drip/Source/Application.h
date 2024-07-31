#ifndef APPLICATION_H
#define APPLICATION_H

// include events
#include "Events/Playevents.h"
// Contains our global game settings
#include "GameConfig.h"
#include "Components/Model_Data.h"
// Load all entities+prefabs used by the game 
#include "Entities/PlayerData.h"
#include "Entities/GhostData.h"
#include "Entities/SpecialData.h"
#include "Entities/WallData.h"
#include "Entities/PelletData.h"
#include "Entities/PowerPelletData.h"
#include "Entities/NodeData.h"
// Include all systems used by the game and their associated components
#include "Systems/PlayerLogic.h"
#include "Systems/VulkanRendererLogic.h"
#include "Systems/LevelLogic.h"
#include "Systems/PhysicsLogic.h"
#include "Systems/PelletLogic.h"
#include "Systems/TravelNodeLogic.h"
#include "Systems/WallCollisionLogic.h"
#include "Systems/GhostLogic.h"
#include "Systems/SpecialLogic.h"

// Allocates and runs all sub-systems essential to operating the game
class Application 
{
	XTime playerTimer;
	bool isPaused = false;
	unsigned pelletNumber;
	// gateware libs used to access operating system
	GW::SYSTEM::GWindow window; // gateware multi-platform window
	GW::GRAPHICS::GDirectX11Surface d3d11;// gateware vulkan API wrapper
	GW::INPUT::GController gamePads; // controller support
	GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
	GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	GW::AUDIO::GAudio audioEngine; // can create music & sound effects
	GW::AUDIO::GMusic currentSong;
	// third-party gameplay & utility libraries
	std::shared_ptr<flecs::world> game; // ECS database for gameplay
	std::shared_ptr<GameConfig> gameConfig; // .ini file game settings
	// ECS Entities and Prefabs that need to be loaded
	EROD::PlayerData players;
	EROD::PelletData pellets;
	EROD::WallData walls;
	EROD::GhostData ghosts;
	EROD::PowerPelletData powerPellets;
	EROD::SpecialData specials;
	EROD::NodeData nodes;
	// specific ECS systems used to run the game
	EROD::PlayerLogic playerSystem;
	EROD::LevelLogic levelSystem;
	EROD::PhysicsLogic physicsSystem;
	EROD::PelletLogic pelletSystem;
	EROD::PowerPelletLogic wallSystem;
	EROD::TravelNodeLogic travelNodeSystem;
	EROD::GhostLogic ghostSystem;
	EROD::SpecialLogic specialSystem;
	//ESG::VulkanRendererLogic vkRenderingSystem;
	EROD::D3D11RendererLogic d3dRenderingSystem;
	// EventGenerator for Game Events
	GW::CORE::GEventGenerator eventPusher;
	GW::SYSTEM::GLog log;
	Level_Data levelData;

public:
	bool Init();
	bool Run();
	bool Shutdown();
private:
	bool InitWindow();
	bool InitInput();
	bool InitAudio();
	bool InitGraphics();
	bool InitEntities();
	bool InitSystems();
	bool GameLoop();
	void LoadLevelEntities();
};

#endif 