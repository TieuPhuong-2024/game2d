#include "InputManager.h"
#include <algorithm>
#include <xinput.h>

// Link DirectInput library
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

InputManager::InputManager() 
    : m_directInput(nullptr)
    , m_keyboard(nullptr)
    , m_hwnd(nullptr)
    , m_currentTime(0.0f)
    , m_inputEnabled(true)
    , m_initialized(false) {
    
    // Initialize gamepad array
    m_gamepads.fill(nullptr);
}

InputManager::~InputManager() {
    Shutdown();
}

bool InputManager::Initialize(HWND hwnd) {
    LOG_INFO("Initializing InputManager...");
    
    if (m_initialized) {
        LOG_WARNING("InputManager already initialized");
        return true;
    }
    
    m_hwnd = hwnd;
    
    // Initialize DirectInput
    if (!InitializeDirectInput()) {
        LOG_ERROR("Failed to initialize DirectInput");
        return false;
    }
    
    // Initialize keyboard
    if (!InitializeKeyboard()) {
        LOG_ERROR("Failed to initialize keyboard");
        return false;
    }
    
    // Initialize gamepads (non-critical, can fail)
    InitializeGamepads();
    
    // Load input configuration
    LoadConfiguration();
    
    m_initialized = true;
    LOG_INFO("InputManager initialized successfully");
    return true;
}

bool InputManager::InitializeDirectInput() {
    // Create DirectInput object
    HRESULT hr = DirectInput8Create(
        GetModuleHandle(nullptr),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void**)&m_directInput,
        nullptr
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create DirectInput8 object");
        return false;
    }
    
    return true;
}

bool InputManager::InitializeKeyboard() {
    if (!m_directInput) return false;
    
    // Create keyboard device
    HRESULT hr = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, nullptr);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create keyboard device");
        return false;
    }
    
    // Set keyboard data format
    hr = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set keyboard data format");
        return false;
    }
    
    // Set keyboard cooperative level
    hr = m_keyboard->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set keyboard cooperative level");
        return false;
    }
    
    // Acquire the keyboard
    m_keyboard->Acquire();
    
    LOG_INFO("Keyboard initialized successfully");
    return true;
}

bool InputManager::InitializeGamepads() {
    if (!m_directInput) return false;
    
    // Enumerate gamepads using DirectInput
    HRESULT hr = m_directInput->EnumDevices(
        DI8DEVCLASS_GAMECTRL,
        EnumGamepadsCallback,
        this,
        DIEDFL_ATTACHEDONLY
    );
    
    if (FAILED(hr)) {
        LOG_WARNING("Failed to enumerate gamepads");
        return false;
    }
    
    LOG_INFO("Gamepad enumeration completed");
    return true;
}

BOOL CALLBACK InputManager::EnumGamepadsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
    InputManager* inputManager = static_cast<InputManager*>(pContext);
    
    // Find first available slot
    for (int i = 0; i < 4; ++i) {
        if (inputManager->m_gamepads[i] == nullptr) {
            // Create gamepad device
            HRESULT hr = inputManager->m_directInput->CreateDevice(
                pdidInstance->guidInstance,
                &inputManager->m_gamepads[i],
                nullptr
            );
            
            if (SUCCEEDED(hr)) {
                // Set data format
                hr = inputManager->m_gamepads[i]->SetDataFormat(&c_dfDIJoystick2);
                if (SUCCEEDED(hr)) {
                    // Set cooperative level
                    hr = inputManager->m_gamepads[i]->SetCooperativeLevel(
                        inputManager->m_hwnd,
                        DISCL_FOREGROUND | DISCL_EXCLUSIVE
                    );
                    
                    if (SUCCEEDED(hr)) {
                        // Acquire the device
                        inputManager->m_gamepads[i]->Acquire();
                        inputManager->m_gamepadStates[i].connected = true;
                        
                        LOG_INFO("Gamepad " + std::to_string(i) + " initialized: " + 
                                std::string(pdidInstance->tszProductName, pdidInstance->tszProductName + wcslen(pdidInstance->tszProductName)));
                    }
                }
                
                if (FAILED(hr)) {
                    inputManager->m_gamepads[i]->Release();
                    inputManager->m_gamepads[i] = nullptr;
                }
            }
            break;
        }
    }
    
    return DIENUM_CONTINUE;
}

void InputManager::Update(float deltaTime) {
    if (!m_initialized || !m_inputEnabled) return;
    
    m_currentTime += deltaTime;
    
    // Update keyboard state
    UpdateKeyboard();
    
    // Update gamepad states
    UpdateGamepads();
    
    // Update input buffer
    UpdateInputBuffer(deltaTime);
}

void InputManager::UpdateKeyboard() {
    if (!m_keyboard) return;
    
    // Store previous state
    m_keyboardState.previousKeys = m_keyboardState.keys;
    
    // Get current keyboard state
    BYTE keyState[256];
    HRESULT hr = m_keyboard->GetDeviceState(sizeof(keyState), keyState);
    
    if (FAILED(hr)) {
        // Try to reacquire if device was lost
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            m_keyboard->Acquire();
        }
        return;
    }
    
    // Update key states and check for action mappings
    for (int i = 0; i < 256; ++i) {
        bool wasPressed = m_keyboardState.keys[i];
        m_keyboardState.keys[i] = (keyState[i] & 0x80) != 0;
        
        // Check if this key maps to any action and add to buffer
        if (m_keyboardState.keys[i] != wasPressed) {
            for (int actionIndex = 0; actionIndex < static_cast<int>(InputAction::COUNT); ++actionIndex) {
                InputAction action = static_cast<InputAction>(actionIndex);
                if (m_config.GetKeyBinding(action) == i) {
                    AddToInputBuffer(action, m_keyboardState.keys[i]);
                }
            }
        }
    }
}

void InputManager::UpdateGamepads() {
    for (int i = 0; i < 4; ++i) {
        if (!m_gamepads[i]) {
            // Try XInput as fallback for Xbox controllers
            XINPUT_STATE xinputState;
            DWORD result = XInputGetState(i, &xinputState);
            
            if (result == ERROR_SUCCESS) {
                // Store previous state
                m_gamepadStates[i].previousButtons = m_gamepadStates[i].buttons;
                
                // Update connection status
                m_gamepadStates[i].connected = true;
                
                // Update button states
                WORD buttons = xinputState.Gamepad.wButtons;
                m_gamepadStates[i].buttons[0] = (buttons & XINPUT_GAMEPAD_A) != 0;
                m_gamepadStates[i].buttons[1] = (buttons & XINPUT_GAMEPAD_B) != 0;
                m_gamepadStates[i].buttons[2] = (buttons & XINPUT_GAMEPAD_X) != 0;
                m_gamepadStates[i].buttons[3] = (buttons & XINPUT_GAMEPAD_Y) != 0;
                m_gamepadStates[i].buttons[4] = (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
                m_gamepadStates[i].buttons[5] = (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
                m_gamepadStates[i].buttons[6] = (buttons & XINPUT_GAMEPAD_BACK) != 0;
                m_gamepadStates[i].buttons[7] = (buttons & XINPUT_GAMEPAD_START) != 0;
                m_gamepadStates[i].buttons[8] = (buttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
                m_gamepadStates[i].buttons[9] = (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
                m_gamepadStates[i].buttons[10] = (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0;
                m_gamepadStates[i].buttons[11] = (buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
                m_gamepadStates[i].buttons[12] = (buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
                m_gamepadStates[i].buttons[13] = (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
                
                // Update analog sticks (normalize to -1.0 to 1.0)
                m_gamepadStates[i].leftStickX = xinputState.Gamepad.sThumbLX / 32767.0f;
                m_gamepadStates[i].leftStickY = xinputState.Gamepad.sThumbLY / 32767.0f;
                m_gamepadStates[i].rightStickX = xinputState.Gamepad.sThumbRX / 32767.0f;
                m_gamepadStates[i].rightStickY = xinputState.Gamepad.sThumbRY / 32767.0f;
                
                // Update triggers (normalize to 0.0 to 1.0)
                m_gamepadStates[i].leftTrigger = xinputState.Gamepad.bLeftTrigger / 255.0f;
                m_gamepadStates[i].rightTrigger = xinputState.Gamepad.bRightTrigger / 255.0f;
                
                // Check for button state changes and add to buffer
                for (int buttonIndex = 0; buttonIndex < 14; ++buttonIndex) {
                    if (m_gamepadStates[i].buttons[buttonIndex] != m_gamepadStates[i].previousButtons[buttonIndex]) {
                        for (int actionIndex = 0; actionIndex < static_cast<int>(InputAction::COUNT); ++actionIndex) {
                            InputAction action = static_cast<InputAction>(actionIndex);
                            if (m_config.GetGamepadBinding(action) == buttonIndex) {
                                AddToInputBuffer(action, m_gamepadStates[i].buttons[buttonIndex]);
                            }
                        }
                    }
                }
            } else {
                m_gamepadStates[i].connected = false;
            }
            continue;
        }
        
        // DirectInput gamepad handling
        DIJOYSTATE2 joyState;
        HRESULT hr = m_gamepads[i]->GetDeviceState(sizeof(joyState), &joyState);
        
        if (FAILED(hr)) {
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
                m_gamepads[i]->Acquire();
            }
            m_gamepadStates[i].connected = false;
            continue;
        }
        
        // Store previous state
        m_gamepadStates[i].previousButtons = m_gamepadStates[i].buttons;
        
        // Update connection status
        m_gamepadStates[i].connected = true;
        
        // Update button states (first 14 buttons)
        for (int j = 0; j < 14 && j < 128; ++j) {
            bool wasPressed = m_gamepadStates[i].buttons[j];
            m_gamepadStates[i].buttons[j] = (joyState.rgbButtons[j] & 0x80) != 0;
            
            // Add to input buffer if state changed
            if (m_gamepadStates[i].buttons[j] != wasPressed) {
                for (int actionIndex = 0; actionIndex < static_cast<int>(InputAction::COUNT); ++actionIndex) {
                    InputAction action = static_cast<InputAction>(actionIndex);
                    if (m_config.GetGamepadBinding(action) == j) {
                        AddToInputBuffer(action, m_gamepadStates[i].buttons[j]);
                    }
                }
            }
        }
        
        // Update analog sticks (normalize DirectInput values)
        m_gamepadStates[i].leftStickX = (joyState.lX - 32767) / 32767.0f;
        m_gamepadStates[i].leftStickY = (joyState.lY - 32767) / 32767.0f;
        m_gamepadStates[i].rightStickX = (joyState.lZ - 32767) / 32767.0f;
        m_gamepadStates[i].rightStickY = (joyState.lRz - 32767) / 32767.0f;
    }
}

void InputManager::UpdateInputBuffer(float deltaTime) {
    // Remove old entries from input buffer
    m_inputBuffer.erase(
        std::remove_if(m_inputBuffer.begin(), m_inputBuffer.end(),
            [this](const InputBufferEntry& entry) {
                return (m_currentTime - entry.timestamp) > 0.5f; // Keep entries for 0.5 seconds
            }),
        m_inputBuffer.end()
    );
}

void InputManager::ProcessWindowMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    // Handle window messages that might affect input
    switch (message) {
        case WM_ACTIVATE:
            // Re-acquire devices when window gains focus
            if (LOWORD(wParam) != WA_INACTIVE) {
                if (m_keyboard) m_keyboard->Acquire();
                for (auto& gamepad : m_gamepads) {
                    if (gamepad) gamepad->Acquire();
                }
            }
            break;
            
        case WM_KILLFOCUS:
            // Clear input states when losing focus
            m_keyboardState.keys.fill(false);
            for (auto& gamepadState : m_gamepadStates) {
                gamepadState.buttons.fill(false);
                gamepadState.leftStickX = gamepadState.leftStickY = 0.0f;
                gamepadState.rightStickX = gamepadState.rightStickY = 0.0f;
                gamepadState.leftTrigger = gamepadState.rightTrigger = 0.0f;
            }
            break;
    }
}

const GamepadState& InputManager::GetGamepadState(int controllerIndex) const {
    static GamepadState emptyState;
    if (controllerIndex >= 0 && controllerIndex < 4) {
        return m_gamepadStates[controllerIndex];
    }
    return emptyState;
}

bool InputManager::IsActionDown(InputAction action) const {
    if (!m_inputEnabled) return false;
    
    int actionIndex = static_cast<int>(action);
    if (actionIndex < 0 || actionIndex >= static_cast<int>(InputAction::COUNT)) return false;
    
    // Check keyboard mapping
    int keyCode = m_config.GetKeyBinding(action);
    if (keyCode >= 0 && keyCode < 256 && m_keyboardState.IsKeyDown(keyCode)) {
        return true;
    }
    
    // Check gamepad mappings
    int buttonIndex = m_config.GetGamepadBinding(action);
    if (buttonIndex >= 0) {
        for (const auto& gamepadState : m_gamepadStates) {
            if (gamepadState.IsButtonDown(buttonIndex)) {
                return true;
            }
        }
    }
    
    return false;
}

bool InputManager::IsActionPressed(InputAction action) const {
    if (!m_inputEnabled) return false;
    
    int actionIndex = static_cast<int>(action);
    if (actionIndex < 0 || actionIndex >= static_cast<int>(InputAction::COUNT)) return false;
    
    // Check keyboard mapping
    int keyCode = m_config.GetKeyBinding(action);
    if (keyCode >= 0 && keyCode < 256 && m_keyboardState.IsKeyPressed(keyCode)) {
        return true;
    }
    
    // Check gamepad mappings
    int buttonIndex = m_config.GetGamepadBinding(action);
    if (buttonIndex >= 0) {
        for (const auto& gamepadState : m_gamepadStates) {
            if (gamepadState.IsButtonPressed(buttonIndex)) {
                return true;
            }
        }
    }
    
    return false;
}

bool InputManager::IsActionReleased(InputAction action) const {
    if (!m_inputEnabled) return false;
    
    int actionIndex = static_cast<int>(action);
    if (actionIndex < 0 || actionIndex >= static_cast<int>(InputAction::COUNT)) return false;
    
    // Check keyboard mapping
    int keyCode = m_config.GetKeyBinding(action);
    if (keyCode >= 0 && keyCode < 256 && m_keyboardState.IsKeyReleased(keyCode)) {
        return true;
    }
    
    // Check gamepad mappings
    int buttonIndex = m_config.GetGamepadBinding(action);
    if (buttonIndex >= 0) {
        for (const auto& gamepadState : m_gamepadStates) {
            if (gamepadState.IsButtonReleased(buttonIndex)) {
                return true;
            }
        }
    }
    
    return false;
}

bool InputManager::GetBufferedAction(InputAction action, float maxAge) const {
    for (const auto& entry : m_inputBuffer) {
        if (entry.action == action && entry.pressed && 
            (m_currentTime - entry.timestamp) <= maxAge) {
            return true;
        }
    }
    return false;
}

void InputManager::ClearInputBuffer() {
    m_inputBuffer.clear();
}

bool InputManager::IsGamepadConnected(int controllerIndex) const {
    if (controllerIndex >= 0 && controllerIndex < 4) {
        return m_gamepadStates[controllerIndex].connected;
    }
    return false;
}

int InputManager::GetConnectedGamepadCount() const {
    int count = 0;
    for (const auto& gamepadState : m_gamepadStates) {
        if (gamepadState.connected) count++;
    }
    return count;
}

bool InputManager::LoadConfiguration(const std::string& filename) {
    return m_config.LoadFromFile(filename);
}

bool InputManager::SaveConfiguration(const std::string& filename) const {
    return m_config.SaveToFile(filename);
}

void InputManager::ResetToDefaults() {
    m_config.ResetToDefaults();
}

void InputManager::AddToInputBuffer(InputAction action, bool pressed) {
    m_inputBuffer.emplace_back(action, pressed, m_currentTime);
}

void InputManager::Shutdown() {
    if (!m_initialized) return;
    
    LOG_INFO("Shutting down InputManager...");
    
    // Release gamepads
    for (auto& gamepad : m_gamepads) {
        if (gamepad) {
            gamepad->Unacquire();
            gamepad->Release();
            gamepad = nullptr;
        }
    }
    
    // Release keyboard
    if (m_keyboard) {
        m_keyboard->Unacquire();
        m_keyboard->Release();
        m_keyboard = nullptr;
    }
    
    // Release DirectInput
    if (m_directInput) {
        m_directInput->Release();
        m_directInput = nullptr;
    }
    
    // Clear states
    for (auto& gamepadState : m_gamepadStates) {
        gamepadState.connected = false;
    }
    
    m_inputBuffer.clear();
    m_initialized = false;
    
    LOG_INFO("InputManager shutdown complete");
}