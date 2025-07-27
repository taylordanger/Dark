#include "InputManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace RPGEngine {
namespace Input {

InputManager::InputManager(GLFWwindow* window)
    : System("InputManager")
    , m_window(window)
    , m_nextCallbackId(1)
{
}

InputManager::~InputManager() {
    if (isInitialized()) {
        shutdown();
    }
}

bool InputManager::onInitialize() {
    if (!m_window) {
        std::cerr << "Invalid window handle provided to InputManager" << std::endl;
        return false;
    }
    
    // Create and initialize keyboard device
    m_keyboardDevice = std::make_shared<KeyboardDevice>(m_window);
    if (!m_keyboardDevice->initialize()) {
        std::cerr << "Failed to initialize keyboard device" << std::endl;
        return false;
    }
    
    // Create and initialize mouse device
    m_mouseDevice = std::make_shared<MouseDevice>(m_window);
    if (!m_mouseDevice->initialize()) {
        std::cerr << "Failed to initialize mouse device" << std::endl;
        m_keyboardDevice->shutdown();
        return false;
    }
    
    // Register devices
    m_devices.push_back(m_keyboardDevice);
    m_devices.push_back(m_mouseDevice);
    
    // Initialize gamepad devices (support up to 4 gamepads)
#ifndef NO_GRAPHICS
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_4; ++i) {
        auto gamepad = std::make_shared<GamepadDevice>(m_window, i);
        if (gamepad->initialize()) {
            m_gamepadDevices.push_back(gamepad);
            m_devices.push_back(gamepad);
        }
    }
#endif
    
    std::cout << "InputManager initialized with " << m_gamepadDevices.size() << " gamepad devices" << std::endl;
    return true;
}

void InputManager::onUpdate(float deltaTime) {
    // Update all devices
    for (auto& device : m_devices) {
        device->update();
    }
    
    // Update input actions
    updateActions();
}

void InputManager::onShutdown() {
    // Clear actions and bindings
    m_actions.clear();
    m_bindings.clear();
    m_actionCallbacks.clear();
    
    // Shutdown and clear devices
    for (auto& device : m_devices) {
        device->shutdown();
    }
    m_devices.clear();
    
    m_keyboardDevice.reset();
    m_mouseDevice.reset();
    m_gamepadDevices.clear();
    
    std::cout << "InputManager shutdown" << std::endl;
}

std::shared_ptr<IInputDevice> InputManager::getDevice(InputDeviceType type) const {
    for (const auto& device : m_devices) {
        if (device->getType() == type) {
            return device;
        }
    }
    
    return nullptr;
}

std::shared_ptr<IInputDevice> InputManager::getDeviceByName(const std::string& name) const {
    for (const auto& device : m_devices) {
        if (device->getName() == name) {
            return device;
        }
    }
    
    return nullptr;
}

std::shared_ptr<GamepadDevice> InputManager::getGamepadDevice(int gamepadId) const {
    for (const auto& gamepad : m_gamepadDevices) {
        if (gamepad->getGamepadId() == gamepadId) {
            return gamepad;
        }
    }
    
    return nullptr;
}

size_t InputManager::getConnectedGamepadCount() const {
    size_t count = 0;
    for (const auto& gamepad : m_gamepadDevices) {
        if (gamepad->isConnected()) {
            count++;
        }
    }
    
    return count;
}

bool InputManager::registerDevice(std::shared_ptr<IInputDevice> device) {
    if (!device) {
        return false;
    }
    
    // Check if device already exists
    for (const auto& existingDevice : m_devices) {
        if (existingDevice->getName() == device->getName()) {
            std::cerr << "Device '" << device->getName() << "' already registered" << std::endl;
            return false;
        }
    }
    
    // Initialize device if not already initialized
    if (!device->isInitialized() && !device->initialize()) {
        std::cerr << "Failed to initialize device '" << device->getName() << "'" << std::endl;
        return false;
    }
    
    // Add device
    m_devices.push_back(device);
    
    // Add to gamepad devices if it's a gamepad
    if (device->getType() == InputDeviceType::Gamepad) {
        auto gamepad = std::dynamic_pointer_cast<GamepadDevice>(device);
        if (gamepad) {
            m_gamepadDevices.push_back(gamepad);
        }
    }
    
    std::cout << "Registered device '" << device->getName() << "'" << std::endl;
    return true;
}

bool InputManager::unregisterDevice(std::shared_ptr<IInputDevice> device) {
    if (!device) {
        return false;
    }
    
    // Find device
    auto it = std::find(m_devices.begin(), m_devices.end(), device);
    if (it == m_devices.end()) {
        return false;
    }
    
    // Shutdown device
    (*it)->shutdown();
    
    // Remove from gamepad devices if it's a gamepad
    if (device->getType() == InputDeviceType::Gamepad) {
        auto gamepad = std::dynamic_pointer_cast<GamepadDevice>(device);
        if (gamepad) {
            auto gamepadIt = std::find(m_gamepadDevices.begin(), m_gamepadDevices.end(), gamepad);
            if (gamepadIt != m_gamepadDevices.end()) {
                m_gamepadDevices.erase(gamepadIt);
            }
        }
    }
    
    // Remove device
    m_devices.erase(it);
    
    std::cout << "Unregistered device '" << device->getName() << "'" << std::endl;
    return true;
}

bool InputManager::unregisterDeviceByType(InputDeviceType type) {
    auto device = getDevice(type);
    if (!device) {
        return false;
    }
    
    return unregisterDevice(device);
}

bool InputManager::unregisterDeviceByName(const std::string& name) {
    auto device = getDeviceByName(name);
    if (!device) {
        return false;
    }
    
    return unregisterDevice(device);
}

bool InputManager::createAction(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    // Check if action already exists
    if (hasAction(name)) {
        return true; // Action already exists
    }
    
    // Create action
    m_actions.emplace(name, InputAction(name));
    
    // Create empty bindings list
    m_bindings[name] = std::vector<InputBinding>();
    
    return true;
}

bool InputManager::removeAction(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    // Check if action exists
    if (!hasAction(name)) {
        return false;
    }
    
    // Remove action
    m_actions.erase(name);
    
    // Remove bindings
    m_bindings.erase(name);
    
    // Remove callbacks for this action
    auto it = m_actionCallbacks.begin();
    while (it != m_actionCallbacks.end()) {
        if (it->second.actionName == name) {
            it = m_actionCallbacks.erase(it);
        } else {
            ++it;
        }
    }
    
    return true;
}

bool InputManager::hasAction(const std::string& name) const {
    return m_actions.find(name) != m_actions.end();
}

const InputAction* InputManager::getAction(const std::string& name) const {
    auto it = m_actions.find(name);
    if (it != m_actions.end()) {
        return &it->second;
    }
    
    return nullptr;
}

bool InputManager::isActionActive(const std::string& name) const {
    auto action = getAction(name);
    return action ? action->active : false;
}

bool InputManager::isActionJustActivated(const std::string& name) const {
    auto action = getAction(name);
    return action ? action->justActivated : false;
}

bool InputManager::isActionJustDeactivated(const std::string& name) const {
    auto action = getAction(name);
    return action ? action->justDeactivated : false;
}

float InputManager::getActionValue(const std::string& name) const {
    auto action = getAction(name);
    return action ? action->value : 0.0f;
}

bool InputManager::bindKeyToAction(const std::string& actionName, KeyCode key, float scale) {
    // Create action if it doesn't exist
    if (!hasAction(actionName)) {
        createAction(actionName);
    }
    
    // Add binding
    m_bindings[actionName].emplace_back(
        actionName,
        InputBindingType::KeyboardKey,
        static_cast<int>(key),
        scale
    );
    
    return true;
}

bool InputManager::bindMouseButtonToAction(const std::string& actionName, MouseButton button, float scale) {
    // Create action if it doesn't exist
    if (!hasAction(actionName)) {
        createAction(actionName);
    }
    
    // Add binding
    m_bindings[actionName].emplace_back(
        actionName,
        InputBindingType::MouseButton,
        static_cast<int>(button),
        scale
    );
    
    return true;
}

bool InputManager::bindMouseAxisToAction(const std::string& actionName, int axis, float scale) {
    // Create action if it doesn't exist
    if (!hasAction(actionName)) {
        createAction(actionName);
    }
    
    // Add binding
    m_bindings[actionName].emplace_back(
        actionName,
        InputBindingType::MouseAxis,
        axis,
        scale
    );
    
    return true;
}

bool InputManager::bindGamepadButtonToAction(const std::string& actionName, GamepadButton button, int gamepadId, float scale) {
    // Create action if it doesn't exist
    if (!hasAction(actionName)) {
        createAction(actionName);
    }
    
    // Add binding
    InputBinding binding(actionName, InputBindingType::GamepadButton, static_cast<int>(button), scale);
    
    // Store gamepad ID in the upper 16 bits of the code
    if (gamepadId >= 0) {
        binding.code |= (gamepadId << 16);
    }
    
    m_bindings[actionName].push_back(binding);
    
    return true;
}

bool InputManager::bindGamepadAxisToAction(const std::string& actionName, GamepadAxis axis, int gamepadId, float scale) {
    // Create action if it doesn't exist
    if (!hasAction(actionName)) {
        createAction(actionName);
    }
    
    // Add binding
    InputBinding binding(actionName, InputBindingType::GamepadAxis, static_cast<int>(axis), scale);
    
    // Store gamepad ID in the upper 16 bits of the code
    if (gamepadId >= 0) {
        binding.code |= (gamepadId << 16);
    }
    
    m_bindings[actionName].push_back(binding);
    
    return true;
}

bool InputManager::removeActionBindings(const std::string& actionName) {
    if (!hasAction(actionName)) {
        return false;
    }
    
    // Clear bindings
    m_bindings[actionName].clear();
    
    return true;
}

bool InputManager::removeBinding(const std::string& actionName, InputBindingType type, int code) {
    if (!hasAction(actionName)) {
        return false;
    }
    
    auto& bindings = m_bindings[actionName];
    
    // Find and remove binding
    auto it = std::remove_if(bindings.begin(), bindings.end(),
        [type, code](const InputBinding& binding) {
            return binding.type == type && binding.code == code;
        }
    );
    
    if (it == bindings.end()) {
        return false;
    }
    
    bindings.erase(it, bindings.end());
    return true;
}

bool InputManager::loadBindingsFromFile(const std::string& filepath) {
    try {
        // Open file
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open input bindings file: " << filepath << std::endl;
            return false;
        }
        
        // Parse JSON
        json j;
        file >> j;
        
        // Clear existing bindings
        m_bindings.clear();
        
        // Load actions and bindings
        for (const auto& actionJson : j["actions"]) {
            std::string actionName = actionJson["name"];
            
            // Create action
            createAction(actionName);
            
            // Load bindings
            for (const auto& bindingJson : actionJson["bindings"]) {
                std::string typeStr = bindingJson["type"];
                int code = bindingJson["code"];
                float scale = bindingJson["scale"];
                
                InputBindingType type;
                if (typeStr == "keyboard") {
                    type = InputBindingType::KeyboardKey;
                } else if (typeStr == "mouseButton") {
                    type = InputBindingType::MouseButton;
                } else if (typeStr == "mouseAxis") {
                    type = InputBindingType::MouseAxis;
                } else if (typeStr == "gamepadButton") {
                    type = InputBindingType::GamepadButton;
                } else if (typeStr == "gamepadAxis") {
                    type = InputBindingType::GamepadAxis;
                } else {
                    type = InputBindingType::Custom;
                }
                
                // Add binding
                m_bindings[actionName].emplace_back(actionName, type, code, scale);
            }
        }
        
        std::cout << "Loaded input bindings from " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading input bindings: " << e.what() << std::endl;
        return false;
    }
}

bool InputManager::saveBindingsToFile(const std::string& filepath) {
    try {
        // Create JSON structure
        json j;
        j["actions"] = json::array();
        
        // Add actions and bindings
        for (const auto& pair : m_bindings) {
            const std::string& actionName = pair.first;
            const std::vector<InputBinding>& bindings = pair.second;
            
            json actionJson;
            actionJson["name"] = actionName;
            actionJson["bindings"] = json::array();
            
            // Add bindings
            for (const auto& binding : bindings) {
                json bindingJson;
                
                // Convert binding type to string
                std::string typeStr;
                switch (binding.type) {
                    case InputBindingType::KeyboardKey:
                        typeStr = "keyboard";
                        break;
                    case InputBindingType::MouseButton:
                        typeStr = "mouseButton";
                        break;
                    case InputBindingType::MouseAxis:
                        typeStr = "mouseAxis";
                        break;
                    case InputBindingType::GamepadButton:
                        typeStr = "gamepadButton";
                        break;
                    case InputBindingType::GamepadAxis:
                        typeStr = "gamepadAxis";
                        break;
                    case InputBindingType::Custom:
                        typeStr = "custom";
                        break;
                }
                
                bindingJson["type"] = typeStr;
                bindingJson["code"] = binding.code;
                bindingJson["scale"] = binding.scale;
                
                actionJson["bindings"].push_back(bindingJson);
            }
            
            j["actions"].push_back(actionJson);
        }
        
        // Write to file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            return false;
        }
        
        file << j.dump(4); // Pretty print with 4-space indentation
        
        std::cout << "Saved input bindings to " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving input bindings: " << e.what() << std::endl;
        return false;
    }
}

int InputManager::registerActionCallback(const std::string& actionName, 
                                       const std::function<void(const InputAction&)>& callback) {
    if (!hasAction(actionName) || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_actionCallbacks[callbackId] = {actionName, callback};
    return callbackId;
}

bool InputManager::unregisterActionCallback(int callbackId) {
    if (callbackId < 1) {
        return false;
    }
    
    auto it = m_actionCallbacks.find(callbackId);
    if (it != m_actionCallbacks.end()) {
        m_actionCallbacks.erase(it);
        return true;
    }
    
    return false;
}

void InputManager::updateActions() {
    // Store previous action states
    std::unordered_map<std::string, bool> previousActiveStates;
    for (const auto& pair : m_actions) {
        previousActiveStates[pair.first] = pair.second.active;
    }
    
    // Reset just activated/deactivated flags
    for (auto& pair : m_actions) {
        pair.second.justActivated = false;
        pair.second.justDeactivated = false;
    }
    
    // Update each action based on its bindings
    for (auto& pair : m_actions) {
        const std::string& actionName = pair.first;
        InputAction& action = pair.second;
        
        // Get bindings for this action
        auto bindingsIt = m_bindings.find(actionName);
        if (bindingsIt != m_bindings.end()) {
            updateAction(action, bindingsIt->second);
        } else {
            // No bindings, reset action
            action.active = false;
            action.value = 0.0f;
        }
        
        // Update just activated/deactivated flags
        bool previouslyActive = previousActiveStates[actionName];
        if (action.active && !previouslyActive) {
            action.justActivated = true;
        } else if (!action.active && previouslyActive) {
            action.justDeactivated = true;
        }
    }
    
    // Notify callbacks
    for (const auto& pair : m_actionCallbacks) {
        const std::string& actionName = pair.second.actionName;
        auto actionIt = m_actions.find(actionName);
        
        if (actionIt != m_actions.end()) {
            const InputAction& action = actionIt->second;
            
            // Only notify if the action state changed
            if (action.justActivated || action.justDeactivated) {
                pair.second.callback(action);
            }
        }
    }
}

void InputManager::updateAction(InputAction& action, const std::vector<InputBinding>& bindings) {
    // Reset action state
    action.active = false;
    action.value = 0.0f;
    
    // Process each binding
    for (const auto& binding : bindings) {
        float value = 0.0f;
        bool active = false;
        
        // Process binding based on type
        switch (binding.type) {
            case InputBindingType::KeyboardKey:
                if (m_keyboardDevice) {
                    KeyCode key = static_cast<KeyCode>(binding.code);
                    active = m_keyboardDevice->isKeyPressed(key);
                    value = active ? 1.0f : 0.0f;
                }
                break;
                
            case InputBindingType::MouseButton:
                if (m_mouseDevice) {
                    MouseButton button = static_cast<MouseButton>(binding.code);
                    active = m_mouseDevice->isButtonPressed(button);
                    value = active ? 1.0f : 0.0f;
                }
                break;
                
            case InputBindingType::MouseAxis:
                if (m_mouseDevice) {
                    double deltaX, deltaY;
                    m_mouseDevice->getDelta(deltaX, deltaY);
                    
                    if (binding.code == 0) { // X axis
                        value = static_cast<float>(deltaX);
                    } else if (binding.code == 1) { // Y axis
                        value = static_cast<float>(deltaY);
                    } else if (binding.code == 2) { // Scroll X
                        double scrollX, scrollY;
                        m_mouseDevice->getScrollOffset(scrollX, scrollY);
                        value = static_cast<float>(scrollX);
                    } else if (binding.code == 3) { // Scroll Y
                        double scrollX, scrollY;
                        m_mouseDevice->getScrollOffset(scrollX, scrollY);
                        value = static_cast<float>(scrollY);
                    }
                    
                    // Apply scale
                    value *= binding.scale;
                    
                    // Axis is active if value is non-zero
                    active = value != 0.0f;
                }
                break;
                
            case InputBindingType::GamepadButton:
                {
                    // Extract gamepad ID from the upper 16 bits of the code
                    int gamepadId = (binding.code >> 16) & 0xFFFF;
                    int buttonCode = binding.code & 0xFFFF;
                    GamepadButton button = static_cast<GamepadButton>(buttonCode);
                    
                    if (gamepadId >= 0) {
                        // Check specific gamepad
                        auto gamepad = getGamepadDevice(gamepadId);
                        if (gamepad && gamepad->isConnected()) {
                            active = gamepad->isButtonPressed(button);
                            value = active ? 1.0f : 0.0f;
                        }
                    } else {
                        // Check all gamepads
                        for (const auto& gamepad : m_gamepadDevices) {
                            if (gamepad->isConnected() && gamepad->isButtonPressed(button)) {
                                active = true;
                                value = 1.0f;
                                break;
                            }
                        }
                    }
                }
                break;
                
            case InputBindingType::GamepadAxis:
                {
                    // Extract gamepad ID from the upper 16 bits of the code
                    int gamepadId = (binding.code >> 16) & 0xFFFF;
                    int axisCode = binding.code & 0xFFFF;
                    GamepadAxis axis = static_cast<GamepadAxis>(axisCode);
                    
                    if (gamepadId >= 0) {
                        // Check specific gamepad
                        auto gamepad = getGamepadDevice(gamepadId);
                        if (gamepad && gamepad->isConnected()) {
                            value = gamepad->getAxisValue(axis) * binding.scale;
                            active = std::abs(value) > 0.1f;
                        }
                    } else {
                        // Check all gamepads
                        for (const auto& gamepad : m_gamepadDevices) {
                            if (gamepad->isConnected()) {
                                float axisValue = gamepad->getAxisValue(axis) * binding.scale;
                                if (std::abs(axisValue) > std::abs(value)) {
                                    value = axisValue;
                                    active = std::abs(value) > 0.1f;
                                }
                            }
                        }
                    }
                }
                break;
                
            default:
                break;
        }
        
        // Update action state
        if (active) {
            action.active = true;
        }
        
        // Update action value (use the binding with the highest absolute value)
        if (std::abs(value) > std::abs(action.value)) {
            action.value = value;
        }
    }
}

} // namespace Input
} // namespace RPGEngine