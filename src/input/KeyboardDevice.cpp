#include "KeyboardDevice.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace RPGEngine {
namespace Input {

// Initialize static instance map
std::unordered_map<GLFWwindow*, KeyboardDevice*> KeyboardDevice::s_instances;

KeyboardDevice::KeyboardDevice(GLFWwindow* window)
    : m_window(window)
    , m_name("Keyboard")
    , m_nextCallbackId(1)
    , m_initialized(false)
{
}

KeyboardDevice::~KeyboardDevice() {
    if (m_initialized) {
        shutdown();
    }
}

bool KeyboardDevice::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_window) {
        std::cerr << "Invalid window handle provided to KeyboardDevice" << std::endl;
        return false;
    }
    
    // Register this instance for callbacks
    s_instances[m_window] = this;
    
    // Set GLFW key callback
    glfwSetKeyCallback(m_window, keyCallback);
    
    m_initialized = true;
    std::cout << "KeyboardDevice initialized" << std::endl;
    return true;
}

void KeyboardDevice::update() {
    if (!m_initialized) {
        return;
    }
    
    // Store current key states as previous
    m_previousKeyStates = m_currentKeyStates;
}

void KeyboardDevice::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Remove this instance from the callback map
    s_instances.erase(m_window);
    
    // Clear key states and callbacks
    m_currentKeyStates.clear();
    m_previousKeyStates.clear();
    m_keyCallbacks.clear();
    
    m_initialized = false;
    std::cout << "KeyboardDevice shutdown" << std::endl;
}

bool KeyboardDevice::isKeyPressed(KeyCode key) const {
    if (!m_initialized) {
        return false;
    }
    
    auto it = m_currentKeyStates.find(key);
    if (it != m_currentKeyStates.end()) {
        return it->second == KeyState::Pressed || it->second == KeyState::Repeated;
    }
    
    return false;
}

bool KeyboardDevice::isKeyJustPressed(KeyCode key) const {
    if (!m_initialized) {
        return false;
    }
    
    auto currentIt = m_currentKeyStates.find(key);
    auto previousIt = m_previousKeyStates.find(key);
    
    bool currentPressed = (currentIt != m_currentKeyStates.end()) && 
                         (currentIt->second == KeyState::Pressed || currentIt->second == KeyState::Repeated);
    
    bool previousPressed = (previousIt != m_previousKeyStates.end()) && 
                          (previousIt->second == KeyState::Pressed || previousIt->second == KeyState::Repeated);
    
    return currentPressed && !previousPressed;
}

bool KeyboardDevice::isKeyJustReleased(KeyCode key) const {
    if (!m_initialized) {
        return false;
    }
    
    auto currentIt = m_currentKeyStates.find(key);
    auto previousIt = m_previousKeyStates.find(key);
    
    bool currentReleased = (currentIt == m_currentKeyStates.end()) || (currentIt->second == KeyState::Released);
    bool previousPressed = (previousIt != m_previousKeyStates.end()) && 
                          (previousIt->second == KeyState::Pressed || previousIt->second == KeyState::Repeated);
    
    return currentReleased && previousPressed;
}

KeyState KeyboardDevice::getKeyState(KeyCode key) const {
    if (!m_initialized) {
        return KeyState::Released;
    }
    
    auto it = m_currentKeyStates.find(key);
    if (it != m_currentKeyStates.end()) {
        return it->second;
    }
    
    return KeyState::Released;
}

bool KeyboardDevice::isShiftPressed() const {
    return isKeyPressed(KeyCode::LeftShift) || isKeyPressed(KeyCode::RightShift);
}

bool KeyboardDevice::isControlPressed() const {
    return isKeyPressed(KeyCode::LeftControl) || isKeyPressed(KeyCode::RightControl);
}

bool KeyboardDevice::isAltPressed() const {
    return isKeyPressed(KeyCode::LeftAlt) || isKeyPressed(KeyCode::RightAlt);
}

bool KeyboardDevice::isSuperPressed() const {
    return isKeyPressed(KeyCode::LeftSuper) || isKeyPressed(KeyCode::RightSuper);
}

int KeyboardDevice::registerKeyCallback(const std::function<void(const KeyEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_keyCallbacks[callbackId] = callback;
    return callbackId;
}

bool KeyboardDevice::unregisterKeyCallback(int callbackId) {
    if (!m_initialized || callbackId < 1) {
        return false;
    }
    
    auto it = m_keyCallbacks.find(callbackId);
    if (it != m_keyCallbacks.end()) {
        m_keyCallbacks.erase(it);
        return true;
    }
    
    return false;
}

void KeyboardDevice::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Find the keyboard device instance for this window
    auto it = s_instances.find(window);
    if (it == s_instances.end()) {
        return;
    }
    
    KeyboardDevice* device = it->second;
    
    // Convert GLFW key to our key code
    KeyCode keyCode = static_cast<KeyCode>(key);
    
    // Convert GLFW action to our key state
    KeyState keyState;
    switch (action) {
        case GLFW_PRESS:
            keyState = KeyState::Pressed;
            break;
        case GLFW_RELEASE:
            keyState = KeyState::Released;
            break;
        case GLFW_REPEAT:
            keyState = KeyState::Repeated;
            break;
        default:
            return; // Unknown action
    }
    
    // Update key state
    device->m_currentKeyStates[keyCode] = keyState;
    
    // Create key event
    KeyEvent event(
        keyCode,
        keyState,
        (mods & GLFW_MOD_SHIFT) != 0,
        (mods & GLFW_MOD_CONTROL) != 0,
        (mods & GLFW_MOD_ALT) != 0,
        (mods & GLFW_MOD_SUPER) != 0
    );
    
    // Notify callbacks
    for (const auto& pair : device->m_keyCallbacks) {
        pair.second(event);
    }
}

} // namespace Input
} // namespace RPGEngine