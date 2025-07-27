#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <iomanip>
#include "../src/graphics/IGraphicsAPI.h"
#include "../src/input/InputManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Input;
using namespace RPGEngine::Graphics;

// Simple class to display input state
class InputDisplay {
public:
    InputDisplay(std::shared_ptr<InputManager> inputManager)
        : m_inputManager(inputManager)
    {
    }
    
    void update() {
        // Clear console
        std::cout << "\033[2J\033[1;1H";
        
        // Display keyboard state
        displayKeyboardState();
        
        // Display mouse state
        displayMouseState();
        
        // Display gamepad state
        displayGamepadState();
        
        // Display action state
        displayActionState();
    }
    
private:
    void displayKeyboardState() {
        auto keyboard = m_inputManager->getKeyboardDevice();
        if (!keyboard) {
            return;
        }
        
        std::cout << "=== Keyboard State ===" << std::endl;
        std::cout << "Shift: " << (keyboard->isShiftPressed() ? "Pressed" : "Released") << std::endl;
        std::cout << "Control: " << (keyboard->isControlPressed() ? "Pressed" : "Released") << std::endl;
        std::cout << "Alt: " << (keyboard->isAltPressed() ? "Pressed" : "Released") << std::endl;
        std::cout << "Super: " << (keyboard->isSuperPressed() ? "Pressed" : "Released") << std::endl;
        
        std::cout << "\nPressed Keys: ";
        
        // Check common keys
        std::vector<KeyCode> keysToCheck = {
            KeyCode::W, KeyCode::A, KeyCode::S, KeyCode::D,
            KeyCode::Space, KeyCode::Escape, KeyCode::Enter,
            KeyCode::Up, KeyCode::Down, KeyCode::Left, KeyCode::Right
        };
        
        bool anyKeyPressed = false;
        for (KeyCode key : keysToCheck) {
            if (keyboard->isKeyPressed(key)) {
                std::cout << static_cast<int>(key) << " ";
                anyKeyPressed = true;
            }
        }
        
        if (!anyKeyPressed) {
            std::cout << "None";
        }
        
        std::cout << std::endl << std::endl;
    }
    
    void displayMouseState() {
        auto mouse = m_inputManager->getMouseDevice();
        if (!mouse) {
            return;
        }
        
        std::cout << "=== Mouse State ===" << std::endl;
        
        // Position
        double x, y;
        mouse->getPosition(x, y);
        std::cout << "Position: (" << std::fixed << std::setprecision(1) << x << ", " << y << ")" << std::endl;
        
        // Delta
        double deltaX, deltaY;
        mouse->getDelta(deltaX, deltaY);
        std::cout << "Delta: (" << std::fixed << std::setprecision(1) << deltaX << ", " << deltaY << ")" << std::endl;
        
        // Scroll
        double scrollX, scrollY;
        mouse->getScrollOffset(scrollX, scrollY);
        std::cout << "Scroll: (" << std::fixed << std::setprecision(1) << scrollX << ", " << scrollY << ")" << std::endl;
        
        // Buttons
        std::cout << "Left Button: " << (mouse->isButtonPressed(MouseButton::Left) ? "Pressed" : "Released") << std::endl;
        std::cout << "Right Button: " << (mouse->isButtonPressed(MouseButton::Right) ? "Pressed" : "Released") << std::endl;
        std::cout << "Middle Button: " << (mouse->isButtonPressed(MouseButton::Middle) ? "Pressed" : "Released") << std::endl;
        
        std::cout << std::endl;
    }
    
    void displayGamepadState() {
        const auto& gamepads = m_inputManager->getGamepadDevices();
        if (gamepads.empty()) {
            std::cout << "=== Gamepad State ===" << std::endl;
            std::cout << "No gamepads detected" << std::endl << std::endl;
            return;
        }
        
        std::cout << "=== Gamepad State ===" << std::endl;
        std::cout << "Connected Gamepads: " << m_inputManager->getConnectedGamepadCount() << std::endl;
        
        for (const auto& gamepad : gamepads) {
            if (!gamepad->isConnected()) {
                continue;
            }
            
            std::cout << "\nGamepad " << gamepad->getGamepadId() << " (" << gamepad->getName() << ")" << std::endl;
            
            // Buttons
            std::cout << "A: " << (gamepad->isButtonPressed(GamepadButton::A) ? "Pressed" : "Released") << std::endl;
            std::cout << "B: " << (gamepad->isButtonPressed(GamepadButton::B) ? "Pressed" : "Released") << std::endl;
            std::cout << "X: " << (gamepad->isButtonPressed(GamepadButton::X) ? "Pressed" : "Released") << std::endl;
            std::cout << "Y: " << (gamepad->isButtonPressed(GamepadButton::Y) ? "Pressed" : "Released") << std::endl;
            
            // D-Pad
            std::cout << "D-Pad Up: " << (gamepad->isButtonPressed(GamepadButton::DPadUp) ? "Pressed" : "Released") << std::endl;
            std::cout << "D-Pad Down: " << (gamepad->isButtonPressed(GamepadButton::DPadDown) ? "Pressed" : "Released") << std::endl;
            std::cout << "D-Pad Left: " << (gamepad->isButtonPressed(GamepadButton::DPadLeft) ? "Pressed" : "Released") << std::endl;
            std::cout << "D-Pad Right: " << (gamepad->isButtonPressed(GamepadButton::DPadRight) ? "Pressed" : "Released") << std::endl;
            
            // Axes
            std::cout << "Left Stick: (" 
                      << std::fixed << std::setprecision(2) << gamepad->getAxisValue(GamepadAxis::LeftX) << ", " 
                      << gamepad->getAxisValue(GamepadAxis::LeftY) << ")" << std::endl;
            std::cout << "Right Stick: (" 
                      << std::fixed << std::setprecision(2) << gamepad->getAxisValue(GamepadAxis::RightX) << ", " 
                      << gamepad->getAxisValue(GamepadAxis::RightY) << ")" << std::endl;
            std::cout << "Triggers: (L: " 
                      << std::fixed << std::setprecision(2) << gamepad->getAxisValue(GamepadAxis::LeftTrigger) << ", R: " 
                      << gamepad->getAxisValue(GamepadAxis::RightTrigger) << ")" << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    void displayActionState() {
        std::cout << "=== Input Actions ===" << std::endl;
        
        // Display common actions
        displayAction("MoveUp");
        displayAction("MoveDown");
        displayAction("MoveLeft");
        displayAction("MoveRight");
        displayAction("Jump");
        displayAction("Fire");
        displayAction("MouseLook");
        displayAction("GamepadLook");
        
        std::cout << std::endl;
    }
    
    void displayAction(const std::string& actionName) {
        if (!m_inputManager->hasAction(actionName)) {
            return;
        }
        
        bool active = m_inputManager->isActionActive(actionName);
        bool justActivated = m_inputManager->isActionJustActivated(actionName);
        bool justDeactivated = m_inputManager->isActionJustDeactivated(actionName);
        float value = m_inputManager->getActionValue(actionName);
        
        std::cout << actionName << ": " 
                  << (active ? "Active" : "Inactive")
                  << (justActivated ? " (Just Activated)" : "")
                  << (justDeactivated ? " (Just Deactivated)" : "")
                  << " - Value: " << std::fixed << std::setprecision(2) << value
                  << std::endl;
    }
    
    std::shared_ptr<InputManager> m_inputManager;
};

int main() {
    std::cout << "=== Input System Test ===\n" << std::endl;
    
    // Create graphics API (needed for window)
    auto graphicsAPI = createGraphicsAPI();
    
    // Initialize graphics API
    if (!graphicsAPI->initialize(800, 600, "RPG Engine Input Test", false)) {
        std::cerr << "Failed to initialize graphics API" << std::endl;
        return -1;
    }
    
    // Get GLFW window handle
    GLFWwindow* window = static_cast<GLFWwindow*>(glfwGetCurrentContext());
    
    // Create input manager
    auto inputManager = std::make_shared<InputManager>(window);
    if (!inputManager->initialize()) {
        std::cerr << "Failed to initialize input manager" << std::endl;
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create input display
    InputDisplay inputDisplay(inputManager);
    
    // Set up input actions
    inputManager->createAction("MoveUp");
    inputManager->createAction("MoveDown");
    inputManager->createAction("MoveLeft");
    inputManager->createAction("MoveRight");
    inputManager->createAction("Jump");
    inputManager->createAction("Fire");
    inputManager->createAction("MouseLook");
    inputManager->createAction("GamepadLook");
    
    // Bind keys to actions
    inputManager->bindKeyToAction("MoveUp", KeyCode::W);
    inputManager->bindKeyToAction("MoveUp", KeyCode::Up);
    
    inputManager->bindKeyToAction("MoveDown", KeyCode::S);
    inputManager->bindKeyToAction("MoveDown", KeyCode::Down);
    
    inputManager->bindKeyToAction("MoveLeft", KeyCode::A);
    inputManager->bindKeyToAction("MoveLeft", KeyCode::Left);
    
    inputManager->bindKeyToAction("MoveRight", KeyCode::D);
    inputManager->bindKeyToAction("MoveRight", KeyCode::Right);
    
    inputManager->bindKeyToAction("Jump", KeyCode::Space);
    
    inputManager->bindMouseButtonToAction("Fire", MouseButton::Left);
    
    inputManager->bindMouseAxisToAction("MouseLook", 0); // X axis
    
    // Bind gamepad controls
    inputManager->bindGamepadButtonToAction("MoveUp", GamepadButton::DPadUp);
    inputManager->bindGamepadButtonToAction("MoveDown", GamepadButton::DPadDown);
    inputManager->bindGamepadButtonToAction("MoveLeft", GamepadButton::DPadLeft);
    inputManager->bindGamepadButtonToAction("MoveRight", GamepadButton::DPadRight);
    
    inputManager->bindGamepadButtonToAction("Jump", GamepadButton::A);
    inputManager->bindGamepadButtonToAction("Fire", GamepadButton::X);
    
    inputManager->bindGamepadAxisToAction("GamepadLook", GamepadAxis::RightX);
    
    // Register action callback
    inputManager->registerActionCallback("Jump", [](const InputAction& action) {
        if (action.justActivated) {
            std::cout << "Jump action activated!" << std::endl;
        } else if (action.justDeactivated) {
            std::cout << "Jump action deactivated!" << std::endl;
        }
    });
    
    // Main loop
    float deltaTime = 0.016f;
    while (!graphicsAPI->shouldClose()) {
        // Begin frame
        graphicsAPI->beginFrame();
        graphicsAPI->clear(0.2f, 0.3f, 0.3f, 1.0f);
        
        // Update input manager
        inputManager->update(deltaTime);
        
        // Display input state
        inputDisplay.update();
        
        // End frame
        graphicsAPI->endFrame();
    }
    
    // Clean up
    inputManager->shutdown();
    graphicsAPI->shutdown();
    
    std::cout << "\nInput System Test completed successfully!" << std::endl;
    return 0;
}