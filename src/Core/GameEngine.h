#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <chrono>
#include <vector>
#include "../Utils/Logger.h"
#include "../Graphics/D3DRenderer.h"
#include "../Input/InputManager.h"
#include "../Entities/Player.h"
#include "../Physics/CollisionSystem.h"

class GameEngine {
public:
    GameEngine();
    ~GameEngine();
    
    bool Initialize(HINSTANCE hInstance, int windowWidth = 800, int windowHeight = 600);
    void Run();
    void Shutdown();
    
    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
private:
    // Window management
    HWND m_hwnd;
    HINSTANCE m_hInstance;
    int m_windowWidth;
    int m_windowHeight;
    bool m_isRunning;
    
    // Rendering system
    D3DRenderer* m_renderer;
    
    // Input system
    InputManager* m_inputManager;
    
    // Game systems
    CollisionSystem* m_collisionSystem;
    Player* m_player;
    
    // Test platforms for collision
    struct Platform {
        Rect bounds;
        D3DCOLOR color;
    };
    std::vector<Platform> m_testPlatforms;
    
    // Timing for 60 FPS
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    const double m_targetFrameTime = 1.0 / 60.0; // 60 FPS = 16.67ms per frame
    
    // Core methods
    bool CreateWindows();
    bool InitializeRenderer();
    bool InitializeInput();
    bool InitializeGameSystems();
    void Update(float deltaTime);
    void Render();
    void HandleDeviceLost();
    
    // Utility methods
    void CalculateFrameStats();
    bool IsDeviceLost();
    void CheckPlayerCollisions();
};