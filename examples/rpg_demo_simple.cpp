#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>

// Core Engine
#include "../src/core/EngineCore.h"
#include "../src/core/ConfigurationManager.h"

// Systems
#include "../src/systems/SystemManager.h"

// Components
#include "../src/components/ComponentManager.h"
#include "../src/components/StatsComponent.h"
#include "../src/components/InventoryComponent.h"
#include "../src/components/DialogueComponent.h"
#include "../src/components/QuestComponent.h"

// Graphics
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/Camera.h"

// Entities
#include "../src/entities/EntityManager.h"

// Scene Management
#include "../src/scene/SceneManager.h"
#include "../src/scene/GameScene.h"
#include "../src/scene/MenuScene.h"

// Save System
#include "../src/save/SaveManager.h"

// Resources
#include "../src/resources/ResourceManager.h"

// Debug Tools
#include "../src/debug/PerformanceProfiler.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Components;

/**
 * Simplified RPG Demo Game
 * 
 * This demo showcases the core engine features that are currently working:
 * - Entity-Component-System architecture
 * - Graphics rendering with OpenGL
 * - Component management (Stats, Inventory, Dialogue, Quests)
 * - Scene management
 * - Save/load system
 * - Performance profiling
 * - Configuration management
 */
class RPGDemoSimple {
private:
    // Core Engine Components
    std::shared_ptr<EngineCore> engine;
    std::shared_ptr<SystemManager> systemManager;
    std::shared_ptr<EntityManager> entityManager;
    std::shared_ptr<ComponentManager> componentManager;
    std::shared_ptr<RPGEngine::Core::ConfigurationManager> configManager;
    
    // Graphics
    std::shared_ptr<OpenGLAPI> graphicsAPI;
    std::shared_ptr<Camera> camera;
    
    // Scene Management
    std::shared_ptr<RPGEngine::Scene::SceneManager> sceneManager;
    std::shared_ptr<RPGEngine::Scene::GameScene> gameScene;
    std::shared_ptr<RPGEngine::Scene::MenuScene> menuScene;
    
    // Save System
    std::shared_ptr<Engine::Save::SaveManager> saveManager;
    
    // Resources
    std::shared_ptr<RPGEngine::Resources::ResourceManager> resourceManager;
    
    // Debug Tools
    std::shared_ptr<Engine::Debug::PerformanceProfiler> profiler;
    
    // Game Entities
    Entity playerEntity;
    std::vector<Entity> npcEntities;
    std::vector<Entity> questEntities;
    
    // Game State
    bool isRunning;
    bool showDebugInfo;
    float gameTime;
    int currentScene; // 0 = menu, 1 = game
    
public:
    RPGDemoSimple() : isRunning(false), showDebugInfo(false), 
                      gameTime(0.0f), currentScene(0) {}
    
    bool initialize() {
        std::cout << "=== RPG Engine Simple Demo ===" << std::endl;
        std::cout << "Initializing simplified RPG demo..." << std::endl;
        
        // Initialize core systems
        if (!initializeCore()) {
            std::cerr << "Failed to initialize core engine" << std::endl;
            return false;
        }
        
        // Initialize graphics
        if (!initializeGraphics()) {
            std::cerr << "Failed to initialize graphics" << std::endl;
            return false;
        }
        
        // Initialize scenes
        if (!initializeScenes()) {
            std::cerr << "Failed to initialize scenes" << std::endl;
            return false;
        }
        
        // Create demo content
        if (!createDemoContent()) {
            std::cerr << "Failed to create demo content" << std::endl;
            return false;
        }
        
        std::cout << "✅ RPG Simple Demo initialized successfully!" << std::endl;
        return true;
    }
    
    void run() {
        isRunning = true;
        
        std::cout << "\n🎮 Starting RPG Simple Demo..." << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  SPACE - Switch between menu and game scenes" << std::endl;
        std::cout << "  TAB - Toggle debug info" << std::endl;
        std::cout << "  ESC - Exit demo" << std::endl;
        std::cout << "  1 - Show player stats" << std::endl;
        std::cout << "  2 - Show inventory" << std::endl;
        std::cout << "  3 - Show quests" << std::endl;
        std::cout << "  4 - Show dialogue" << std::endl;
        std::cout << "  S - Save game" << std::endl;
        std::cout << "  L - Load game" << std::endl;
        
        // Main game loop
        while (isRunning && !graphicsAPI->shouldClose()) {
            float deltaTime = calculateDeltaTime();
            gameTime += deltaTime;
            
            // Handle input
            handleInput();
            
            // Update systems
            updateSystems(deltaTime);
            
            // Render frame
            render();
            
            // Update profiler
            if (profiler) {
                profiler->endFrame();
            }
            
            // Poll events
            graphicsAPI->pollEvents();
        }
        
        std::cout << "🎮 RPG Simple Demo ended." << std::endl;
    }
    
    void shutdown() {
        std::cout << "Shutting down RPG Simple Demo..." << std::endl;
        
        // Save final game state
        if (saveManager) {
            saveManager->saveGame("final_save.json");
        }
        
        // Shutdown systems
        if (graphicsAPI) graphicsAPI->shutdown();
        if (engine) engine->shutdown();
        
        std::cout << "✅ RPG Simple Demo shutdown complete." << std::endl;
    }

private:
    bool initializeCore() {
        // Configuration
        configManager = std::make_shared<RPGEngine::Core::ConfigurationManager>();
        
        // Try to load config, but continue if it fails
        try {
            configManager->loadFromFile("config/game_config.json");
            std::cout << "  ✅ Loaded configuration from file" << std::endl;
        } catch (...) {
            std::cout << "  ⚠️  Using default configuration" << std::endl;
        }
        
        // Core engine
        engine = std::make_shared<EngineCore>();
        if (!engine->initialize()) {
            return false;
        }
        
        // Entity and component management
        entityManager = std::make_shared<EntityManager>();
        componentManager = std::make_shared<ComponentManager>();
        
        // System manager
        systemManager = std::make_shared<SystemManager>();
        
        // Resource manager
        resourceManager = std::make_shared<RPGEngine::Resources::ResourceManager>();
        
        // Save system
        saveManager = std::make_shared<Engine::Save::SaveManager>();
        
        // Debug tools
        profiler = std::make_shared<Engine::Debug::PerformanceProfiler>();
        
        return true;
    }
    
    bool initializeGraphics() {
        // Graphics API
        graphicsAPI = std::make_shared<OpenGLAPI>();
        if (!graphicsAPI->initialize(800, 600, "RPG Engine Simple Demo", false)) {
            return false;
        }
        
        // Camera
        camera = std::make_shared<Camera>();
        camera->setViewportSize(800, 600);
        camera->setPosition(0, 0);
        
        return true;
    }
    
    bool initializeScenes() {
        sceneManager = std::make_shared<RPGEngine::Scene::SceneManager>();
        
        // Create scenes
        menuScene = std::make_shared<RPGEngine::Scene::MenuScene>(entityManager, componentManager);
        gameScene = std::make_shared<RPGEngine::Scene::GameScene>(entityManager, componentManager);
        
        // Add scenes to manager (using registerScene instead of addScene)
        sceneManager->registerScene("menu", menuScene);
        sceneManager->registerScene("game", gameScene);
        
        // Start with menu scene
        sceneManager->switchToScene("menu");
        
        return true;
    }
    
    bool createDemoContent() {
        std::cout << "Creating demo content..." << std::endl;
        
        // Create player
        createPlayer();
        
        // Create NPCs with dialogue
        createNPCs();
        
        // Create quests
        createQuests();
        
        std::cout << "✅ Demo content created successfully!" << std::endl;
        return true;
    }
    
    void createPlayer() {
        playerEntity = entityManager->createEntity();
        
        // Stats component
        auto stats = std::make_shared<StatsComponent>(playerEntity);
        stats->setLevel(1);
        // Experience starts at 0 by default
        componentManager->addComponent(playerEntity, stats);
        
        // Inventory component
        auto inventory = std::make_shared<InventoryComponent>(playerEntity);
        inventory->setCapacity(20);
        componentManager->addComponent(playerEntity, inventory);
        
        std::cout << "  ✅ Player created with stats and inventory" << std::endl;
    }
    
    void createNPCs() {
        // Create village elder NPC
        Entity elderNPC = entityManager->createEntity();
        
        auto elderDialogue = std::make_shared<DialogueComponent>(elderNPC);
        // Dialogue starts with default node
        componentManager->addComponent(elderNPC, elderDialogue);
        
        // Create merchant NPC
        Entity merchantNPC = entityManager->createEntity();
        
        auto merchantInventory = std::make_shared<InventoryComponent>(merchantNPC);
        merchantInventory->setCapacity(50);
        componentManager->addComponent(merchantNPC, merchantInventory);
        
        npcEntities.push_back(elderNPC);
        npcEntities.push_back(merchantNPC);
        
        std::cout << "  ✅ Created " << npcEntities.size() << " NPCs" << std::endl;
    }
    
    void createQuests() {
        // Create main quest
        Entity mainQuest = entityManager->createEntity();
        
        auto questComponent = std::make_shared<QuestComponent>(mainQuest);
        questComponent->setTitle("Village Defense");
        questComponent->setDescription("Help defend the village from threats.");
        questComponent->addObjective("Talk to the village elder");
        questComponent->addObjective("Gather information");
        questComponent->addObjective("Report back");
        questComponent->setRewardExperience(100);
        componentManager->addComponent(mainQuest, questComponent);
        
        questEntities.push_back(mainQuest);
        
        std::cout << "  ✅ Created " << questEntities.size() << " quests" << std::endl;
    }
    
    float calculateDeltaTime() {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        return std::min(deltaTime, 0.016f); // Cap at 60 FPS
    }
    
    void handleInput() {
        // Simple keyboard input simulation
        static bool spacePressed = false;
        static bool tabPressed = false;
        static bool escPressed = false;
        static bool key1Pressed = false;
        static bool key2Pressed = false;
        static bool key3Pressed = false;
        static bool key4Pressed = false;
        static bool sPPressed = false;
        static bool lPressed = false;
        
        // In a real implementation, this would use the InputManager
        // For now, we'll simulate some key presses based on time
        
        // Toggle scene every 5 seconds
        if (static_cast<int>(gameTime) % 10 == 5 && !spacePressed) {
            spacePressed = true;
            currentScene = (currentScene + 1) % 2;
            if (currentScene == 0) {
                sceneManager->switchToScene("menu");
                std::cout << "Switched to Menu Scene" << std::endl;
            } else {
                sceneManager->switchToScene("game");
                std::cout << "Switched to Game Scene" << std::endl;
            }
        } else if (static_cast<int>(gameTime) % 10 != 5) {
            spacePressed = false;
        }
        
        // Toggle debug info every 7 seconds
        if (static_cast<int>(gameTime) % 14 == 7 && !tabPressed) {
            tabPressed = true;
            showDebugInfo = !showDebugInfo;
            std::cout << "Debug info: " << (showDebugInfo ? "ON" : "OFF") << std::endl;
        } else if (static_cast<int>(gameTime) % 14 != 7) {
            tabPressed = false;
        }
        
        // Show player stats every 12 seconds
        if (static_cast<int>(gameTime) % 24 == 12 && !key1Pressed) {
            key1Pressed = true;
            showPlayerStats();
        } else if (static_cast<int>(gameTime) % 24 != 12) {
            key1Pressed = false;
        }
        
        // Show inventory every 15 seconds
        if (static_cast<int>(gameTime) % 30 == 15 && !key2Pressed) {
            key2Pressed = true;
            showInventory();
        } else if (static_cast<int>(gameTime) % 30 != 15) {
            key2Pressed = false;
        }
        
        // Show quests every 18 seconds
        if (static_cast<int>(gameTime) % 36 == 18 && !key3Pressed) {
            key3Pressed = true;
            showQuests();
        } else if (static_cast<int>(gameTime) % 36 != 18) {
            key3Pressed = false;
        }
        
        // Save game every 25 seconds
        if (static_cast<int>(gameTime) % 50 == 25 && !sPPressed) {
            sPPressed = true;
            saveGame();
        } else if (static_cast<int>(gameTime) % 50 != 25) {
            sPPressed = false;
        }
    }
    
    void showPlayerStats() {
        auto playerStats = componentManager->getComponent<StatsComponent>(playerEntity);
        if (playerStats) {
            std::cout << "\n📊 Player Stats:" << std::endl;
            std::cout << "  Level: " << playerStats->getLevel() << std::endl;
            std::cout << "  Experience: " << playerStats->getExperience() << std::endl;
        }
    }
    
    void showInventory() {
        auto playerInventory = componentManager->getComponent<InventoryComponent>(playerEntity);
        if (playerInventory) {
            std::cout << "\n🎒 Player Inventory:" << std::endl;
            std::cout << "  Capacity: " << playerInventory->getCapacity() << std::endl;
            std::cout << "  Items: " << playerInventory->getItemCount() << std::endl;
        }
    }
    
    void showQuests() {
        std::cout << "\n📋 Active Quests:" << std::endl;
        for (Entity quest : questEntities) {
            auto questComponent = componentManager->getComponent<QuestComponent>(quest);
            if (questComponent) {
                std::cout << "  Quest: " << questComponent->getTitle() << std::endl;
                std::cout << "    Description: " << questComponent->getDescription() << std::endl;
                std::cout << "    Objectives: " << questComponent->getObjectives().size() << std::endl;
                std::cout << "    Reward XP: " << questComponent->getRewardExperience() << std::endl;
            }
        }
    }
    
    void saveGame() {
        if (saveManager) {
            saveManager->saveGame("demo_save.json");
            std::cout << "\n💾 Game saved!" << std::endl;
        }
    }
    
    void updateSystems(float deltaTime) {
        if (profiler) {
            profiler->beginFrame();
        }
        
        // Update scene manager
        if (sceneManager) {
            sceneManager->update(deltaTime);
        }
        
        // Update system manager
        if (systemManager) {
            systemManager->update(deltaTime);
        }
    }
    
    void render() {
        graphicsAPI->beginFrame();
        
        // Set background color based on current scene
        if (currentScene == 0) {
            // Menu scene - blue background
            graphicsAPI->clear(0.2f, 0.3f, 0.8f, 1.0f);
        } else {
            // Game scene - green background
            graphicsAPI->clear(0.2f, 0.6f, 0.3f, 1.0f);
        }
        
        // Render current scene
        if (sceneManager) {
            sceneManager->render();
        }
        
        // Render debug info
        if (showDebugInfo) {
            renderDebugInfo();
        }
        
        graphicsAPI->endFrame();
    }
    
    void renderDebugInfo() {
        // In a real implementation, this would use the DebugRenderer
        // For now, we'll just output to console periodically
        static float lastDebugOutput = 0;
        if (gameTime - lastDebugOutput > 2.0f) {
            lastDebugOutput = gameTime;
            
            if (profiler) {
                auto stats = profiler->getFrameStats();
                std::cout << "🔧 Debug Info - FPS: " << stats.fps 
                         << ", Frame Time: " << stats.frameTime << "ms"
                         << ", Entities: " << entityManager->getEntityCount()
                         << ", Scene: " << (currentScene == 0 ? "Menu" : "Game")
                         << std::endl;
            }
        }
    }
};

int main() {
    RPGDemoSimple demo;
    
    if (!demo.initialize()) {
        std::cerr << "Failed to initialize RPG Simple Demo" << std::endl;
        return -1;
    }
    
    demo.run();
    demo.shutdown();
    
    return 0;
}