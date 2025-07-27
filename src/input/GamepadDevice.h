#pragma once

#include "IInputDevice.h"
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <array>

// Forward declarations for GLFW
struct GLFWwindow;

namespace RPGEngine {
namespace Input {

/**
 * Gamepad button enumeration
 * Based on GLFW gamepad button codes for compatibility
 */
enum class GamepadButton {
    A = 0,
    B = 1,
    X = 2,
    Y = 3,
    LeftBumper = 4,
    RightBumper = 5,
    Back = 6,
    Start = 7,
    Guide = 8,
    LeftThumb = 9,
    RightThumb = 10,
    DPadUp = 11,
    DPadRight = 12,
    DPadDown = 13,
    DPadLeft = 14,
    Unknown = -1
};

/**
 * Gamepad axis enumeration
 * Based on GLFW gamepad axis codes for compatibility
 */
enum class GamepadAxis {
    LeftX = 0,
    LeftY = 1,
    RightX = 2,
    RightY = 3,
    LeftTrigger = 4,
    RightTrigger = 5,
    Unknown = -1
};

/**
 * Gamepad button state enumeration
 */
enum class GamepadButtonState {
    Released,
    Pressed
};

/**
 * Gamepad button event structure
 */
struct GamepadButtonEvent {
    int gamepadId;
    GamepadButton button;
    GamepadButtonState state;
    
    GamepadButtonEvent(int gamepadId, GamepadButton button, GamepadButtonState state)
        : gamepadId(gamepadId), button(button), state(state) {}
};

/**
 * Gamepad axis event structure
 */
struct GamepadAxisEvent {
    int gamepadId;
    GamepadAxis axis;
    float value;
    
    GamepadAxisEvent(int gamepadId, GamepadAxis axis, float value)
        : gamepadId(gamepadId), axis(axis), value(value) {}
};

/**
 * Gamepad connection event structure
 */
struct GamepadConnectionEvent {
    int gamepadId;
    bool connected;
    std::string name;
    
    GamepadConnectionEvent(int gamepadId, bool connected, const std::string& name)
        : gamepadId(gamepadId), connected(connected), name(name) {}
};

/**
 * Gamepad device
 * Handles gamepad input
 */
class GamepadDevice : public IInputDevice {
public:
    /**
     * Constructor
     * @param window GLFW window handle
     * @param gamepadId Gamepad ID (0-15)
     */
    GamepadDevice(GLFWwindow* window, int gamepadId);
    
    /**
     * Destructor
     */
    ~GamepadDevice();
    
    // IInputDevice interface implementation
    bool initialize() override;
    void update() override;
    void shutdown() override;
    InputDeviceType getType() const override { return InputDeviceType::Gamepad; }
    const std::string& getName() const override { return m_name; }
    bool isConnected() const override { return m_connected; }
    bool isInitialized() const override { return m_initialized; }
    
    /**
     * Get the gamepad ID
     * @return Gamepad ID
     */
    int getGamepadId() const { return m_gamepadId; }
    
    /**
     * Check if a gamepad button is pressed
     * @param button Gamepad button
     * @return true if the button is pressed
     */
    bool isButtonPressed(GamepadButton button) const;
    
    /**
     * Check if a gamepad button was just pressed this frame
     * @param button Gamepad button
     * @return true if the button was just pressed
     */
    bool isButtonJustPressed(GamepadButton button) const;
    
    /**
     * Check if a gamepad button was just released this frame
     * @param button Gamepad button
     * @return true if the button was just released
     */
    bool isButtonJustReleased(GamepadButton button) const;
    
    /**
     * Get the state of a gamepad button
     * @param button Gamepad button
     * @return Gamepad button state
     */
    GamepadButtonState getButtonState(GamepadButton button) const;
    
    /**
     * Get the value of a gamepad axis
     * @param axis Gamepad axis
     * @return Axis value (-1.0 to 1.0)
     */
    float getAxisValue(GamepadAxis axis) const;
    
    /**
     * Set the deadzone for analog sticks
     * @param deadzone Deadzone value (0.0-1.0)
     */
    void setDeadzone(float deadzone);
    
    /**
     * Get the deadzone for analog sticks
     * @return Deadzone value
     */
    float getDeadzone() const { return m_deadzone; }
    
    /**
     * Register a gamepad button event callback
     * @param callback Function to call when a gamepad button event occurs
     * @return Callback ID for unregistering
     */
    int registerButtonCallback(const std::function<void(const GamepadButtonEvent&)>& callback);
    
    /**
     * Unregister a gamepad button event callback
     * @param callbackId Callback ID returned from registerButtonCallback
     * @return true if the callback was unregistered
     */
    bool unregisterButtonCallback(int callbackId);
    
    /**
     * Register a gamepad axis event callback
     * @param callback Function to call when a gamepad axis event occurs
     * @return Callback ID for unregistering
     */
    int registerAxisCallback(const std::function<void(const GamepadAxisEvent&)>& callback);
    
    /**
     * Unregister a gamepad axis event callback
     * @param callbackId Callback ID returned from registerAxisCallback
     * @return true if the callback was unregistered
     */
    bool unregisterAxisCallback(int callbackId);
    
    /**
     * Register a gamepad connection event callback
     * @param callback Function to call when a gamepad connection event occurs
     * @return Callback ID for unregistering
     */
    int registerConnectionCallback(const std::function<void(const GamepadConnectionEvent&)>& callback);
    
    /**
     * Unregister a gamepad connection event callback
     * @param callbackId Callback ID returned from registerConnectionCallback
     * @return true if the callback was unregistered
     */
    bool unregisterConnectionCallback(int callbackId);
    
    /**
     * Static gamepad connection callback for GLFW
     * @param jid Joystick ID
     * @param event Connection event
     */
    static void joystickCallback(int jid, int event);
    
private:
    /**
     * Update gamepad connection state
     */
    void updateConnectionState();
    
    /**
     * Apply deadzone to axis value
     * @param value Raw axis value
     * @return Processed axis value
     */
    float applyDeadzone(float value) const;
    
    // Window handle
    GLFWwindow* m_window;
    
    // Gamepad ID
    int m_gamepadId;
    
    // Device name
    std::string m_name;
    
    // Connection state
    bool m_connected;
    
    // Button states
    std::unordered_map<GamepadButton, GamepadButtonState> m_currentButtonStates;
    std::unordered_map<GamepadButton, GamepadButtonState> m_previousButtonStates;
    
    // Axis values
    std::array<float, 6> m_axisValues;
    std::array<float, 6> m_previousAxisValues;
    
    // Deadzone
    float m_deadzone;
    
    // Callbacks
    std::unordered_map<int, std::function<void(const GamepadButtonEvent&)>> m_buttonCallbacks;
    std::unordered_map<int, std::function<void(const GamepadAxisEvent&)>> m_axisCallbacks;
    std::unordered_map<int, std::function<void(const GamepadConnectionEvent&)>> m_connectionCallbacks;
    int m_nextCallbackId;
    
    // Initialization state
    bool m_initialized;
    
    // Static instance map for callbacks
    static std::unordered_map<int, GamepadDevice*> s_instances;
};

} // namespace Input
} // namespace RPGEngine