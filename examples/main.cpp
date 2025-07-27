#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "core/EngineCore.h"
#include "core/ISystem.h"
#include "core/Types.h"

using namespace RPGEngine;

// Simple test system for demonstration
class TestSystem : public ISystem {
public:
    TestSystem(const std::string& name) : m_name(name), m_initialized(false), m_updateCount(0) {}
    
    bool initialize() override {
        std::cout << "TestSystem (" << m_name << ") initializing..." << std::endl;
        m_initialized = true;
        return true;
    }
    
    void update(float deltaTime) override {
        m_updateCount++;
        
        // Print update info every 60 frames (approximately 1 second at 60 FPS)
        if (m_updateCount % 60 == 0) {
            std::cout << m_name << " - Frame " << m_updateCount 
                      << ", Delta: " << deltaTime << "s" << std::endl;
        }
    }
    
    void shutdown() override {
        std::cout << "TestSystem (" << m_name << ") shutting down after " 
                  << m_updateCount << " updates" << std::endl;
        m_initialized = false;
    }
    
    const std::string& getName() const override {
        return m_name;
    }
    
    bool isInitialized() const override {
        return m_initialized;
    }
    
private:
    std::string m_name;
    bool m_initialized;
    int m_updateCount;
};

int main() {
    std::cout << "=== RPG Engine Framework Test ===" << std::endl;
    
    try {
        // Create engine instance
        auto engine = std::make_unique<EngineCore>();
        
        // Register test systems
        auto inputSystem = std::make_shared<TestSystem>("InputSystem");
        auto renderSystem = std::make_shared<TestSystem>("RenderSystem");
        auto physicsSystem = std::make_shared<TestSystem>("PhysicsSystem");
        
        engine->addSystem(SystemType::Input, inputSystem);
        engine->addSystem(SystemType::Rendering, renderSystem);
        engine->addSystem(SystemType::Physics, physicsSystem);
        
        std::cout << "Registered test systems with engine" << std::endl;
        
        // Test system retrieval
        auto retrievedSystem = engine->getSystem(SystemType::Input);
        if (retrievedSystem) {
            std::cout << "Successfully retrieved input system: " 
                      << retrievedSystem->getName() << std::endl;
        }
        
        // Start the engine in a separate thread for testing
        std::cout << "\n=== Starting Engine ===" << std::endl;
        std::thread engineThread([&engine]() {
            engine->run();
        });
        
        // Let it run for a few seconds
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // Request shutdown
        std::cout << "\n=== Requesting Engine Shutdown ===" << std::endl;
        engine->requestShutdown();
        
        // Wait for engine thread to finish
        if (engineThread.joinable()) {
            engineThread.join();
        }
        
        std::cout << "Engine thread completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "RPG Engine Framework test completed successfully!" << std::endl;
    return 0;
}