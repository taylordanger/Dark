#pragma once

#include "IInputDevice.h"
#include <unordered_map>
#include <vector>
#include <functional>

// Forward declarations for GLFW
struct GLFWwindow;

namespace RPGEngine {
namespace Input {

/**
 * Mouse button enumeration
 * Based on GLFW mouse button codes for compatibility
 */
enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7,
    Unknown = -1
};

/**
 * Mouse button state enumeration
 */
enum class MouseButtonState {
    Released,
    Pressed
};

/**
 * Mouse button event structure
 */
struct MouseButtonEvent {
    MouseButton button;
    MouseButtonState state;
    double x;
    double y;
    bool shift;
    bool control;
    bool alt;
    bool super;
    
    MouseButtonEvent(MouseButton button, MouseButtonState state, double x, double y,
                    bool shift, bool control, bool alt, bool super)
        : button(button), state(state), x(x), y(y),
          shift(shift), control(control), alt(alt), super(super) {}
};

/**
 * Mouse move event structure
 */
struct MouseMoveEvent {
    double x;
    double y;
    double deltaX;
    double deltaY;
    
    MouseMoveEvent(double x, double y, double deltaX, double deltaY)
        : x(x), y(y), deltaX(deltaX), deltaY(deltaY) {}
};

/**
 * Mouse scroll event structure
 */
struct MouseScrollEvent {
    double x;
    double y;
    double offsetX;
    double offsetY;
    
    MouseScrollEvent(double x, double y, double offsetX, double offsetY)
        : x(x), y(y), offsetX(offsetX), offsetY(offsetY) {}
};

/**
 * Mouse device
 * Handles mouse input
 */
class MouseDevice : public IInputDevice {
public:
    /**
     * Constructor
     * @param window GLFW window handle
     */
    explicit MouseDevice(GLFWwindow* window);
    
    /**
     * Destructor
     */
    ~MouseDevice();
    
    // IInputDevice interface implementation
    bool initialize() override;
    void update() override;
    void shutdown() override;
    InputDeviceType getType() const override { return InputDeviceType::Mouse; }
    const std::string& getName() const override { return m_name; }
    bool isConnected() const override { return true; } // Mouse is always connected
    bool isInitialized() const override { return m_initialized; }
    
    /**
     * Check if a mouse button is pressed
     * @param button Mouse button
     * @return true if the button is pressed
     */
    bool isButtonPressed(MouseButton button) const;
    
    /**
     * Check if a mouse button was just pressed this frame
     * @param button Mouse button
     * @return true if the button was just pressed
     */
    bool isButtonJustPressed(MouseButton button) const;
    
    /**
     * Check if a mouse button was just released this frame
     * @param button Mouse button
     * @return true if the button was just released
     */
    bool isButtonJustReleased(MouseButton button) const;
    
    /**
     * Get the state of a mouse button
     * @param button Mouse button
     * @return Mouse button state
     */
    MouseButtonState getButtonState(MouseButton button) const;
    
    /**
     * Get the mouse position
     * @param x Output X position
     * @param y Output Y position
     */
    void getPosition(double& x, double& y) const;
    
    /**
     * Get the mouse position X
     * @return X position
     */
    double getPositionX() const { return m_positionX; }
    
    /**
     * Get the mouse position Y
     * @return Y position
     */
    double getPositionY() const { return m_positionY; }
    
    /**
     * Get the mouse movement delta
     * @param deltaX Output X delta
     * @param deltaY Output Y delta
     */
    void getDelta(double& deltaX, double& deltaY) const;
    
    /**
     * Get the mouse scroll offset
     * @param offsetX Output X offset
     * @param offsetY Output Y offset
     */
    void getScrollOffset(double& offsetX, double& offsetY) const;
    
    /**
     * Set the mouse position
     * @param x X position
     * @param y Y position
     */
    void setPosition(double x, double y);
    
    /**
     * Set the mouse cursor visibility
     * @param visible Whether the cursor should be visible
     */
    void setCursorVisible(bool visible);
    
    /**
     * Check if the mouse cursor is visible
     * @return true if the cursor is visible
     */
    bool isCursorVisible() const { return m_cursorVisible; }
    
    /**
     * Set the mouse cursor lock state
     * @param locked Whether the cursor should be locked to the window
     */
    void setCursorLocked(bool locked);
    
    /**
     * Check if the mouse cursor is locked
     * @return true if the cursor is locked
     */
    bool isCursorLocked() const { return m_cursorLocked; }
    
    /**
     * Register a mouse button event callback
     * @param callback Function to call when a mouse button event occurs
     * @return Callback ID for unregistering
     */
    int registerButtonCallback(const std::function<void(const MouseButtonEvent&)>& callback);
    
    /**
     * Unregister a mouse button event callback
     * @param callbackId Callback ID returned from registerButtonCallback
     * @return true if the callback was unregistered
     */
    bool unregisterButtonCallback(int callbackId);
    
    /**
     * Register a mouse move event callback
     * @param callback Function to call when a mouse move event occurs
     * @return Callback ID for unregistering
     */
    int registerMoveCallback(const std::function<void(const MouseMoveEvent&)>& callback);
    
    /**
     * Unregister a mouse move event callback
     * @param callbackId Callback ID returned from registerMoveCallback
     * @return true if the callback was unregistered
     */
    bool unregisterMoveCallback(int callbackId);
    
    /**
     * Register a mouse scroll event callback
     * @param callback Function to call when a mouse scroll event occurs
     * @return Callback ID for unregistering
     */
    int registerScrollCallback(const std::function<void(const MouseScrollEvent&)>& callback);
    
    /**
     * Unregister a mouse scroll event callback
     * @param callbackId Callback ID returned from registerScrollCallback
     * @return true if the callback was unregistered
     */
    bool unregisterScrollCallback(int callbackId);
    
    /**
     * Static mouse button callback for GLFW
     * @param window GLFW window handle
     * @param button Button code
     * @param action Action (press, release)
     * @param mods Modifier keys
     */
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    
    /**
     * Static mouse position callback for GLFW
     * @param window GLFW window handle
     * @param xpos X position
     * @param ypos Y position
     */
    static void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
    
    /**
     * Static mouse scroll callback for GLFW
     * @param window GLFW window handle
     * @param xoffset X scroll offset
     * @param yoffset Y scroll offset
     */
    static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
private:
    // Window handle
    GLFWwindow* m_window;
    
    // Device name
    std::string m_name;
    
    // Mouse state
    std::unordered_map<MouseButton, MouseButtonState> m_currentButtonStates;
    std::unordered_map<MouseButton, MouseButtonState> m_previousButtonStates;
    
    double m_positionX;
    double m_positionY;
    double m_previousPositionX;
    double m_previousPositionY;
    double m_deltaX;
    double m_deltaY;
    
    double m_scrollOffsetX;
    double m_scrollOffsetY;
    
    bool m_cursorVisible;
    bool m_cursorLocked;
    
    // Callbacks
    std::unordered_map<int, std::function<void(const MouseButtonEvent&)>> m_buttonCallbacks;
    std::unordered_map<int, std::function<void(const MouseMoveEvent&)>> m_moveCallbacks;
    std::unordered_map<int, std::function<void(const MouseScrollEvent&)>> m_scrollCallbacks;
    int m_nextCallbackId;
    
    // Initialization state
    bool m_initialized;
    
    // Static instance map for callbacks
    static std::unordered_map<GLFWwindow*, MouseDevice*> s_instances;
};

} // namespace Input
} // namespace RPGEngine