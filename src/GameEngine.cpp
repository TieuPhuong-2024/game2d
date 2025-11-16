#include "GameEngine.h"
#include <thread>

GameEngine::GameEngine() 
    : m_hwnd(nullptr)
    , m_hInstance(nullptr)
    , m_windowWidth(800)
    , m_windowHeight(600)
    , m_isRunning(false)
    , m_renderer(nullptr)
    , m_inputManager(nullptr)
    , m_collisionSystem(nullptr)
    , m_player(nullptr) {
}

GameEngine::~GameEngine() {
    Shutdown();
}

bool GameEngine::Initialize(HINSTANCE hInstance, int windowWidth, int windowHeight) {
    LOG_INFO("Initializing GameEngine...");
    
    m_hInstance = hInstance;
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    
    // Create the window
    if (!CreateWindows()) {
        LOG_ERROR("Failed to create window");
        return false;
    }
    
    // Initialize renderer
    if (!InitializeRenderer()) {
        LOG_ERROR("Failed to initialize renderer");
        return false;
    }
    
    // Initialize input system
    if (!InitializeInput()) {
        LOG_ERROR("Failed to initialize input system");
        return false;
    }
    
    // Initialize game systems
    if (!InitializeGameSystems()) {
        LOG_ERROR("Failed to initialize game systems");
        return false;
    }
    
    // Initialize timing
    m_lastFrameTime = std::chrono::high_resolution_clock::now();
    
    LOG_INFO("GameEngine initialized successfully");
    return true;
}

bool GameEngine::CreateWindows() {
    LOG_INFO("Creating game window...");
    
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"MegaManX3WindowClass";
    
    if (!RegisterClassEx(&wc)) {
        LOG_ERROR("Failed to register window class");
        return false;
    }
    
    // Calculate window size including borders
    RECT windowRect = { 0, 0, m_windowWidth, m_windowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    
    int windowWidthWithBorders = windowRect.right - windowRect.left;
    int windowHeightWithBorders = windowRect.bottom - windowRect.top;
    
    // Create window
    m_hwnd = CreateWindowEx(
        0,
        L"MegaManX3WindowClass",
        L"Mega Man X3",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidthWithBorders, windowHeightWithBorders,
        nullptr, nullptr, m_hInstance, this
    );
    
    if (!m_hwnd) {
        LOG_ERROR("Failed to create window");
        return false;
    }
    
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    
    LOG_INFO("Window created successfully");
    return true;
}

bool GameEngine::InitializeRenderer() {
    LOG_INFO("Initializing renderer...");
    
    // Create the renderer
    m_renderer = new D3DRenderer();
    if (!m_renderer->Initialize(m_hwnd, m_windowWidth, m_windowHeight)) {
        LOG_ERROR("Failed to initialize D3DRenderer");
        delete m_renderer;
        m_renderer = nullptr;
        return false;
    }
    
    LOG_INFO("Renderer initialized successfully");
    return true;
}

bool GameEngine::InitializeInput() {
    LOG_INFO("Initializing input system...");
    
    // Create the input manager
    m_inputManager = new InputManager();
    if (!m_inputManager->Initialize(m_hwnd)) {
        LOG_ERROR("Failed to initialize InputManager");
        delete m_inputManager;
        m_inputManager = nullptr;
        return false;
    }
    
    LOG_INFO("Input system initialized successfully");
    return true;
}

bool GameEngine::InitializeGameSystems() {
    LOG_INFO("Initializing game systems...");
    
    // Create collision system
    m_collisionSystem = new CollisionSystem();
    
    // Create player
    m_player = new Player();
    m_player->SetInputManager(m_inputManager);
    
    // Initialize player above the ground platform
    Vector2 playerStartPos(100.0f, 100.0f);
    m_player->Initialize(playerStartPos);
    
    // Create test platforms
    // Ground platform
    Platform ground;
    ground.bounds = Rect(0, 500, 800, 100);  // Bottom of screen
    ground.color = D3DCOLOR_ARGB(255, 100, 200, 100);  // Green
    m_testPlatforms.push_back(ground);
    
    // Wall platform
    Platform wall;
    wall.bounds = Rect(600, 300, 50, 200);  // Right side wall
    wall.color = D3DCOLOR_ARGB(255, 200, 100, 100);  // Red
    m_testPlatforms.push_back(wall);
    
    // Another platform in the middle
    Platform midPlatform;
    midPlatform.bounds = Rect(300, 350, 200, 30);
    midPlatform.color = D3DCOLOR_ARGB(255, 150, 150, 200);  // Purple
    m_testPlatforms.push_back(midPlatform);
    
    LOG_INFO("Game systems initialized successfully");
    return true;
}

void GameEngine::Run() {
    LOG_INFO("Starting main game loop...");
    m_isRunning = true;
    
    MSG msg = {};
    
    while (m_isRunning) {
        // Process Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (!m_isRunning) break;
        
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTimeMs = std::chrono::duration<double>(currentTime - m_lastFrameTime).count();
        
        // Fixed timestep: only update if enough time has passed
        if (deltaTimeMs >= m_targetFrameTime) {
            // Check for device loss
            if (m_renderer && m_renderer->IsDeviceLost()) {
                HandleDeviceLost();
                continue;
            }
            
            // Update game logic
            Update(static_cast<float>(deltaTimeMs));
            
            // Render frame
            Render();
            
            m_lastFrameTime = currentTime;
        } else {
            // Sleep for a short time to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    LOG_INFO("Main game loop ended");
}

void GameEngine::Update(float deltaTime) {
    // Update input system
    if (m_inputManager) {
        m_inputManager->Update(deltaTime);
        
        // Check for exit input (ESC key or gamepad start button)
        if (m_inputManager->IsActionPressed(InputAction::PAUSE)) {
            m_isRunning = false;
        }
    }
    
    // Update player
    if (m_player) {
        m_player->Update(deltaTime);
    }
    
    // Update collision system
    if (m_collisionSystem) {
        m_collisionSystem->Update();
    }
    
    // FPS logging
    static int frameCount = 0;
    static float timeAccumulator = 0.0f;
    
    frameCount++;
    timeAccumulator += deltaTime;
    
    // Log FPS and player state every second
    if (timeAccumulator >= 1.0f) {
        LOG_INFO("FPS: " + std::to_string(frameCount) + " (Target: 60)");
        
        if (m_player) {
            Vector2 pos = m_player->GetPosition();
            Vector2 vel = m_player->GetPhysicsBody().GetVelocity();
            LOG_INFO("Player - Pos: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + 
                     ") Vel: (" + std::to_string(vel.x) + ", " + std::to_string(vel.y) + ")");
        }
        
        frameCount = 0;
        timeAccumulator = 0.0f;
    }
}

void GameEngine::Render() {
    if (!m_renderer) return;
    
    // Clear the screen
    m_renderer->Clear(Color(50, 50, 100)); // Dark blue background
    
    // Begin frame
    m_renderer->BeginFrame();
    
    // Draw test platforms
    for (const auto& platform : m_testPlatforms) {
        m_renderer->DrawDebugRect(platform.bounds, platform.color);
    }
    
    // Draw player as a colored square
    if (m_player) {
        Vector2 playerPos = m_player->GetPosition();
        CollisionBox playerBox = m_player->GetCollisionBox();
        
        // Draw player collision box
        float x = playerPos.x + playerBox.offset.x;
        float y = playerPos.y + playerBox.offset.y;
        
        D3DCOLOR playerColor = D3DCOLOR_ARGB(255, 255, 255, 0);  // Yellow
        m_renderer->DrawDebugRect(x, y, playerBox.width, playerBox.height, playerColor);
    }
    
    // End frame
    m_renderer->EndFrame();
}

bool GameEngine::IsDeviceLost() {
    if (!m_renderer) return true;
    return m_renderer->IsDeviceLost();
}

void GameEngine::HandleDeviceLost() {
    if (!m_renderer) return;
    
    LOG_INFO("Handling device loss...");
    if (m_renderer->ResetDevice()) {
        LOG_INFO("Device reset successfully");
    } else {
        LOG_ERROR("Failed to reset device");
    }
}

void GameEngine::Shutdown() {
    LOG_INFO("Shutting down GameEngine...");
    
    m_isRunning = false;
    
    // Cleanup game systems
    if (m_player) {
        delete m_player;
        m_player = nullptr;
    }
    
    if (m_collisionSystem) {
        delete m_collisionSystem;
        m_collisionSystem = nullptr;
    }
    
    // Save input configuration before shutdown
    if (m_inputManager) {
        m_inputManager->SaveConfiguration();
        m_inputManager->Shutdown();
        delete m_inputManager;
        m_inputManager = nullptr;
    }
    
    // Shutdown renderer
    if (m_renderer) {
        m_renderer->Shutdown();
        delete m_renderer;
        m_renderer = nullptr;
    }
    
    // Destroy window
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    
    LOG_INFO("GameEngine shutdown complete");
}

LRESULT CALLBACK GameEngine::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GameEngine* engine = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        // Store the GameEngine pointer in the window's user data
        CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        engine = reinterpret_cast<GameEngine*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(engine));
    } else {
        // Retrieve the GameEngine pointer from the window's user data
        engine = reinterpret_cast<GameEngine*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    // Forward input messages to InputManager
    if (engine && engine->m_inputManager) {
        engine->m_inputManager->ProcessWindowMessage(uMsg, wParam, lParam);
    }
    
    switch (uMsg) {
        case WM_CLOSE:
            if (engine) {
                engine->m_isRunning = false;
            }
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
            // Let InputManager handle all keyboard input now
            // Keep ESC as backup exit method
            if (wParam == VK_ESCAPE) {
                if (engine) {
                    engine->m_isRunning = false;
                }
                PostQuitMessage(0);
            }
            return 0;
            
        case WM_SIZE:
            // Handle window resizing if needed in future
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}