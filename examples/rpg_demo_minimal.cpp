#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

// Core Engine
#include "../src/core/EngineCore.h"

// Components
#include "../src/components/ComponentManager.h"
#include "../src/components/StatsComponent.h"
#include "../src/components/InventoryComponent.h"

// Graphics
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/Camera.h"

// Entities
#include "../src/entities/EntityManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Components;

/**
 * Minimal RPG Demo
 * 
 * This demo showcases the core working features:
 * - Entity-Component-System architecture
 * - Basic graphics initialization
 * - Component management (Stats, Inventory)
 * - Simple game loop
 */
class RPGDemoMinimal {
private:
    // Core Engine Components
    std::shared_ptr<EngineCore> engine;
    std::shared_ptr<EntityManager> entityManager;
    std::shared_ptr<ComponentManager> componentManager;
    
    // Graphics
    std::shared_ptr<OpenGLAPI> graphicsAPI;
    std::shared_ptr<Camera> camera;
    
    // Game Entities
    Entity playerEntity;
    std::vector<Entity> npcEntities;
    
    // Game State
    bool isRunning;
    float gameTime;
    
public:
    RPGDemoMinimal() : isRunning(false), gameTime(0.0f) {}
    
    bool initialize() {
        std::cout << "=== RPG Engine Minimal Demo ===" << std::endl;
        std::cout << "Initializing minimal RPG demo..." << std::endl;
        
        // Initialize core systems
        engine = std::make_shared<EngineCore>();
        if (!engine->initialize()) {
            std::cerr << "Failed to initialize core engine" << std::endl;
            return false;
        }
        
        // Entity and component management
        entityManager = std::make_shared<EntityManager>();
        componentManager = std::make_shared<ComponentManager>();
        
        // Graphics API
        graphicsAPI = std::make_shared<OpenGLAPI>();
        if (!graphicsAPI->initialize(800, 600, "RPG Engine Minimal Demo", false)) {
            std::cerr << "Failed to initialize graphics" << std::endl;
            return false;
        }
        
        // Camera
        camera = std::make_shared<Camera>();
        camera->setViewportSize(800, 600);
        camera->setPosition(0, 0);
        
        // Create demo content
        createDemoContent();
        
        std::cout << "✅ RPG Minimal Demo initialized successfully!" << std::endl;
        return true;
    }
    
    void run() {
        isRunning = true;
        
        std::cout << "\n🎮 Starting RPG Minimal Demo..." << std::endl;
        std::cout << "This demo will run for 30 seconds and demonstrate:" << std::endl;
        std::cout << "- Entity creation and management" << std::endl;
        std::cout << "- Component attachment and queries" << std::endl;
        std::cout << "- Basic graphics rendering loop" << std::endl;
        std::cout << "- Performance monitoring" << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Main game loop
        while (isRunning && !graphicsAPI->shouldClose()) {
            float deltaTime = calculateDeltaTime();
            gameTime += deltaTime;
            
            // Run for 30 seconds then exit
            if (gameTime > 30.0f) {
                isRunning = false;
                break;
            }
            
            // Update and demonstrate features
            update(deltaTime);
            
            // Render frame
            render();
            
            // Poll events
            graphicsAPI->pollEvents();
            
            // Show progress every 5 seconds
            if (static_cast<int>(gameTime) % 5 == 0) {
                static int lastSecond = -1;
                int currentSecond = static_cast<int>(gameTime);
                if (currentSecond != lastSecond) {
                    lastSecond = currentSecond;
                    showProgress();
                }
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "\n🎮 RPG Minimal Demo completed!" << std::endl;
        std::cout << "Total runtime: " << duration.count() << "ms" << std::endl;
    }
    
    void shutdown() {
        std::cout << "Shutting down RPG Minimal Demo..." << std::endl;
        
        // Cleanup
        if (graphicsAPI) graphicsAPI->shutdown();
        if (engine) engine->shutdown();
        
        std::cout << "✅ RPG Minimal Demo shutdown complete." << std::endl;
    }

private:
    void createDemoContent() {
        std::cout << "Creating demo content..." << std::endl;
        
        // Create player
        playerEntity = entityManager->createEntity();
        
        // Add stats component
        auto playerStats = std::make_shared<StatsComponent>(playerEntity);
        playerStats->setLevel(1);
        componentManager->addComponent(playerEntity, playerStats);
        
        // Add inventory component
        auto playerInventory = std::make_shared<InventoryComponent>(playerEntity);
        playerInventory->setCapacity(20);
        componentManager->addComponent(playerEntity, playerInventory);
        
        // Create some NPCs
        for (int i = 0; i < 3; ++i) {
            Entity npc = entityManager->createEntity();
            
            auto npcStats = std::make_shared<StatsComponent>(npc);
            npcStats->setLevel(i + 1);
            componentManager->addComponent(npc, npcStats);
            
            npcEntities.push_back(npc);
        }
        
        std::cout << "  ✅ Created player and " << npcEntities.size() << " NPCs" << std::endl;
    }
    
    float calculateDeltaTime() {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        return std::min(deltaTime, 0.016f); // Cap at 60 FPS
    }
    
    void update(float deltaTime) {
        // Simulate some game logic
        
        // Every 10 seconds, level up the player
        if (static_cast<int>(gameTime) % 10 == 0) {
            static int lastLevelUp = -1;
            int currentCheck = static_cast<int>(gameTime);
            if (currentCheck != lastLevelUp && currentCheck > 0) {
                lastLevelUp = currentCheck;
                levelUpPlayer();
            }
        }
        
        // Every 7 seconds, show entity information
        if (static_cast<int>(gameTime) % 7 == 0) {
            static int lastInfo = -1;
            int currentCheck = static_cast<int>(gameTime);
            if (currentCheck != lastInfo && currentCheck > 0) {
                lastInfo = currentCheck;
                showEntityInfo();
            }
        }
    }
    
    void levelUpPlayer() {
        auto playerStats = componentManager->getComponent<StatsComponent>(playerEntity);
        if (playerStats) {
            int currentLevel = playerStats->getLevel();
            playerStats->setLevel(currentLevel + 1);
            std::cout << "🎉 Player leveled up to level " << playerStats->getLevel() << "!" << std::endl;
        }
    }
    
    void showEntityInfo() {
        std::cout << "\n📊 Entity Information:" << std::endl;
        std::cout << "  Total entities: " << entityManager->getEntityCount() << std::endl;
        
        // Show player stats
        auto playerStats = componentManager->getComponent<StatsComponent>(playerEntity);
        auto playerInventory = componentManager->getComponent<InventoryComponent>(playerEntity);
        
        if (playerStats) {
            std::cout << "  Player Level: " << playerStats->getLevel() << std::endl;
        }
        
        if (playerInventory) {
            std::cout << "  Player Inventory Capacity: " << playerInventory->getCapacity() << std::endl;
        }
        
        // Show NPC count
        std::cout << "  NPCs created: " << npcEntities.size() << std::endl;
    }
    
    void showProgress() {
        int secondsRemaining = 30 - static_cast<int>(gameTime);
        std::cout << "⏱️  Demo running... " << secondsRemaining << " seconds remaining" << std::endl;
    }
    
    void render() {
        graphicsAPI->beginFrame();
        
        // Cycle through colors based on time
        float r = 0.2f + 0.3f * sin(gameTime * 0.5f);
        float g = 0.3f + 0.3f * sin(gameTime * 0.7f);
        float b = 0.8f + 0.2f * sin(gameTime * 0.3f);
        
        graphicsAPI->clear(r, g, b, 1.0f);
        
        // In a full implementation, we would render sprites here
        // For now, just the animated background color shows the render loop is working
        
        graphicsAPI->endFrame();
    }
};

int main() {
    RPGDemoMinimal demo;
    
    if (!demo.initialize()) {
        std::cerr << "Failed to initialize RPG Minimal Demo" << std::endl;
        return -1;
    }
    
    demo.run();
    demo.shutdown();
    
    return 0;
}