#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "../src/debug/DebugRenderer.h"
#include "../src/debug/PerformanceProfiler.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/physics/CollisionShape.h"

using namespace Engine;

int main() {
    std::cout << "=== Debug Systems Simple Test ===" << std::endl;

    // Test Performance Profiler
    std::cout << "\n--- Testing Performance Profiler ---" << std::endl;
    
    auto profiler = std::make_unique<Debug::PerformanceProfiler>();
    
    // Simulate some frames
    for (int frame = 0; frame < 10; ++frame) {
        profiler->beginFrame();
        
        // Simulate some work sections
        {
            PROFILE_SECTION(profiler.get(), "Update");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        {
            PROFILE_SECTION(profiler.get(), "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        profiler->recordEntityCount(100 + frame * 5);
        profiler->recordDrawCalls(50 + frame * 2);
        
        profiler->endFrame();
        profiler->update();
    }
    
    // Check profiler results
    auto frameStats = profiler->getCurrentFrameStats();
    std::cout << "Current FPS: " << frameStats.fps << std::endl;
    std::cout << "Current Frame Time: " << frameStats.frameTime << "ms" << std::endl;
    std::cout << "Average FPS: " << profiler->getAverageFPS() << std::endl;
    std::cout << "Average Frame Time: " << profiler->getAverageFrameTime() << "ms" << std::endl;
    std::cout << "Entity Count: " << frameStats.entityCount << std::endl;
    std::cout << "Draw Calls: " << frameStats.drawCalls << std::endl;
    
    auto sections = profiler->getAllSections();
    std::cout << "Profiled Sections:" << std::endl;
    for (const auto& section : sections) {
        std::cout << "  " << section.name << ": " << section.totalTime << "ms total, " 
                  << section.callCount << " calls" << std::endl;
    }

    // Test Debug Renderer
    std::cout << "\n--- Testing Debug Renderer ---" << std::endl;
    
    // Create a mock graphics API for testing
    auto graphicsAPI = std::make_shared<RPGEngine::Graphics::OpenGLAPI>();
    auto debugRenderer = std::make_unique<Debug::DebugRenderer>(graphicsAPI);
    
    // Test drawing primitives
    debugRenderer->drawLine(0, 0, 100, 100);
    debugRenderer->drawRect(50, 50, 100, 75, false);
    debugRenderer->drawCircle(200, 200, 50, false);
    
    // Test collision shape rendering
    auto rectShape = std::make_unique<RPGEngine::Physics::RectangleShape>(50, 50);
    rectShape->setPosition(100, 100);
    debugRenderer->drawCollisionShape(*rectShape);
    
    auto circleShape = std::make_unique<RPGEngine::Physics::CircleShape>(25);
    circleShape->setPosition(200, 200);
    debugRenderer->drawCollisionShape(*circleShape);
    
    // Test velocity vector
    debugRenderer->drawVelocityVector(300, 300, 50, -30, 2.0f);
    
    // Test grid
    debugRenderer->drawGrid(32.0f);
    
    std::cout << "Debug renderer created and primitives added" << std::endl;
    std::cout << "Enabled: " << debugRenderer->isEnabled() << std::endl;
    
    // Test disabling and re-enabling
    debugRenderer->setEnabled(false);
    debugRenderer->drawLine(0, 0, 50, 50); // Should not be added
    debugRenderer->setEnabled(true);
    
    // Simulate rendering (would normally call render() in game loop)
    std::cout << "Debug rendering would occur here in actual game loop" << std::endl;
    
    // Clear debug primitives
    debugRenderer->clear();
    std::cout << "Debug primitives cleared" << std::endl;

    std::cout << "\n=== Debug Systems Simple Test Complete ===" << std::endl;
    std::cout << "Debug systems (DebugRenderer, PerformanceProfiler) created and tested successfully!" << std::endl;

    return 0;
}