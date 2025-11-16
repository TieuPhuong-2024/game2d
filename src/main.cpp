#include <windows.h>
#include "GameEngine.h"
#include "Logger.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize logger first
    Logger::GetInstance().Initialize("MegaManX3.log");
    
    LOG_INFO("=== Mega Man X3 Game Starting ===");
    
    try {
        // Create and initialize the game engine
        GameEngine engine;
        
        if (!engine.Initialize(hInstance, 800, 600)) {
            LOG_FATAL("Failed to initialize game engine");
            return -1;
        }
        
        LOG_INFO("Game engine initialized successfully, starting main loop");
        
        // Run the main game loop
        engine.Run();
        
        LOG_INFO("Game loop ended, shutting down");
        
        // Cleanup is handled by GameEngine destructor
        
    } catch (const std::exception& e) {
        LOG_FATAL("Unhandled exception: " + std::string(e.what()));
        return -1;
    } catch (...) {
        LOG_FATAL("Unknown exception occurred");
        return -1;
    }
    
    LOG_INFO("=== Mega Man X3 Game Ended ===");
    Logger::GetInstance().Shutdown();
    
    return 0;
}