#pragma once

#include <string>
#include <memory>

namespace RPGEngine {
namespace Input {

/**
 * Input device type enumeration
 */
enum class InputDeviceType {
    Keyboard,
    Mouse,
    Gamepad,
    Touch,
    Custom
};

/**
 * Input device interface
 * Base interface for all input devices
 */
class IInputDevice {
public:
    /**
     * Virtual destructor
     */
    virtual ~IInputDevice() = default;
    
    /**
     * Initialize the input device
     * @return true if initialization was successful
     */
    virtual bool initialize() = 0;
    
    /**
     * Update the input device state
     */
    virtual void update() = 0;
    
    /**
     * Shutdown the input device
     */
    virtual void shutdown() = 0;
    
    /**
     * Get the input device type
     * @return Input device type
     */
    virtual InputDeviceType getType() const = 0;
    
    /**
     * Get the input device name
     * @return Input device name
     */
    virtual const std::string& getName() const = 0;
    
    /**
     * Check if the input device is connected
     * @return true if the input device is connected
     */
    virtual bool isConnected() const = 0;
    
    /**
     * Check if the input device is initialized
     * @return true if the input device is initialized
     */
    virtual bool isInitialized() const = 0;
};

} // namespace Input
} // namespace RPGEngine