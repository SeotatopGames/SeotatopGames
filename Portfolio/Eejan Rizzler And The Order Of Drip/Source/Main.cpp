// handles everything
#include "Application.h"
// program entry point

// Enables memory leak detection
#define _CRTDBG_MAP_ALLOC

int main()
{
	// Memory leak detection code
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// This function call will set a breakpoint at the location of a leaked block
	// Set the parameter to the identifier for a leaked block
	_CrtSetBreakAlloc(-1);

	Application exampleSpaceGame;
	if (exampleSpaceGame.Init()) {
		if (exampleSpaceGame.Run()) {
			return exampleSpaceGame.Shutdown() ? 0 : 1;
		}
	}
	return 1;
}