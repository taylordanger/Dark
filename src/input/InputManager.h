#pragma once

#include "IInputDevice.h"
#include "KeyboardDevice.h"
#include "MouseDevice.h"
#include "GamepadDevice.h"
#include "../systems/System.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

// Forward declarations for GLFW
struct GLFWwindow;

namespace RPGEngine {
namespace Input {

/**
 * Input action structure
 * Represents a named input action that can be triggered by various inputs
 */
struct InputAction {
    std::string name;
    bool active;
    bool justActivated;
    bool justDeactivated;
    float value;
    
    InputAction(const std::string& name)
        : name(name), active(false), justActivated(false), justDeactivated(false), value(0.0f) {}
};

/**
 * Input binding type enumeration
 */
enum class InputBindingType {
    KeyboardKey,
    MouseButton,
    MouseAxis,
    GamepadButton,
    GamepadAxis,
    Custom
};

/**
 * Input binding structure
 * Represents a binding between an input source and an action
 */
struct InputBinding {
    std::string actionName;
    InputBindingType type;
    int code;
    float scale;
    
    InputBinding(const std::string& actionName, InputBindingType type, int code, float scale = 1.0f)
        : actionName(actionName), type(type), code(code), scale(scale) {}
};

/**
 * Input manager
 * Manages input devices and input mapping
 */
class InputManager : public System {
public:
    /**
     * Constructor
     * @param window GLFW window handle
     */
    explicit InputManager(GLFWwindow* window);
    
    /**
     * Destructor
     */
    ~InputManager();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Get the keyboard device
     * @return Shared pointer to the keyboard device
     */
    std::shared_ptr<KeyboardDevice> getKeyboardDevice() const { return m_keyboardDevice; }
    
    /**
     * Get the mouse device
     * @return Shared pointer to the mouse device
     */
    std::shared_ptr<MouseDevice> getMouseDevice() const { return m_mouseDevice; }
    
    /**
     * Get a gamepad device by ID
     * @param gamepadId Gamepad ID
     * @return Shared pointer to the gamepad device, or nullptr if not found
     */
    std::shared_ptr<GamepadDevice> getGamepadDevice(int gamepadId) const;
    
    /**
     * Get all gamepad devices
     * @return Vector of gamepad devices
     */
    const std::vector<std::shared_ptr<GamepadDevice>>& getGamepadDevices() const { return m_gamepadDevices; }
    
    /**
     * Get the number of connected gamepads
     * @return Number of connected gamepads
     */
    size_t getConnectedGamepadCount() const;
    
    /**
     * Get an input device by type
     * @param type Input device type
     * @return Shared pointer to the input device, or nullptr if not found
     */
    std::shared_ptr<IInputDevice> getDevice(InputDeviceType type) const;
    
    /**
     * Get an input device by name
     * @param name Input device name
     * @return Shared pointer to the input device, or nullptr if not found
     */
    std::shared_ptr<IInputDevice> getDeviceByName(const std::string& name) const;
    
    /**
     * Register an input device
     * @param device Input device to register
     * @return true if the device was registered
     */
    bool registerDevice(std::shared_ptr<IInputDevice> device);
    
    /**
     * Unregister an input device
     * @param device Input device to unregister
     * @return true if the device was unregistered
     */
    bool unregisterDevice(std::shared_ptr<IInputDevice> device);
    
    /**
     * Unregister an input device by type
     * @param type Input device type
     * @return true if the device was unregistered
     */
    bool unregisterDeviceByType(InputDeviceType type);
    
    /**
     * Unregister an input device by name
     * @param name Input device name
     * @return true if the device was unregistered
     */
    bool unregisterDeviceByName(const std::string& name);
    
    /**
     * Create an input action
     * @param name Action name
     * @return true if the action was created
     */
    bool createAction(const std::string& name);
    
    /**
     * Remove an input action
     * @param name Action name
     * @return true if the action was removed
     */
    bool removeAction(const std::string& name);
    
    /**
     * Check if an input action exists
     * @param name Action name
     * @return true if the action exists
     */
    bool hasAction(const std::string& name) const;
    
    /**
     * Get an input action
     * @param name Action name
     * @return Pointer to the input action, or nullptr if not found
     */
    const InputAction* getAction(const std::string& name) const;
    
    /**
     * Check if an input action is active
     * @param name Action name
     * @return true if the action is active
     */
    bool isActionActive(const std::string& name) const;
    
    /**
     * Check if an input action was just activated this frame
     * @param name Action name
     * @return true if the action was just activated
     */
    bool isActionJustActivated(const std::string& name) const;
    
    /**
     * Check if an input action was just deactivated this frame
     * @param name Action name
     * @return true if the action was just deactivated
     */
    bool isActionJustDeactivated(const std::string& name) const;
    
    /**
     * Get the value of an input action
     * @param name Action name
     * @return Action value (0.0-1.0 for buttons, -1.0-1.0 for axes)
     */
    float getActionValue(const std::string& name) const;
    
    /**
     * Bind a keyboard key to an input action
     * @param actionName Action name
     * @param key Key code
     * @param scale Value scale (default: 1.0)
     * @return true if the binding was created
     */
    bool bindKeyToAction(const std::string& actionName, KeyCode key, float scale = 1.0f);
    
    /**
     * Bind a mouse button to an input action
     * @param actionName Action name
     * @param button Mouse button
     * @param scale Value scale (default: 1.0)
     * @return true if the binding was created
     */
    bool bindMouseButtonToAction(const std::string& actionName, MouseButton button, float scale = 1.0f);
    
    /**
     * Bind a mouse axis to an input action
     * @param actionName Action name
     * @param axis Axis (0 = X, 1 = Y)
     * @param scale Value scale (default: 1.0)
     * @return true if the binding was created
     */
    bool bindMouseAxisToAction(const std::string& actionName, int axis, float scale = 1.0f);
    
    /**
     * Bind a gamepad button to an input action
     * @param actionName Action name
     * @param button Gamepad button
     * @param gamepadId Gamepad ID (-1 for any gamepad)
     * @param scale Value scale (default: 1.0)
     * @return true if the binding was created
     */
    bool bindGamepadButtonToAction(const std::string& actionName, GamepadButton button, int gamepadId = -1, float scale = 1.0f);
    
    /**
     * Bind a gamepad axis to an input action
     * @param actionName Action name
     * @param axis Gamepad axis
     * @param gamepadId Gamepad ID (-1 for any gamepad)
     * @param scale Value scale (default: 1.0)
     * @return true if the binding was created
     */
    bool bindGamepadAxisToAction(const std::string& actionName, GamepadAxis axis, int gamepadId = -1, float scale = 1.0f);
    
    /**
     * Remove all bindings for an action
     * @param actionName Action name
     * @return true if any bindings were removed
     */
    bool removeActionBindings(const std::string& actionName);
    
    /**
     * Remove a specific binding
     * @param actionName Action name
     * @param type Binding type
     * @param code Binding code
     * @return true if the binding was removed
     */
    bool removeBinding(const std::string& actionName, InputBindingType type, int code);
    
    /**
     * Load input bindings from a file
     * @param filepath Path to the input bindings file
     * @return true if the bindings were loaded successfully
     */
    bool loadBindingsFromFile(const std::string& filepath);
    
    /**
     * Save input bindings to a file
     * @param filepath Path to the input bindings file
     * @return true if the bindings were saved successfully
     */
    bool saveBindingsToFile(const std::string& filepath);
    
    /**
     * Register an action callback
     * @param actionName Action name
     * @param callback Function to call when the action state changes
     * @return Callback ID for unregistering
     */
    int registerActionCallback(const std::string& actionName, 
                              const std::function<void(const InputAction&)>& callback);
    
    /**
     * Unregister an action callback
     * @param callbackId Callback ID returned from registerActionCallback
     * @return true if the callback was unregistered
     */
    bool unregisterActionCallback(int callbackId);
    
private:
    /**
     * Update input actions based on device states
     */
    void updateActions();
    
    /**
     * Update a specific action based on its bindings
     * @param action Input action to update
     * @param bindings Bindings for the action
     */
    void updateAction(InputAction& action, const std::vector<InputBinding>& bindings);
    
    // Window handle
    GLFWwindow* m_window;
    
    // Input devices
    std::shared_ptr<KeyboardDevice> m_keyboardDevice;
    std::shared_ptr<MouseDevice> m_mouseDevice;
    std::vector<std::shared_ptr<GamepadDevice>> m_gamepadDevices;
    std::vector<std::shared_ptr<IInputDevice>> m_devices;
    
    // Input actions and bindings
    std::unordered_map<std::string, InputAction> m_actions;
    std::unordered_map<std::string, std::vector<InputBinding>> m_bindings;
    
    // Action callbacks
    struct ActionCallback {
        std::string actionName;
        std::function<void(const InputAction&)> callback;
    };
    std::unordered_map<int, ActionCallback> m_actionCallbacks;
    int m_nextCallbackId;
};

} // namespace Input
} // namespace RPGEngine