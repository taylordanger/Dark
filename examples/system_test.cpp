#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "systems/SystemManager.h"
#include "systems/System.h"
#include "core/Event.h"

using namespace RPGEngine;

// Custom event for testing
class TestEvent : public Event<TestEvent> {
public:
    TestEvent(const std::string& message) : message(message) {}
    std::string message;
};

// Custom system for testing
class TestSystem : public System {
public:
    TestSystem(const std::string& name, bool useFixedTimestep = false)
        : System(name)
        , m_updateCount(0)
        , m_eventCount(0)
    {
        setUseFixedTimestep(useFixedTimestep);
    }
    
protected:
    bool onInitialize() override {
        std::cout << "TestSystem '" << getName() << "' initializing..." << std::endl;
        return true;
    }
    
    void onUpdate(float deltaTime) override {
        m_updateCount++;
        m_totalTime += deltaTime;
        
        // Print update info every 60 frames (approximately 1 second at 60 FPS)
        if (m_updateCount % 60 == 0) {
            std::cout << getName() << " - Frame " << m_updateCount 
                      << ", Delta: " << deltaTime << "s"
                      << ", Total: " << m_totalTime << "s"
                      << ", Events: " << m_eventCount << std::endl;
        }
    }
    
    void onShutdown() override {
        std::cout << "TestSystem '" << getName() << "' shutting down after " 
                  << m_updateCount << " updates and " 
                  << m_eventCount << " events" << std::endl;
    }
    
public:
    void handleEvent(const TestEvent& event) {
        m_eventCount++;
        std::cout << getName() << " received event: " << event.message << std::endl;
    }
    
    int getUpdateCount() const { return m_updateCount; }
    int getEventCount() const { return m_eventCount; }
    
private:
    int m_updateCount;
    int m_eventCount;
    float m_totalTime = 0.0f;
};

// System that depends on other systems
class DependentSystem : public TestSystem {
public:
    DependentSystem(const std::string& name, SystemType dependency)
        : TestSystem(name)
    {
        // Add dependency
        addDependency(dependency);
    }
    
protected:
    bool onInitialize() override {
        std::cout << "DependentSystem '" << getName() << "' initializing..." << std::endl;
        std::cout << "  (depends on other systems)" << std::endl;
        return true;
    }
};

int main() {
    std::cout << "=== SystemManager Test ===" << std::endl;
    
    try {
        // Create system manager
        SystemManager manager;
        
        // Register systems
        auto inputSystem = std::make_shared<TestSystem>("InputSystem");
        auto physicsSystem = std::make_shared<TestSystem>("PhysicsSystem", true); // Uses fixed timestep
        auto renderSystem = std::make_shared<TestSystem>("RenderSystem");
        
        // Register a system that depends on others
        auto gameSystem = std::make_shared<DependentSystem>("GameSystem", SystemType::Input);
        
        // Register systems with manager
        manager.registerSystem(inputSystem, SystemType::Input);
        manager.registerSystem(physicsSystem, SystemType::Physics);
        manager.registerSystem(renderSystem, SystemType::Rendering);
        manager.registerSystem(gameSystem, SystemType::ECS);
        
        std::cout << "Registered " << manager.getSystemCount() << " systems" << std::endl;
        
        // Add additional dependency
        manager.addSystemDependency(SystemType::ECS, SystemType::Physics);
        
        // Subscribe to system events
        manager.getEventDispatcher().subscribe<SystemEvent>([](const SystemEvent& event) {
            std::cout << "System event: " 
                      << static_cast<int>(event.eventType) << " for system " 
                      << static_cast<int>(event.systemType) << std::endl;
        });
        
        // Initialize all systems
        std::cout << "\n=== Initializing Systems ===" << std::endl;
        if (!manager.initializeAll()) {
            std::cerr << "Failed to initialize systems" << std::endl;
            return -1;
        }
        
        // Print execution order
        std::cout << "\n=== System Execution Order ===" << std::endl;
        std::cout << "Order: ";
        for (SystemType type : manager.getExecutionOrder()) {
            std::cout << static_cast<int>(type) << " ";
        }
        std::cout << std::endl;
        
        // Test system updates
        std::cout << "\n=== Updating Systems ===" << std::endl;
        
        // Update all systems a few times
        for (int i = 0; i < 5; i++) {
            manager.updateAll(0.016f); // ~60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        // Update fixed systems with fixed timestep
        std::cout << "\n=== Updating Fixed Systems ===" << std::endl;
        for (int i = 0; i < 5; i++) {
            manager.updateAll(0.016f, SystemUpdateMode::Fixed);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        // Update variable systems with variable timestep
        std::cout << "\n=== Updating Variable Systems ===" << std::endl;
        for (int i = 0; i < 5; i++) {
            manager.updateAll(0.016f, SystemUpdateMode::Variable);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        // Test system events
        std::cout << "\n=== Testing System Events ===" << std::endl;
        
        // Disable a system
        manager.setSystemEnabled(SystemType::Rendering, false);
        std::cout << "RenderSystem enabled: " << (manager.isSystemEnabled(SystemType::Rendering) ? "yes" : "no") << std::endl;
        
        // Update all systems again
        for (int i = 0; i < 5; i++) {
            manager.updateAll(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        // Re-enable the system
        manager.setSystemEnabled(SystemType::Rendering, true);
        std::cout << "RenderSystem enabled: " << (manager.isSystemEnabled(SystemType::Rendering) ? "yes" : "no") << std::endl;
        
        // Dispatch a custom event
        TestEvent event("Hello from main!");
        for (SystemType type : manager.getExecutionOrder()) {
            auto system = manager.getSystem<TestSystem>(type);
            if (system) {
                system->handleEvent(event);
            }
        }
        
        // Shutdown all systems
        std::cout << "\n=== Shutting Down Systems ===" << std::endl;
        manager.shutdownAll();
        
        std::cout << "SystemManager test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}