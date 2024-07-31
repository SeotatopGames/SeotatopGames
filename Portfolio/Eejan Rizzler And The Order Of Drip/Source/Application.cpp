#include "Application.h"
// open some Gateware namespaces for conveinence 
// NEVER do this in a header file!
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
using namespace EROD;

bool Application::Init()
{
	eventPusher.Create();

	// load all game settigns
	gameConfig = std::make_shared<GameConfig>();
	// create the ECS system
	game = std::make_shared<flecs::world>();

	enum LEVELS
	{
		ONE = 1,
		TWO,
		THREE,
		JOSH,
		VICTOR,
		JYMEER,
		NAJEE,
		COLYN,
		JOHN
	};

	std::string levelPath_1 = gameConfig->at("Level1").at("levelTextPath1").as<std::string>();
	std::string ModelPath_1 = gameConfig->at("Level1").at("levelModelPath1").as<std::string>();
	log.Create("Error.txt");
	log.EnableConsoleLogging(true);
	switch (LEVELS::ONE)
	{
	case 1: //1
	{
		if (levelData.LoadLevel(levelPath_1.c_str(), ModelPath_1.c_str(), log) == false)
			return false;
		break;
	}
	case 4: //Josh
	{
		std::string levelPath_4 = gameConfig->at("LevelJosh").at("levelTextPathJosh").as<std::string>();
		std::string ModelPath_4 = gameConfig->at("LevelJosh").at("levelModelPathJosh").as<std::string>();
		if (levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log) == false)
			return false;
		break;
	}
	case 5: //Victor
	{
		std::string levelPath_4 = gameConfig->at("LevelVictor").at("levelTextPathVictor").as<std::string>();
		std::string ModelPath_4 = gameConfig->at("LevelVictor").at("levelModelPathVictor").as<std::string>();
		if (levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log) == false)
			return false;
		break;
	}
	case 6: //Jymeer
	{
		std::string levelPath_4 = gameConfig->at("LevelJymeer").at("levelTextPathJymeer").as<std::string>();
		std::string ModelPath_4 = gameConfig->at("LevelJymeer").at("levelModelPathJymeer").as<std::string>();
		if (levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log) == false)
			return false;
		break;
	}
	case 7: //Najee
	{
		std::string levelPath_4 = gameConfig->at("LevelNajee").at("levelTextPathNajee").as<std::string>();
		std::string ModelPath_4 = gameConfig->at("LevelNajee").at("levelModelPathNajee").as<std::string>();
		if (levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log) == false)
			return false;
		break;
	}
	case 8: //Colyn
	{
		std::string levelPath_4 = gameConfig->at("LevelColyn").at("levelTextPathColyn").as<std::string>();
		std::string ModelPath_4 = gameConfig->at("LevelColyn").at("levelModelPathColyn").as<std::string>();
		if (levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log) == false)
			return false;
		break;
	}
	case 9: //John
	{
		std::string levelPath_4 = gameConfig->at("LevelJohn").at("levelTextPathJohn").as<std::string>();
		std::string ModelPath_4 = gameConfig->at("LevelJohn").at("levelModelPathJohn").as<std::string>();
		bool y = levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log);
		if (levelData.LoadLevel(levelPath_4.c_str(), ModelPath_4.c_str(), log) == false)
			return false;
		break;
	}
	default:
	{
		if (levelData.LoadLevel(levelPath_1.c_str(), ModelPath_1.c_str(), log) == false)
			return false;
		break;
	}
	}
	//levelData.GhostTestProgram(0);
	// init all other systems
	if (InitWindow() == false)
		return false;
	if (InitInput() == false)
		return false;
	if (InitAudio() == false)
		return false;
	if (InitGraphics() == false)
		return false;
	if (InitEntities() == false)
		return false;
	if (InitSystems() == false)
		return false;
	return true;
}

bool Application::Run()
{
	float clr[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	// grab vsync selection
	bool vsync = gameConfig->at("Window").at("vsync").as<bool>();
	// set background color from settings
	const char* channels[] = { "red", "green", "blue" };
	for (int i = 0; i < std::size(channels); ++i)
	{
		clr[i] =
			gameConfig->at("BackGroundColor").at(channels[i]).as<float>();
	}
	// create an event handler to see if the window was closed early
	bool winClosed = false;
	GW::CORE::GEventResponder winHandler;
	winHandler.Create([&winClosed](GW::GEvent e) {
		GW::SYSTEM::GWindow::Events ev;
		if (+e.Read(ev) && ev == GW::SYSTEM::GWindow::Events::DESTROY)
			winClosed = true;
		});
	window.Register(winHandler);

	while (+window.ProcessWindowEvents())
	{
		if (winClosed == true)
		{
			return true;
		}
		IDXGISwapChain* swap;
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;
		if (+d3d11.GetImmediateContext((void**)&con) &&
			+d3d11.GetRenderTargetView((void**)&view) &&
			+d3d11.GetDepthStencilView((void**)&depth) &&
			+d3d11.GetSwapchain((void**)&swap))
		{
			con->ClearRenderTargetView(view, clr);
			con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);
			GameLoop();
			swap->Present((int)vsync, 0);
			// release incremented COM reference counts
			swap->Release();
			view->Release();
			depth->Release();
			con->Release();
		}
	}
}
bool Application::Shutdown()
{
	// disconnect systems from global ECS
	if (levelSystem.Shutdown() == false)
		return false;
	if (d3dRenderingSystem.Shutdown() == false)
		return false;
	if (playerSystem.Shutdown() == false)
		return false;
	if (physicsSystem.Shutdown() == false)
		return false;
	if (ghostSystem.Shutdown() == false)
		return false;
	if (pelletSystem.Shutdown() == false)
		return false;
	if (wallSystem.Shutdown() == false)
		return false;
	if (specialSystem.Shutdown() == false)
		return false;

	return true;
}

bool Application::InitWindow()
{
	// grab settings
	int width = gameConfig->at("Window").at("width").as<int>();
	int height = gameConfig->at("Window").at("height").as<int>();
	int xstart = gameConfig->at("Window").at("xstart").as<int>();
	int ystart = gameConfig->at("Window").at("ystart").as<int>();
	std::string title = gameConfig->at("Window").at("title").as<std::string>();
	// open window
	if (+window.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDLOCKED) &&
		+window.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-immediateInput.Create(window))
		return false;
	if (-bufferedInput.Create(window))
		return false;
	return true;
}

bool Application::InitAudio()
{
	if (-audioEngine.Create())
		return false;
	return true;
}

bool Application::InitGraphics()
{

	if (+d3d11.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
	{
		return true;
	}
	return false;
}

bool Application::InitEntities()
{
	LoadLevelEntities();

	return true;
}

bool Application::InitSystems()
{
	// connect systems to global ECS
	if (playerSystem.Init(game, gameConfig, immediateInput, bufferedInput,
		gamePads, audioEngine, eventPusher, &isPaused, currentSong) == false)
		return false;
	if (travelNodeSystem.Init(game, gameConfig, immediateInput, levelData) == false)
		return false;
	if (pelletSystem.Init(game, gameConfig, eventPusher, d3dRenderingSystem,levelData,log, audioEngine,pellets, powerPellets, currentSong) == false)
		return false;
	if (wallSystem.Init(game, gameConfig, eventPusher, d3dRenderingSystem, levelData) == false)
		return false;
	if (levelSystem.Init(game, gameConfig, audioEngine, currentSong) == false)
		return false;
	if (ghostSystem.Init(game, gameConfig, d3dRenderingSystem, log, audioEngine, levelData, currentSong, &isPaused) == false)
		return false;
	if (d3dRenderingSystem.Init(game, gameConfig, d3d11, window,levelData, pelletNumber) == false)//pelletNumber) == false)
		return false;
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;
	if (specialSystem.Init(game, gameConfig, eventPusher, &isPaused) == false)
		return false;

	return true;
}
	
bool Application::GameLoop()
{
	// compute delta time and pass to the ECS system
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	// let the ECS system run
	return game->progress(static_cast<float>(elapsed));
}
void Application::LoadLevelEntities()
{
	for (auto& i : levelData.levelInstances)
	{
		auto model = levelData.levelModels[i.modelIndex];
		int switchCase = (int)levelData.levelEntityTypes[i.entityTypeIndex];
		switch (switchCase)
		{
		case 0: //player
		{
			players.Load(game, gameConfig, levelData, model, i, playerTimer);
			break;
		}
		case 1: //enemy 1
		{
			ghosts.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 2: //special
		{
			specials.Load(game, gameConfig, levelData, model, i);
			break;
		}
		case 3: //wall
		{
			walls.Load(game, gameConfig, levelData, model, i);
			break;
		}
		case 4: //pellets
		{
			pelletNumber = i.transformCount;
			pellets.Load(game, gameConfig, levelData, model, i);
			break;
		}
		case 5: //power pellets
		{
			powerPellets.Load(game, gameConfig, levelData, model, i);
			break;
		}
		case 6: //power pellet spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 7: //special spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 8: //wall enemy only
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 9: //enemy 1 spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 10: //enemy 2 spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 11: //enemy 3 spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 12: //enemy 4 spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 13: //player spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 14: //enemy 2
		{
			ghosts.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 15: //enemy 3
		{
			ghosts.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 16: //enemy 4
		{
			ghosts.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 17: //exit left
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 18: //exit right
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 19: //travel only
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		case 20: //pellet spawn
		{
			nodes.Load(game, gameConfig, levelData, model, i, switchCase);
			break;
		}
		}
	}
}