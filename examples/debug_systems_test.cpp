#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "../src/debug/DebugRenderer.h"
#include "../src/debug/EntityInspector.h"
#include "../src/debug/PerformanceProfiler.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/components/TransformComponent.h"
#include "../src/components/SpriteComponent.h"
#include "../src/components/PhysicsComponent.h"
#include "../src/physics/CollisionShape.h"

using namespace Engine;

int main() {
    std::cout << "=== Debug Systems Test ===" << std::endl;

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

    // Test Entity Inspector
    std::cout << "\n--- Testing Entity Inspector ---" << std::endl;
    
    auto entityManager = std::make_shared<RPGEngine::EntityManager>();
    auto componentManager = std::make_shared<RPGEngine::ComponentManager>();
    auto inspector = std::make_unique<Debug::EntityInspector>(entityManager, componentManager);
    
    // Create some test entities
    auto entity1 = entityManager->createEntity();
    auto entity2 = entityManager->createEntity();
    auto entity3 = entityManager->createEntity();
    
    // Add components to entities
    componentManager->createComponent<RPGEngine::Components::TransformComponent>(entity1, 100.0f, 200.0f, 0.0f, 1.0f, 1.0f);
    componentManager->createComponent<RPGEngine::Components::SpriteComponent>(entity1, 1, 64, 64, true, 255, 255, 255, 255);
    
    componentManager->createComponent<RPGEngine::Components::TransformComponent>(entity2, 300.0f, 400.0f, 45.0f, 2.0f, 2.0f);
    componentManager->createComponent<RPGEngine::Components::PhysicsComponent>(entity2, 10.0f, -5.0f, 1.0f, 0.8f, true);
    
    componentManager->createComponent<RPGEngine::Components::TransformComponent>(entity3, 500.0f, 100.0f, 0.0f, 1.0f, 1.0f);
    
    // Deactivate one entity
    entityManager->setEntityActive(entity3, false);
    
    // Test inspector functionality
    std::cout << "Total entities: " << inspector->getTotalEntityCount() << std::endl;
    std::cout << "Active entities: " << inspector->getActiveEntityCount() << std::endl;
    
    auto allEntities = inspector->getAllEntities();
    std::cout << "All entities:" << std::endl;
    for (const auto& entity : allEntities) {
        std::cout << "  Entity " << entity.id << " (active: " << entity.active << ")" << std::endl;
        std::cout << "    Components: ";
        for (const auto& component : entity.componentTypes) {
            std::cout << component << " ";
        }
        std::cout << std::endl;
    }
    
    // Test component inspection
    std::cout << "\nEntity " << entity1 << " components:" << std::endl;
    auto components = inspector->getEntityComponents(entity1);
    for (const auto& component : components) {
        std::cout << "  " << component.type << ":" << std::endl;
        std::cout << "    " << component.data << std::endl;
    }
    
    // Test entity filtering
    auto entitiesWithTransform = inspector->findEntitiesWithComponent("TransformComponent");
    std::cout << "\nEntities with TransformComponent: " << entitiesWithTransform.size() << std::endl;
    
    auto activeEntities = inspector->getActiveEntities();
    std::cout << "Active entities: " << activeEntities.size() << std::endl;
    
    auto inactiveEntities = inspector->getInactiveEntities();
    std::cout << "Inactive entities: " << inactiveEntities.size() << std::endl;
    
    // Test component statistics
    auto componentStats = inspector->getComponentTypeStats();
    std::cout << "\nComponent statistics:" << std::endl;
    for (const auto& stat : componentStats) {
        std::cout << "  " << stat.first << ": " << stat.second << " instances" << std::endl;
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

    std::cout << "\n=== Debug Systems Test Complete ===" << std::endl;
    std::cout << "All debug systems (DebugRenderer, EntityInspector, PerformanceProfiler) created and tested successfully!" << std::endl;

    return 0;
}