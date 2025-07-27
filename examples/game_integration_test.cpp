#include <iostream>
#include <memory>
#include <vector>

// Core systems
#include "../src/core/EngineCore.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"

// Graphics
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/graphics/ShaderManager.h"
#include "../src/graphics/Camera.h"
#include "../src/graphics/Sprite.h"

// Game components
#include "../src/components/StatsComponent.h"
#include "../src/components/InventoryComponent.h"

// Input
#include "../src/input/InputManager.h"

// Scene management
#include "../src/scene/SceneManager.h"
#include "../src/scene/GameScene.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Components;

/**
 * Game Integration Test
 * Tests the integration of graphics with core game systems
 */
class GameIntegrationTest {
public:
    GameIntegrationTest() 
        : m_running(true)
        , m_frameCount(0)
    {}
    
    bool initialize() {
        std::cout << "=== RPG Engine Game Integration Test ===" << std::endl;
        
        // 1. Initialize Graphics
        std::cout << "1. Initializing Graphics System..." << std::endl;
        m_graphicsAPI = std::make_shared<OpenGLAPI>();
        if (!m_graphicsAPI->initialize(1024, 768, "RPG Engine Integration Test", false)) {
            std::cerr << "❌ Failed to initialize graphics" << std::endl;
            return false;
        }
        std::cout << "  ✅ Graphics initialized: " << m_graphicsAPI->getAPIName() << std::endl;
        
        // 2. Initialize Shader Manager
        m_shaderManager = std::make_shared<ShaderManager>(m_graphicsAPI);
        if (!m_shaderManager->initialize()) {
            std::cerr << "❌ Failed to initialize shader manager" << std::endl;
            return false;
        }
        std::cout << "  ✅ Shader manager initialized" << std::endl;
        
        // 3. Initialize Entity System
        std::cout << "2. Initializing Entity System..." << std::endl;
        m_entityManager = std::make_shared<EntityManager>();
        m_componentManager = std::make_shared<ComponentManager>();
        std::cout << "  ✅ Entity and Component managers initialized" << std::endl;
        
        // 4. Initialize Camera
        std::cout << "3. Setting up Camera..." << std::endl;
        m_camera = std::make_shared<Camera>();
        m_camera->setPosition(0.0f, 0.0f);
        m_camera->setViewportSize(1024, 768);
        m_camera->setZoom(1.0f);
        std::cout << "  ✅ Camera configured" << std::endl;
        
        // 5. Create some test entities
        std::cout << "4. Creating Test Entities..." << std::endl;
        createTestEntities();
        
        return true;
    }
    
    void createTestEntities() {
        // Create a player entity
        Entity player = m_entityManager->createEntity();
        std::cout << "  ✅ Created player entity: " << static_cast<uint32_t>(player) << std::endl;
        
        // Add stats component to player
        auto playerStats = std::make_shared<StatsComponent>(player);
        playerStats->setBaseMaxHP(100);
        playerStats->setCurrentHP(100);
        playerStats->setBaseMaxMP(50);
        playerStats->setCurrentMP(50);
        playerStats->setLevel(1);
        
        m_componentManager->addComponent(player, playerStats);
        std::cout << "    ✅ Added stats component (HP: " << playerStats->getCurrentHP() << "/" << playerStats->getMaxHP() << ")" << std::endl;
        
        // Add inventory component to player
        auto playerInventory = std::make_shared<InventoryComponent>(player);
        playerInventory->setCapacity(20);
        
        m_componentManager->addComponent(player, playerInventory);
        std::cout << "    ✅ Added inventory component (Capacity: " << playerInventory->getCapacity() << ")" << std::endl;
        
        // Create some NPCs
        for (int i = 0; i < 3; ++i) {
            Entity npc = m_entityManager->createEntity();
            
            auto npcStats = std::make_shared<StatsComponent>(npc);
            npcStats->setBaseMaxHP(50 + i * 10);
            npcStats->setCurrentHP(50 + i * 10);
            npcStats->setLevel(1 + i);
            
            m_componentManager->addComponent(npc, npcStats);
            
            m_testEntities.push_back(npc);
            std::cout << "    ✅ Created NPC " << (i+1) << " (Entity: " << static_cast<uint32_t>(npc) << ", HP: " << npcStats->getCurrentHP() << ")" << std::endl;
        }
        
        m_playerEntity = player;
    }
    
    void run() {
        std::cout << "5. Starting Game Loop..." << std::endl;
        
        const int maxFrames = 60; // Run for 60 frames (about 1 second at 60fps)
        
        while (m_running && !m_graphicsAPI->shouldClose() && m_frameCount < maxFrames) {
            update();
            render();
            m_frameCount++;
            
            // Print status every 20 frames
            if (m_frameCount % 20 == 0) {
                std::cout << "  Frame " << m_frameCount << " - Systems running..." << std::endl;
            }
        }
        
        std::cout << "  ✅ Game loop completed (" << m_frameCount << " frames)" << std::endl;
    }
    
    void update() {
        // Poll input events
        m_graphicsAPI->pollEvents();
        
        // Update entity systems (simulate some game logic)
        if (m_frameCount % 30 == 0) { // Every 30 frames
            updateGameLogic();
        }
    }
    
    void updateGameLogic() {
        // Simulate some game events
        if (m_componentManager->hasComponent<StatsComponent>(m_playerEntity)) {
            auto playerStats = m_componentManager->getComponent<StatsComponent>(m_playerEntity);
            
            // Simulate gaining experience
            playerStats->addExperience(10);
            
            // Check for level up (simplified)
            if (playerStats->getLevel() < 5) {
                int newLevel = playerStats->getLevel() + 1;
                playerStats->setLevel(newLevel);
                playerStats->setBaseMaxHP(playerStats->getBaseMaxHP() + 20);
                playerStats->setCurrentHP(playerStats->getMaxHP());
                
                std::cout << "    🎉 Player leveled up! Level: " << playerStats->getLevel() 
                         << ", HP: " << playerStats->getCurrentHP() << "/" << playerStats->getMaxHP() << std::endl;
            }
        }
    }
    
    void render() {
        // Begin frame
        m_graphicsAPI->beginFrame();
        
        // Clear with a nice blue color
        m_graphicsAPI->clear(0.2f, 0.3f, 0.8f, 1.0f);
        
        // Render some visual feedback
        renderGameWorld();
        
        // End frame
        m_graphicsAPI->endFrame();
    }
    
    void renderGameWorld() {
        // For now, just change the clear color based on player level
        if (m_componentManager->hasComponent<StatsComponent>(m_playerEntity)) {
            auto playerStats = m_componentManager->getComponent<StatsComponent>(m_playerEntity);
            int level = playerStats->getLevel();
            
            // Change background color based on level
            float r = 0.2f + (level * 0.1f);
            float g = 0.3f + (level * 0.05f);
            float b = 0.8f - (level * 0.1f);
            
            m_graphicsAPI->clear(r, g, b, 1.0f);
        }
    }
    
    void shutdown() {
        std::cout << "6. Shutting down..." << std::endl;
        
        // Clean up entities
        for (Entity entity : m_testEntities) {
            m_entityManager->destroyEntity(entity);
        }
        if (m_playerEntity != 0) {
            m_entityManager->destroyEntity(m_playerEntity);
        }
        
        // Clean up systems
        m_componentManager.reset();
        m_entityManager.reset();
        m_camera.reset();
        
        if (m_shaderManager) {
            m_shaderManager->shutdown();
        }
        
        if (m_graphicsAPI) {
            m_graphicsAPI->shutdown();
        }
        
        std::cout << "  ✅ Cleanup completed" << std::endl;
    }
    
    void printSystemStatus() {
        std::cout << "\n=== System Status ===" << std::endl;
        
        // Entity count
        std::cout << "Entities: " << (m_testEntities.size() + 1) << " active" << std::endl;
        
        // Player status
        if (m_componentManager->hasComponent<StatsComponent>(m_playerEntity)) {
            auto playerStats = m_componentManager->getComponent<StatsComponent>(m_playerEntity);
            std::cout << "Player: Level " << playerStats->getLevel() 
                     << ", HP " << playerStats->getCurrentHP() << "/" << playerStats->getMaxHP() << std::endl;
        }
        
        // Graphics status
        std::cout << "Graphics: " << m_graphicsAPI->getWindowWidth() << "x" << m_graphicsAPI->getWindowHeight() 
                 << " (" << m_graphicsAPI->getAPIName() << ")" << std::endl;
        
        std::cout << "Frames rendered: " << m_frameCount << std::endl;
    }
    
private:
    // Core systems
    std::shared_ptr<OpenGLAPI> m_graphicsAPI;
    std::shared_ptr<ShaderManager> m_shaderManager;
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
    std::shared_ptr<Camera> m_camera;
    
    // Game state
    Entity m_playerEntity;
    std::vector<Entity> m_testEntities;
    bool m_running;
    int m_frameCount;
};

int main() {
    GameIntegrationTest test;
    
    if (!test.initialize()) {
        std::cerr << "❌ Failed to initialize game integration test" << std::endl;
        return 1;
    }
    
    test.run();
    test.printSystemStatus();
    test.shutdown();
    
    std::cout << "\n✅ Game Integration Test completed successfully!" << std::endl;
    std::cout << "=== Integration Test Complete ===" << std::endl;
    
    return 0;
}