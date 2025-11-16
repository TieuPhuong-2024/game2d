#pragma once
#include <windows.h>
#include <dinput.h>
#include <array>
#include <memory>
#include "Logger.h"
#include "InputConfig.h"
#include <vector>

// Input action enumeration for game-specific actions
enum class InputAction {
    MOVE_LEFT,
    MOVE_RIGHT,
    JUMP,
    SHOOT,
    DASH,
    WEAPON_SWITCH_NEXT,
    WEAPON_SWITCH_PREV,
    PAUSE,
    COUNT // Keep this last for array sizing
};

// Keyboard state structure
struct KeyboardState {
    std::array<bool, 256> keys;
    std::array<bool, 256> previousKeys;
    
    KeyboardState() {
        keys.fill(false);
        previousKeys.fill(false);
    }
    
    bool IsKeyDown(int keyCode) const {
        return keys[keyCode];
    }
    
    bool IsKeyPressed(int keyCode) const {
        return keys[keyCode] && !previousKeys[keyCode];
    }
    
    bool IsKeyReleased(int keyCode) const {
        return !keys[keyCode] && previousKeys[keyCode];
    }
};

// Gamepad state structure
struct GamepadState {
    bool connected;
    float leftStickX;
    float leftStickY;
    float rightStickX;
    float rightStickY;
    float leftTrigger;
    float rightTrigger;
    std::array<bool, 14> buttons; // Standard gamepad buttons
    std::array<bool, 14> previousButtons;
    
    GamepadState() {
        connected = false;
        leftStickX = leftStickY = 0.0f;
        rightStickX = rightStickY = 0.0f;
        leftTrigger = rightTrigger = 0.0f;
        buttons.fill(false);
        previousButtons.fill(false);
    }
    
    bool IsButtonDown(int buttonIndex) const {
        return connected && buttons[buttonIndex];
    }
    
    bool IsButtonPressed(int buttonIndex) const {
        return connected && buttons[buttonIndex] && !previousButtons[buttonIndex];
    }
    
    bool IsButtonReleased(int buttonIndex) const {
        return connected && !buttons[buttonIndex] && previousButtons[buttonIndex];
    }
};

// Input buffer entry for precise timing
struct InputBufferEntry {
    InputAction action;
    bool pressed;
    float timestamp;
    
    InputBufferEntry(InputAction act, bool press, float time) 
        : action(act), pressed(press), timestamp(time) {}
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    // Initialization and cleanup
    bool Initialize(HWND hwnd);
    void Shutdown();
    
    // Update methods (called each frame)
    void Update(float deltaTime);
    void ProcessWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
    
    // Input state queries
    const KeyboardState& GetKeyboardState() const { return m_keyboardState; }
    const GamepadState& GetGamepadState(int controllerIndex = 0) const;
    
    // Action-based input queries
    bool IsActionDown(InputAction action) const;
    bool IsActionPressed(InputAction action) const;
    bool IsActionReleased(InputAction action) const;
    
    // Input buffering for precise timing
    bool GetBufferedAction(InputAction action, float maxAge = 0.1f) const;
    void ClearInputBuffer();
    
    // Gamepad detection
    bool IsGamepadConnected(int controllerIndex = 0) const;
    int GetConnectedGamepadCount() const;
    
    // Input focus management
    void SetInputEnabled(bool enabled) { m_inputEnabled = enabled; }
    bool IsInputEnabled() const { return m_inputEnabled; }
    
    // Configuration management
    InputConfig& GetConfig() { return m_config; }
    const InputConfig& GetConfig() const { return m_config; }
    bool LoadConfiguration(const std::string& filename = "input_config.ini");
    bool SaveConfiguration(const std::string& filename = "input_config.ini") const;
    void ResetToDefaults();

private:
    // DirectInput objects
    LPDIRECTINPUT8 m_directInput;
    LPDIRECTINPUTDEVICE8 m_keyboard;
    std::array<LPDIRECTINPUTDEVICE8, 4> m_gamepads; // Support up to 4 controllers
    
    // Window handle
    HWND m_hwnd;
    
    // Input states
    KeyboardState m_keyboardState;
    std::array<GamepadState, 4> m_gamepadStates;
    
    // Input buffer for precise timing
    std::vector<InputBufferEntry> m_inputBuffer;
    float m_currentTime;
    
    // Input management
    bool m_inputEnabled;
    bool m_initialized;
    
    // Configuration system
    InputConfig m_config;
    
    // Private methods
    bool InitializeDirectInput();
    bool InitializeKeyboard();
    bool InitializeGamepads();
    void UpdateKeyboard();
    void UpdateGamepads();
    void UpdateInputBuffer(float deltaTime);
    
    // DirectInput callback for gamepad enumeration
    static BOOL CALLBACK EnumGamepadsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
    
    // Utility methods
    void AddToInputBuffer(InputAction action, bool pressed);
};