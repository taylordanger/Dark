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
 * Key code enumeration
 * Based on GLFW key codes for compatibility
 */
enum class KeyCode {
    // Printable keys
    Space = 32,
    Apostrophe = 39,  // '
    Comma = 44,       // ,
    Minus = 45,       // -
    Period = 46,      // .
    Slash = 47,       // /
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,
    Semicolon = 59,   // ;
    Equal = 61,       // =
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LeftBracket = 91,  // [
    Backslash = 92,    // \
    RightBracket = 93, // ]
    GraveAccent = 96,  // `
    
    // Function keys
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    
    // Keypad
    KP0 = 320,
    KP1 = 321,
    KP2 = 322,
    KP3 = 323,
    KP4 = 324,
    KP5 = 325,
    KP6 = 326,
    KP7 = 327,
    KP8 = 328,
    KP9 = 329,
    KPDecimal = 330,
    KPDivide = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd = 334,
    KPEnter = 335,
    KPEqual = 336,
    
    // Modifiers
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,
    
    // Special
    Unknown = -1
};

/**
 * Key state enumeration
 */
enum class KeyState {
    Released,
    Pressed,
    Repeated
};

/**
 * Key event structure
 */
struct KeyEvent {
    KeyCode key;
    KeyState state;
    bool shift;
    bool control;
    bool alt;
    bool super;
    
    KeyEvent(KeyCode key, KeyState state, bool shift, bool control, bool alt, bool super)
        : key(key), state(state), shift(shift), control(control), alt(alt), super(super) {}
};

/**
 * Keyboard device
 * Handles keyboard input
 */
class KeyboardDevice : public IInputDevice {
public:
    /**
     * Constructor
     * @param window GLFW window handle
     */
    explicit KeyboardDevice(GLFWwindow* window);
    
    /**
     * Destructor
     */
    ~KeyboardDevice();
    
    // IInputDevice interface implementation
    bool initialize() override;
    void update() override;
    void shutdown() override;
    InputDeviceType getType() const override { return InputDeviceType::Keyboard; }
    const std::string& getName() const override { return m_name; }
    bool isConnected() const override { return true; } // Keyboard is always connected
    bool isInitialized() const override { return m_initialized; }
    
    /**
     * Check if a key is pressed
     * @param key Key code
     * @return true if the key is pressed
     */
    bool isKeyPressed(KeyCode key) const;
    
    /**
     * Check if a key was just pressed this frame
     * @param key Key code
     * @return true if the key was just pressed
     */
    bool isKeyJustPressed(KeyCode key) const;
    
    /**
     * Check if a key was just released this frame
     * @param key Key code
     * @return true if the key was just released
     */
    bool isKeyJustReleased(KeyCode key) const;
    
    /**
     * Get the state of a key
     * @param key Key code
     * @return Key state
     */
    KeyState getKeyState(KeyCode key) const;
    
    /**
     * Check if shift is pressed
     * @return true if shift is pressed
     */
    bool isShiftPressed() const;
    
    /**
     * Check if control is pressed
     * @return true if control is pressed
     */
    bool isControlPressed() const;
    
    /**
     * Check if alt is pressed
     * @return true if alt is pressed
     */
    bool isAltPressed() const;
    
    /**
     * Check if super (Windows/Command) is pressed
     * @return true if super is pressed
     */
    bool isSuperPressed() const;
    
    /**
     * Register a key event callback
     * @param callback Function to call when a key event occurs
     * @return Callback ID for unregistering
     */
    int registerKeyCallback(const std::function<void(const KeyEvent&)>& callback);
    
    /**
     * Unregister a key event callback
     * @param callbackId Callback ID returned from registerKeyCallback
     * @return true if the callback was unregistered
     */
    bool unregisterKeyCallback(int callbackId);
    
    /**
     * Static key callback for GLFW
     * @param window GLFW window handle
     * @param key Key code
     * @param scancode Scan code
     * @param action Action (press, release, repeat)
     * @param mods Modifier keys
     */
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
private:
    // Window handle
    GLFWwindow* m_window;
    
    // Device name
    std::string m_name;
    
    // Key states
    std::unordered_map<KeyCode, KeyState> m_currentKeyStates;
    std::unordered_map<KeyCode, KeyState> m_previousKeyStates;
    
    // Callbacks
    std::unordered_map<int, std::function<void(const KeyEvent&)>> m_keyCallbacks;
    int m_nextCallbackId;
    
    // Initialization state
    bool m_initialized;
    
    // Static instance map for callbacks
    static std::unordered_map<GLFWwindow*, KeyboardDevice*> s_instances;
};

} // namespace Input
} // namespace RPGEngine