#include "MouseDevice.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace RPGEngine {
namespace Input {

// Initialize static instance map
std::unordered_map<GLFWwindow*, MouseDevice*> MouseDevice::s_instances;

MouseDevice::MouseDevice(GLFWwindow* window)
    : m_window(window)
    , m_name("Mouse")
    , m_positionX(0.0)
    , m_positionY(0.0)
    , m_previousPositionX(0.0)
    , m_previousPositionY(0.0)
    , m_deltaX(0.0)
    , m_deltaY(0.0)
    , m_scrollOffsetX(0.0)
    , m_scrollOffsetY(0.0)
    , m_cursorVisible(true)
    , m_cursorLocked(false)
    , m_nextCallbackId(1)
    , m_initialized(false)
{
}

MouseDevice::~MouseDevice() {
    if (m_initialized) {
        shutdown();
    }
}

bool MouseDevice::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_window) {
        std::cerr << "Invalid window handle provided to MouseDevice" << std::endl;
        return false;
    }
    
    // Register this instance for callbacks
    s_instances[m_window] = this;
    
    // Set GLFW mouse callbacks
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, mousePosCallback);
    glfwSetScrollCallback(m_window, mouseScrollCallback);
    
    // Get initial mouse position
    glfwGetCursorPos(m_window, &m_positionX, &m_positionY);
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    
    m_initialized = true;
    std::cout << "MouseDevice initialized" << std::endl;
    return true;
}

void MouseDevice::update() {
    if (!m_initialized) {
        return;
    }
    
    // Store current button states as previous
    m_previousButtonStates = m_currentButtonStates;
    
    // Update mouse position delta
    m_deltaX = m_positionX - m_previousPositionX;
    m_deltaY = m_positionY - m_previousPositionY;
    
    // Store current position as previous
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    
    // Reset scroll offset
    m_scrollOffsetX = 0.0;
    m_scrollOffsetY = 0.0;
}

void MouseDevice::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Remove this instance from the callback map
    s_instances.erase(m_window);
    
    // Clear button states and callbacks
    m_currentButtonStates.clear();
    m_previousButtonStates.clear();
    m_buttonCallbacks.clear();
    m_moveCallbacks.clear();
    m_scrollCallbacks.clear();
    
    m_initialized = false;
    std::cout << "MouseDevice shutdown" << std::endl;
}

bool MouseDevice::isButtonPressed(MouseButton button) const {
    if (!m_initialized) {
        return false;
    }
    
    auto it = m_currentButtonStates.find(button);
    if (it != m_currentButtonStates.end()) {
        return it->second == MouseButtonState::Pressed;
    }
    
    return false;
}

bool MouseDevice::isButtonJustPressed(MouseButton button) const {
    if (!m_initialized) {
        return false;
    }
    
    auto currentIt = m_currentButtonStates.find(button);
    auto previousIt = m_previousButtonStates.find(button);
    
    bool currentPressed = (currentIt != m_currentButtonStates.end()) && 
                         (currentIt->second == MouseButtonState::Pressed);
    
    bool previousPressed = (previousIt != m_previousButtonStates.end()) && 
                          (previousIt->second == MouseButtonState::Pressed);
    
    return currentPressed && !previousPressed;
}

bool MouseDevice::isButtonJustReleased(MouseButton button) const {
    if (!m_initialized) {
        return false;
    }
    
    auto currentIt = m_currentButtonStates.find(button);
    auto previousIt = m_previousButtonStates.find(button);
    
    bool currentReleased = (currentIt == m_currentButtonStates.end()) || 
                          (currentIt->second == MouseButtonState::Released);
    
    bool previousPressed = (previousIt != m_previousButtonStates.end()) && 
                          (previousIt->second == MouseButtonState::Pressed);
    
    return currentReleased && previousPressed;
}

MouseButtonState MouseDevice::getButtonState(MouseButton button) const {
    if (!m_initialized) {
        return MouseButtonState::Released;
    }
    
    auto it = m_currentButtonStates.find(button);
    if (it != m_currentButtonStates.end()) {
        return it->second;
    }
    
    return MouseButtonState::Released;
}

void MouseDevice::getPosition(double& x, double& y) const {
    x = m_positionX;
    y = m_positionY;
}

void MouseDevice::getDelta(double& deltaX, double& deltaY) const {
    deltaX = m_deltaX;
    deltaY = m_deltaY;
}

void MouseDevice::getScrollOffset(double& offsetX, double& offsetY) const {
    offsetX = m_scrollOffsetX;
    offsetY = m_scrollOffsetY;
}

void MouseDevice::setPosition(double x, double y) {
    if (!m_initialized) {
        return;
    }
    
    glfwSetCursorPos(m_window, x, y);
    m_positionX = x;
    m_positionY = y;
}

void MouseDevice::setCursorVisible(bool visible) {
    if (!m_initialized || m_cursorVisible == visible) {
        return;
    }
    
    glfwSetInputMode(m_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    m_cursorVisible = visible;
}

void MouseDevice::setCursorLocked(bool locked) {
    if (!m_initialized || m_cursorLocked == locked) {
        return;
    }
    
    glfwSetInputMode(m_window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    m_cursorLocked = locked;
    
    // Update cursor visibility based on lock state
    m_cursorVisible = !locked;
}

int MouseDevice::registerButtonCallback(const std::function<void(const MouseButtonEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_buttonCallbacks[callbackId] = callback;
    return callbackId;
}

bool MouseDevice::unregisterButtonCallback(int callbackId) {
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

int MouseDevice::registerMoveCallback(const std::function<void(const MouseMoveEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_moveCallbacks[callbackId] = callback;
    return callbackId;
}

bool MouseDevice::unregisterMoveCallback(int callbackId) {
    if (!m_initialized || callbackId < 1) {
        return false;
    }
    
    auto it = m_moveCallbacks.find(callbackId);
    if (it != m_moveCallbacks.end()) {
        m_moveCallbacks.erase(it);
        return true;
    }
    
    return false;
}

int MouseDevice::registerScrollCallback(const std::function<void(const MouseScrollEvent&)>& callback) {
    if (!m_initialized || !callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_scrollCallbacks[callbackId] = callback;
    return callbackId;
}

bool MouseDevice::unregisterScrollCallback(int callbackId) {
    if (!m_initialized || callbackId < 1) {
        return false;
    }
    
    auto it = m_scrollCallbacks.find(callbackId);
    if (it != m_scrollCallbacks.end()) {
        m_scrollCallbacks.erase(it);
        return true;
    }
    
    return false;
}

void MouseDevice::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Find the mouse device instance for this window
    auto it = s_instances.find(window);
    if (it == s_instances.end()) {
        return;
    }
    
    MouseDevice* device = it->second;
    
    // Convert GLFW button to our mouse button
    MouseButton mouseButton = static_cast<MouseButton>(button);
    
    // Convert GLFW action to our mouse button state
    MouseButtonState buttonState = (action == GLFW_PRESS) ? 
                                  MouseButtonState::Pressed : 
                                  MouseButtonState::Released;
    
    // Update button state
    device->m_currentButtonStates[mouseButton] = buttonState;
    
    // Get current mouse position
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    
    // Create mouse button event
    MouseButtonEvent event(
        mouseButton,
        buttonState,
        x,
        y,
        (mods & GLFW_MOD_SHIFT) != 0,
        (mods & GLFW_MOD_CONTROL) != 0,
        (mods & GLFW_MOD_ALT) != 0,
        (mods & GLFW_MOD_SUPER) != 0
    );
    
    // Notify callbacks
    for (const auto& pair : device->m_buttonCallbacks) {
        pair.second(event);
    }
}

void MouseDevice::mousePosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Find the mouse device instance for this window
    auto it = s_instances.find(window);
    if (it == s_instances.end()) {
        return;
    }
    
    MouseDevice* device = it->second;
    
    // Calculate delta
    double deltaX = xpos - device->m_positionX;
    double deltaY = ypos - device->m_positionY;
    
    // Update position
    device->m_positionX = xpos;
    device->m_positionY = ypos;
    
    // Create mouse move event
    MouseMoveEvent event(xpos, ypos, deltaX, deltaY);
    
    // Notify callbacks
    for (const auto& pair : device->m_moveCallbacks) {
        pair.second(event);
    }
}

void MouseDevice::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Find the mouse device instance for this window
    auto it = s_instances.find(window);
    if (it == s_instances.end()) {
        return;
    }
    
    MouseDevice* device = it->second;
    
    // Update scroll offset
    device->m_scrollOffsetX += xoffset;
    device->m_scrollOffsetY += yoffset;
    
    // Get current mouse position
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    
    // Create mouse scroll event
    MouseScrollEvent event(x, y, xoffset, yoffset);
    
    // Notify callbacks
    for (const auto& pair : device->m_scrollCallbacks) {
        pair.second(event);
    }
}

} // namespace Input
} // namespace RPGEngine