#pragma once
#include <string>
#include <map>
#include <fstream>
#include "../Utils/Logger.h"

// Forward declaration
enum class InputAction;

// Key binding configuration structure
struct KeyBinding {
    int keyCode;        // DirectInput key code (-1 if unbound)
    int gamepadButton;  // Gamepad button index (-1 if unbound)
    
    KeyBinding() : keyCode(-1), gamepadButton(-1) {}
    KeyBinding(int key, int button) : keyCode(key), gamepadButton(button) {}
};

// Input configuration manager
class InputConfig {
public:
    InputConfig();
    ~InputConfig();
    
    // Configuration file management
    bool LoadFromFile(const std::string& filename = "input_config.ini");
    bool SaveToFile(const std::string& filename = "input_config.ini") const;
    
    // Key binding management
    void SetKeyBinding(InputAction action, int keyCode);
    void SetGamepadBinding(InputAction action, int gamepadButton);
    void SetBinding(InputAction action, int keyCode, int gamepadButton);
    
    KeyBinding GetBinding(InputAction action) const;
    int GetKeyBinding(InputAction action) const;
    int GetGamepadBinding(InputAction action) const;
    
    // Reset to defaults
    void ResetToDefaults();
    
    // Validation
    bool IsValidKeyCode(int keyCode) const;
    bool IsValidGamepadButton(int buttonIndex) const;
    
    // Conflict detection
    bool HasKeyConflict(InputAction action, int keyCode) const;
    bool HasGamepadConflict(InputAction action, int gamepadButton) const;
    InputAction FindKeyConflict(int keyCode) const;
    InputAction FindGamepadConflict(int gamepadButton) const;
    
    // String conversion for UI
    std::string GetKeyName(int keyCode) const;
    std::string GetGamepadButtonName(int buttonIndex) const;
    std::string GetActionName(InputAction action) const;
    
private:
    // Key bindings storage
    std::map<InputAction, KeyBinding> m_bindings;
    
    // Default configuration
    void SetupDefaultBindings();
    
    // File I/O helpers
    std::string ActionToString(InputAction action) const;
    InputAction StringToAction(const std::string& actionStr) const;
    
    // Key name mappings
    void InitializeKeyNames();
    std::map<int, std::string> m_keyNames;
    std::map<int, std::string> m_gamepadButtonNames;
};