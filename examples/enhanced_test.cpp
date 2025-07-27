#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#include "core/EngineCore.h"
#include "core/ISystem.h"
#include "core/Event.h"
#include "core/Types.h"

using namespace RPGEngine;

// Custom event for testing the event system
class TestEvent : public Event<TestEvent> {
public:
    TestEvent(const std::string& message) : message(message) {}
    std::string message;
};

// Simple test system for demonstration
class TestSystem : public ISystem {
public:
    TestSystem(const std::string& name) 
        : m_name(name)
        , m_initialized(false)
        , m_updateCount(0)
        , m_eventCount(0)
    {}
    
    bool initialize() override {
        std::cout << "TestSystem (" << m_name << ") initializing..." << std::endl;
        m_initialized = true;
        return true;
    }
    
    void update(float deltaTime) override {
        m_updateCount++;
        m_totalTime += deltaTime;
        
        // Print update info every 60 frames (approximately 1 second at 60 FPS)
        if (m_updateCount % 60 == 0) {
            std::cout << m_name << " - Frame " << m_updateCount 
                      << ", Delta: " << deltaTime << "s"
                      << ", Total: " << m_totalTime << "s"
                      << ", Events: " << m_eventCount << std::endl;
        }
    }
    
    void shutdown() override {
        std::cout << "TestSystem (" << m_name << ") shutting down after " 
                  << m_updateCount << " updates and " 
                  << m_eventCount << " events" << std::endl;
        m_initialized = false;
    }
    
    const std::string& getName() const override {
        return m_name;
    }
    
    bool isInitialized() const override {
        return m_initialized;
    }
    
    void handleEvent(const TestEvent& event) {
        m_eventCount++;
        std::cout << m_name << " received event: " << event.message << std::endl;
    }
    
private:
    std::string m_name;
    bool m_initialized;
    int m_updateCount;
    int m_eventCount;
    float m_totalTime = 0.0f;
};

int main() {
    std::cout << "=== RPG Engine Enhanced Test ===" << std::endl;
    
    try {
        // Create custom configuration
        EngineConfig config;
        config.targetFPS = 60.0f;
        config.frameRateLimitEnabled = true;
        config.debugMode = true;
        config.showFPS = true;
        
        // Create engine with custom config
        auto engine = std::make_unique<EngineCore>(config);
        
        // Register test systems
        auto inputSystem = std::make_shared<TestSystem>("InputSystem");
        auto renderSystem = std::make_shared<TestSystem>("RenderSystem");
        auto physicsSystem = std::make_shared<TestSystem>("PhysicsSystem");
        
        engine->addSystem(SystemType::Input, inputSystem);
        engine->addSystem(SystemType::Rendering, renderSystem);
        engine->addSystem(SystemType::Physics, physicsSystem);
        
        // Set up system dependencies
        engine->setSystemDependency(SystemType::Rendering, SystemType::Physics);
        
        std::cout << "Registered test systems with engine" << std::endl;
        
        // Subscribe to events
        auto& eventDispatcher = engine->getEventDispatcher();
        eventDispatcher.subscribe<TestEvent>([&inputSystem](const TestEvent& event) {
            inputSystem->handleEvent(event);
        });
        
        eventDispatcher.subscribe<TestEvent>([&renderSystem](const TestEvent& event) {
            renderSystem->handleEvent(event);
        });
        
        // Start the engine in a separate thread
        std::cout << "\n=== Starting Engine ===" << std::endl;
        std::thread engineThread([&engine]() {
            engine->run();
        });
        
        // Let it run for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Test pause/resume functionality
        std::cout << "\n=== Testing Pause/Resume ===" << std::endl;
        engine->pause();
        std::cout << "Engine paused, state: " << static_cast<int>(engine->getState()) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        engine->resume();
        std::cout << "Engine resumed, state: " << static_cast<int>(engine->getState()) << std::endl;
        
        // Dispatch some events
        std::cout << "\n=== Testing Event System ===" << std::endl;
        eventDispatcher.dispatch(TestEvent("Hello from main thread!"));
        
        // Queue an event for later processing
        auto event = std::make_unique<TestEvent>("Queued event");
        eventDispatcher.queueEvent(std::move(event));
        
        // Let it run a bit more
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Print performance stats
        std::cout << "\n=== Performance Stats ===" << std::endl;
        std::cout << "FPS: " << engine->getFPS() << std::endl;
        std::cout << "Average FPS: " << engine->getAverageFPS() << std::endl;
        std::cout << "Frame Count: " << engine->getFrameCount() << std::endl;
        std::cout << "Run Time: " << engine->getRunTime() << "s" << std::endl;
        
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
    
    std::cout << "RPG Engine Enhanced Test completed successfully!" << std::endl;
    return 0;
}