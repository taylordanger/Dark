#include "GamepadDevice.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

namespace RPGEngine {
namespace Input {

// Initialize static instance map
std::unordered_map<int, GamepadDevice*> GamepadDevice::s_instances;

GamepadDevice::GamepadDevice(GLFWwindow* window, int gamepadId)
    : m_window(window)
    , m_gamepadId(gamepadId)
    , m_name("")
    , m_connected(false)
    , m_deadzone(0.15f)
    , m_nextCallbackId(1)
    , m_initialized(false)
{
    // Initialize axis values
    m_axisValues.fill(0.0f);
    m_previousAxisValues.fill(0.0f);
}

GamepadDevice::~GamepadDevice() {
    if (m_initialized) {
        shutdown();
    }
}

bool GamepadDevice::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_window) {
        std::cerr << "Invalid window handle provided to GamepadDevice" << std::endl;
        return false;
    }
    
    // Register this instance for callbacks
    s_instances[m_gamepadId] = this;
    
    // Set GLFW joystick callback
    glfwSetJoystickCallback(joystickCallback);
    
    // Update connection state
    updateConnectionState();
    
    m_initialized = true;
    std::cout << "GamepadDevice " << m_gamepadId << " initialized" << std::endl;
    return true;
}

void GamepadDevice::update() {
    if (!m_initialized) {
        return;
    }
    
    // Store current button states as previous
    m_previousButtonStates = m_currentButtonStates;
    
    // Store current axis values as previous
    m_previousAxisValues = m_axisValues;
    
    // Update connection state
    updateConnectionState();
    
    // If not connected, nothing else to update
    if (!m_connected) {
        return;
    }
    
    // Update button states
    GLFWgamepadstate state;
    if (glfwGetGamepadState(m_gamepadId, &state)) {
        // Update buttons
        for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; ++i) {
            GamepadButton button = static_cast<GamepadButton>(i);
            GamepadButtonState buttonState = (state.buttons[i] == GLFW_PRESS) ? 
                                           GamepadButtonState::Pressed : 
                                           GamepadButtonState::Released;
            
            // Check if button state changed
            auto prevIt = m_previousButtonStates.find(button);
            bool previousPressed = (prevIt != m_previousButtonStates.end()) && 
                                  (prevIt->second == GamepadButtonState::Pressed);
            
            bool currentPressed = (buttonState == GamepadButtonState::Pressed);
            
            // Update button state
            m_currentButtonStates[button] = buttonState;
            
            // Notify callbacks if state changed
            if (currentPressed != previousPressed) {
                GamepadButtonEvent event(m_gamepadId, button, buttonState);
                
                for (const auto& pair : m_buttonCallbacks) {
                    pair.second(event);
                }
            }
        }
        
        // Update axes
        for (int i = 0; i < GLFW_GAMEPAD_AXIS_LAST + 1; ++i) {
            GamepadAxis axis = static_cast<GamepadAxis>(i);
            float rawValue = state.axes[i];
            
            // Apply deadzone
            float value = applyDeadzone(rawValue);
            
            // Store axis value
            m_axisValues[i] = value;
            
            // Check if axis value changed significantly
            float prevValue = m_previousAxisValues[i];
            if (std::abs(value - prevValue) > 0.01f) {
                GamepadAxisEvent event(m_gamepadId, axis, value);
                
                for (const auto& pair : m_axisCallbacks) {
                    pair.second(event);
                }
            }
        }
    }
}

void GamepadDevice::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Remove this instance from the callback map
    s_instances.erase(m_gamepadId);
    
    // Clear button states and callbacks
    m_currentButtonStates.clear();
    m_previousButtonStates.clear();
    m_buttonCallbacks.clear();
    m_axisCallbacks.clear();
    m_connectionCallbacks.clear();
    
    m_initialized = false;
    std::cout << "GamepadDevice " << m_gamepadId << " shutdown" << std::endl;
}

bool GamepadDevice::isButtonPressed(GamepadButton button) const {
    if (!m_initialized || !m_connected) {
        return false;
    }
    
    auto it = m_currentButtonStates.find(button);
    if (it != m_currentButtonStates.end()) {
        return it->second == GamepadButtonState::Pressed;
    }
    
    return false;
}

bool GamepadDevice::isButtonJustPressed(GamepadButton button) const {
    if (!m_initialized || !m_connected) {
        return false;
    }
    
    auto currentIt = m_currentButtonStates.find(button);
    auto previousIt = m_previousButtonStates.find(button);
    
    bool currentPressed = (currentIt != m_currentButtonStates.end()) && 
                         (currentIt->second == GamepadButtonState::Pressed);
    
    bool previousPressed = (previousIt != m_previousButtonStates.end()) && 
                          (previousIt->second == GamepadButtonState::Pressed);
    
    return currentPressed && !previousPressed;
}

bool GamepadDevice::isButtonJustReleased(GamepadButton button) const {
    if (!m_initialized || !m_connected) {
        return false;
    }
    
    auto currentIt = m_currentButtonStates.find(button);
    auto previousIt = m_previousButtonStates.find(button);
    
    bool currentReleased = (currentIt == m_currentButtonStates.end()) || 
                          (currentIt->second == GamepadButtonState::Released);
    
    bool previousPressed = (previousIt != m_previousButtonStates.end()) && 
                          (previousIt->second == GamepadButtonState::Pressed);
    
    return currentReleased && previousPressed;
}

GamepadButtonState GamepadDevice::getButtonState(GamepadButton button) const {
    if (!m_initialized || !m_connected) {
        return GamepadButtonState::Released;
    }
    
    auto it = m_currentButtonStates.find(button);
    if (it != m_currentButtonStates.end()) {
        return it->second;
    }
    
    return GamepadButtonState::Released;
}

float GamepadDevice::getAxisValue(GamepadAxis axis) const {
    if (!m_initialized || !m_connected) {
        return 0.0f;
    }
    
    int axisIndex = static_cast<int>(axis);
    if (axisIndex >= 0 && axisIndex < static_cast<int>(m_axisValues.size())) {
        return m_axisValues[axisIndex];
    }
    
    return 0.0f;
}

void GamepadDevice::setDeadzone(float deadzone) {
    m_deadzone = std::max(0.0f, std::min(1.0f, deadzone));
}

int GamepadDevice::registerButtonCallback(const std::function<void(const GamepadButtonEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_buttonCallbacks[callbackId] = callback;
    return callbackId;
}

bool GamepadDevice::unregisterButtonCallback(int callbackId) {
    if (!m_initialized || callbackId < 1) {
        return false;
    }
    
    auto it = m_buttonCallbacks.find(callbackId);
    if (it != m_buttonCallbacks.end()) {
        m_buttonCallbacks.erase(it);
        return true;
    }
    
    return false;
}

int GamepadDevice::registerAxisCallback(const std::function<void(const GamepadAxisEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_axisCallbacks[callbackId] = callback;
    return callbackId;
}

bool GamepadDevice::unregisterAxisCallback(int callbackId) {
    if (!m_initialized || callbackId < 1) {
        return false;
    }
    
    auto it = m_axisCallbacks.find(callbackId);
    if (it != m_axisCallbacks.end()) {
        m_axisCallbacks.erase(it);
        return true;
    }
    
    return false;
}

int GamepadDevice::registerConnectionCallback(const std::function<void(const GamepadConnectionEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_connectionCallbacks[callbackId] = callback;
    return callbackId;
}

bool GamepadDevice::unregisterConnectionCallback(int callbackId) {
    if (!m_initialized || callbackId < 1) {
        return false;
    }
    
    auto it = m_connectionCallbacks.find(callbackId);
    if (it != m_connectionCallbacks.end()) {
        m_connectionCallbacks.erase(it);
        return true;
    }
    
    return false;
}

void GamepadDevice::joystickCallback(int jid, int event) {
    // Find the gamepad device instance for this joystick
    auto it = s_instances.find(jid);
    if (it == s_instances.end()) {
        return;
    }
    
    GamepadDevice* device = it->second;
    
    // Update connection state
    bool wasConnected = device->m_connected;
    device->updateConnectionState();
    
    // If connection state changed, notify callbacks
    if (wasConnected != device->m_connected) {
        GamepadConnectionEvent connectionEvent(
            device->m_gamepadId,
            device->m_connected,
            device->m_name
        );
        
        for (const auto& pair : device->m_connectionCallbacks) {
            pair.second(connectionEvent);
        }
    }
}

void GamepadDevice::updateConnectionState() {
    bool wasConnected = m_connected;
    
    // Check if gamepad is present
    m_connected = glfwJoystickPresent(m_gamepadId) && glfwJoystickIsGamepad(m_gamepadId);
    
    // Update name if connected
    if (m_connected) {
        const char* name = glfwGetGamepadName(m_gamepadId);
        m_name = name ? name : "Unknown Gamepad";
    } else {
        m_name = "Disconnected Gamepad";
    }
    
    // If connection state changed, log it
    if (wasConnected != m_connected) {
        if (m_connected) {
            std::cout << "Gamepad " << m_gamepadId << " connected: " << m_name << std::endl;
        } else {
            std::cout << "Gamepad " << m_gamepadId << " disconnected" << std::endl;
        }
    }
}

float GamepadDevice::applyDeadzone(float value) const {
    // Apply deadzone
    if (std::abs(value) < m_deadzone) {
        return 0.0f;
    }
    
    // Rescale value to account for deadzone
    float sign = (value > 0.0f) ? 1.0f : -1.0f;
    return sign * (std::abs(value) - m_deadzone) / (1.0f - m_deadzone);
}

} // namespace Input
} // namespace RPGEngine