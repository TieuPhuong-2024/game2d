#include "InputConfig.h"
#include "InputManager.h"
#include <dinput.h>
#include <sstream>
#include <algorithm>

InputConfig::InputConfig() {
    InitializeKeyNames();
    SetupDefaultBindings();
}

InputConfig::~InputConfig() {
}

void InputConfig::InitializeKeyNames() {
    // Initialize DirectInput key names
    m_keyNames[DIK_A] = "A";
    m_keyNames[DIK_B] = "B";
    m_keyNames[DIK_C] = "C";
    m_keyNames[DIK_D] = "D";
    m_keyNames[DIK_E] = "E";
    m_keyNames[DIK_F] = "F";
    m_keyNames[DIK_G] = "G";
    m_keyNames[DIK_H] = "H";
    m_keyNames[DIK_I] = "I";
    m_keyNames[DIK_J] = "J";
    m_keyNames[DIK_K] = "K";
    m_keyNames[DIK_L] = "L";
    m_keyNames[DIK_M] = "M";
    m_keyNames[DIK_N] = "N";
    m_keyNames[DIK_O] = "O";
    m_keyNames[DIK_P] = "P";
    m_keyNames[DIK_Q] = "Q";
    m_keyNames[DIK_R] = "R";
    m_keyNames[DIK_S] = "S";
    m_keyNames[DIK_T] = "T";
    m_keyNames[DIK_U] = "U";
    m_keyNames[DIK_V] = "V";
    m_keyNames[DIK_W] = "W";
    m_keyNames[DIK_X] = "X";
    m_keyNames[DIK_Y] = "Y";
    m_keyNames[DIK_Z] = "Z";
    
    m_keyNames[DIK_1] = "1";
    m_keyNames[DIK_2] = "2";
    m_keyNames[DIK_3] = "3";
    m_keyNames[DIK_4] = "4";
    m_keyNames[DIK_5] = "5";
    m_keyNames[DIK_6] = "6";
    m_keyNames[DIK_7] = "7";
    m_keyNames[DIK_8] = "8";
    m_keyNames[DIK_9] = "9";
    m_keyNames[DIK_0] = "0";
    
    m_keyNames[DIK_SPACE] = "Space";
    m_keyNames[DIK_RETURN] = "Enter";
    m_keyNames[DIK_ESCAPE] = "Escape";
    m_keyNames[DIK_TAB] = "Tab";
    m_keyNames[DIK_LSHIFT] = "Left Shift";
    m_keyNames[DIK_RSHIFT] = "Right Shift";
    m_keyNames[DIK_LCONTROL] = "Left Ctrl";
    m_keyNames[DIK_RCONTROL] = "Right Ctrl";
    m_keyNames[DIK_LALT] = "Left Alt";
    m_keyNames[DIK_RALT] = "Right Alt";
    
    m_keyNames[DIK_UP] = "Up Arrow";
    m_keyNames[DIK_DOWN] = "Down Arrow";
    m_keyNames[DIK_LEFT] = "Left Arrow";
    m_keyNames[DIK_RIGHT] = "Right Arrow";
    
    m_keyNames[DIK_F1] = "F1";
    m_keyNames[DIK_F2] = "F2";
    m_keyNames[DIK_F3] = "F3";
    m_keyNames[DIK_F4] = "F4";
    m_keyNames[DIK_F5] = "F5";
    m_keyNames[DIK_F6] = "F6";
    m_keyNames[DIK_F7] = "F7";
    m_keyNames[DIK_F8] = "F8";
    m_keyNames[DIK_F9] = "F9";
    m_keyNames[DIK_F10] = "F10";
    m_keyNames[DIK_F11] = "F11";
    m_keyNames[DIK_F12] = "F12";
    
    // Initialize gamepad button names
    m_gamepadButtonNames[0] = "A Button";
    m_gamepadButtonNames[1] = "B Button";
    m_gamepadButtonNames[2] = "X Button";
    m_gamepadButtonNames[3] = "Y Button";
    m_gamepadButtonNames[4] = "Left Shoulder";
    m_gamepadButtonNames[5] = "Right Shoulder";
    m_gamepadButtonNames[6] = "Back Button";
    m_gamepadButtonNames[7] = "Start Button";
    m_gamepadButtonNames[8] = "Left Stick";
    m_gamepadButtonNames[9] = "Right Stick";
    m_gamepadButtonNames[10] = "D-Pad Up";
    m_gamepadButtonNames[11] = "D-Pad Down";
    m_gamepadButtonNames[12] = "D-Pad Left";
    m_gamepadButtonNames[13] = "D-Pad Right";
}

void InputConfig::SetupDefaultBindings() {
    // Default keyboard and gamepad bindings
    m_bindings[InputAction::MOVE_LEFT] = KeyBinding(DIK_A, 12);      // A key, D-pad left
    m_bindings[InputAction::MOVE_RIGHT] = KeyBinding(DIK_D, 13);     // D key, D-pad right
    m_bindings[InputAction::JUMP] = KeyBinding(DIK_SPACE, 0);        // Space, A button
    m_bindings[InputAction::SHOOT] = KeyBinding(DIK_J, 2);           // J key, X button
    m_bindings[InputAction::DASH] = KeyBinding(DIK_K, 1);            // K key, B button
    m_bindings[InputAction::WEAPON_SWITCH_NEXT] = KeyBinding(DIK_Q, 4);     // Q key, Left shoulder
    m_bindings[InputAction::WEAPON_SWITCH_PREV] = KeyBinding(DIK_E, 5);     // E key, Right shoulder
    m_bindings[InputAction::PAUSE] = KeyBinding(DIK_ESCAPE, 7);      // Escape, Start button
}

bool InputConfig::LoadFromFile(const std::string& filename) {
    LOG_INFO("Loading input configuration from: " + filename);
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARNING("Could not open input config file: " + filename + ", using defaults");
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Parse line format: ACTION=KEY_CODE,GAMEPAD_BUTTON
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            LOG_WARNING("Invalid config line " + std::to_string(lineNumber) + ": " + line);
            continue;
        }
        
        std::string actionStr = line.substr(0, equalPos);
        std::string bindingStr = line.substr(equalPos + 1);
        
        // Parse action
        InputAction action = StringToAction(actionStr);
        if (action == InputAction::COUNT) {
            LOG_WARNING("Unknown action in config line " + std::to_string(lineNumber) + ": " + actionStr);
            continue;
        }
        
        // Parse key and gamepad binding
        size_t commaPos = bindingStr.find(',');
        if (commaPos == std::string::npos) {
            LOG_WARNING("Invalid binding format in line " + std::to_string(lineNumber) + ": " + bindingStr);
            continue;
        }
        
        try {
            std::string keyStr = bindingStr.substr(0, commaPos);
            std::string gamepadStr = bindingStr.substr(commaPos + 1);
            
            int keyCode = (keyStr == "-1" || keyStr == "NONE") ? -1 : std::stoi(keyStr);
            int gamepadButton = (gamepadStr == "-1" || gamepadStr == "NONE") ? -1 : std::stoi(gamepadStr);
            
            // Validate bindings
            if (keyCode != -1 && !IsValidKeyCode(keyCode)) {
                LOG_WARNING("Invalid key code " + std::to_string(keyCode) + " for action " + actionStr);
                keyCode = -1;
            }
            
            if (gamepadButton != -1 && !IsValidGamepadButton(gamepadButton)) {
                LOG_WARNING("Invalid gamepad button " + std::to_string(gamepadButton) + " for action " + actionStr);
                gamepadButton = -1;
            }
            
            m_bindings[action] = KeyBinding(keyCode, gamepadButton);
            
        } catch (const std::exception& e) {
            LOG_WARNING("Error parsing config line " + std::to_string(lineNumber) + ": " + e.what());
        }
    }
    
    file.close();
    LOG_INFO("Input configuration loaded successfully");
    return true;
}

bool InputConfig::SaveToFile(const std::string& filename) const {
    LOG_INFO("Saving input configuration to: " + filename);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR("Could not create input config file: " + filename);
        return false;
    }
    
    // Write header
    file << "# Mega Man X3 Input Configuration\n";
    file << "# Format: ACTION=KEY_CODE,GAMEPAD_BUTTON\n";
    file << "# Use -1 or NONE for unbound keys/buttons\n";
    file << "\n";
    
    // Write all bindings
    for (const auto& pair : m_bindings) {
        InputAction action = pair.first;
        const KeyBinding& binding = pair.second;
        
        file << ActionToString(action) << "=";
        
        if (binding.keyCode == -1) {
            file << "NONE";
        } else {
            file << binding.keyCode;
        }
        
        file << ",";
        
        if (binding.gamepadButton == -1) {
            file << "NONE";
        } else {
            file << binding.gamepadButton;
        }
        
        file << "\n";
    }
    
    file.close();
    LOG_INFO("Input configuration saved successfully");
    return true;
}

void InputConfig::SetKeyBinding(InputAction action, int keyCode) {
    if (action >= InputAction::COUNT) return;
    
    auto it = m_bindings.find(action);
    if (it != m_bindings.end()) {
        it->second.keyCode = keyCode;
    } else {
        m_bindings[action] = KeyBinding(keyCode, -1);
    }
}

void InputConfig::SetGamepadBinding(InputAction action, int gamepadButton) {
    if (action >= InputAction::COUNT) return;
    
    auto it = m_bindings.find(action);
    if (it != m_bindings.end()) {
        it->second.gamepadButton = gamepadButton;
    } else {
        m_bindings[action] = KeyBinding(-1, gamepadButton);
    }
}

void InputConfig::SetBinding(InputAction action, int keyCode, int gamepadButton) {
    if (action >= InputAction::COUNT) return;
    m_bindings[action] = KeyBinding(keyCode, gamepadButton);
}

KeyBinding InputConfig::GetBinding(InputAction action) const {
    auto it = m_bindings.find(action);
    if (it != m_bindings.end()) {
        return it->second;
    }
    return KeyBinding(); // Returns unbound (-1, -1)
}

int InputConfig::GetKeyBinding(InputAction action) const {
    return GetBinding(action).keyCode;
}

int InputConfig::GetGamepadBinding(InputAction action) const {
    return GetBinding(action).gamepadButton;
}

void InputConfig::ResetToDefaults() {
    LOG_INFO("Resetting input configuration to defaults");
    m_bindings.clear();
    SetupDefaultBindings();
}

bool InputConfig::IsValidKeyCode(int keyCode) const {
    return keyCode >= 0 && keyCode < 256;
}

bool InputConfig::IsValidGamepadButton(int buttonIndex) const {
    return buttonIndex >= 0 && buttonIndex < 14;
}

bool InputConfig::HasKeyConflict(InputAction action, int keyCode) const {
    if (keyCode == -1) return false;
    
    for (const auto& pair : m_bindings) {
        if (pair.first != action && pair.second.keyCode == keyCode) {
            return true;
        }
    }
    return false;
}

bool InputConfig::HasGamepadConflict(InputAction action, int gamepadButton) const {
    if (gamepadButton == -1) return false;
    
    for (const auto& pair : m_bindings) {
        if (pair.first != action && pair.second.gamepadButton == gamepadButton) {
            return true;
        }
    }
    return false;
}

InputAction InputConfig::FindKeyConflict(int keyCode) const {
    if (keyCode == -1) return InputAction::COUNT;
    
    for (const auto& pair : m_bindings) {
        if (pair.second.keyCode == keyCode) {
            return pair.first;
        }
    }
    return InputAction::COUNT;
}

InputAction InputConfig::FindGamepadConflict(int gamepadButton) const {
    if (gamepadButton == -1) return InputAction::COUNT;
    
    for (const auto& pair : m_bindings) {
        if (pair.second.gamepadButton == gamepadButton) {
            return pair.first;
        }
    }
    return InputAction::COUNT;
}

std::string InputConfig::GetKeyName(int keyCode) const {
    auto it = m_keyNames.find(keyCode);
    if (it != m_keyNames.end()) {
        return it->second;
    }
    
    if (keyCode == -1) {
        return "Unbound";
    }
    
    return "Key " + std::to_string(keyCode);
}

std::string InputConfig::GetGamepadButtonName(int buttonIndex) const {
    auto it = m_gamepadButtonNames.find(buttonIndex);
    if (it != m_gamepadButtonNames.end()) {
        return it->second;
    }
    
    if (buttonIndex == -1) {
        return "Unbound";
    }
    
    return "Button " + std::to_string(buttonIndex);
}

std::string InputConfig::GetActionName(InputAction action) const {
    switch (action) {
        case InputAction::MOVE_LEFT: return "Move Left";
        case InputAction::MOVE_RIGHT: return "Move Right";
        case InputAction::JUMP: return "Jump";
        case InputAction::SHOOT: return "Shoot";
        case InputAction::DASH: return "Dash";
        case InputAction::WEAPON_SWITCH_NEXT: return "Next Weapon";
        case InputAction::WEAPON_SWITCH_PREV: return "Previous Weapon";
        case InputAction::PAUSE: return "Pause";
        default: return "Unknown";
    }
}

std::string InputConfig::ActionToString(InputAction action) const {
    switch (action) {
        case InputAction::MOVE_LEFT: return "MOVE_LEFT";
        case InputAction::MOVE_RIGHT: return "MOVE_RIGHT";
        case InputAction::JUMP: return "JUMP";
        case InputAction::SHOOT: return "SHOOT";
        case InputAction::DASH: return "DASH";
        case InputAction::WEAPON_SWITCH_NEXT: return "WEAPON_SWITCH_NEXT";
        case InputAction::WEAPON_SWITCH_PREV: return "WEAPON_SWITCH_PREV";
        case InputAction::PAUSE: return "PAUSE";
        default: return "UNKNOWN";
    }
}

InputAction InputConfig::StringToAction(const std::string& actionStr) const {
    std::string upperStr = actionStr;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    
    if (upperStr == "MOVE_LEFT") return InputAction::MOVE_LEFT;
    if (upperStr == "MOVE_RIGHT") return InputAction::MOVE_RIGHT;
    if (upperStr == "JUMP") return InputAction::JUMP;
    if (upperStr == "SHOOT") return InputAction::SHOOT;
    if (upperStr == "DASH") return InputAction::DASH;
    if (upperStr == "WEAPON_SWITCH_NEXT") return InputAction::WEAPON_SWITCH_NEXT;
    if (upperStr == "WEAPON_SWITCH_PREV") return InputAction::WEAPON_SWITCH_PREV;
    if (upperStr == "PAUSE") return InputAction::PAUSE;
    
    return InputAction::COUNT; // Invalid action
}