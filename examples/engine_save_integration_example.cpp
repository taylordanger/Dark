#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "../src/core/EngineCore.h"
#include "../src/save/SaveLoadManager.h"
#include "../src/save/SaveIntegration.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/scene/SceneManager.h"
#include "../src/world/WorldManager.h"
#include "../src/audio/MusicManager.h"
#include "../src/audio/SoundEffectManager.h"
#include "../src/resources/ResourceManager.h"
#include "../src/input/InputManager.h"

using namespace RPGEngine;
using namespace Engine::Save;

/**
 * Example game class that integrates the save/load system with the engine
 */
class RPGGameWithSaveSystem {
public:
    RPGGameWithSaveSystem() = default;
    ~RPGGameWithSaveSystem() = default;
    
    bool initialize() {
        std::cout << "Initializing RPG Game with Save System..." << std::endl;
        
        // Create core engine
        m_engine = std::make_shared<EngineCore>();
        
        // Create all managers
        m_entityManager = std::make_shared<EntityManager>();
        m_componentManager = std::make_shared<ComponentManager>();
        m_systemManager = std::make_shared<SystemManager>();
        m_resourceManager = std::make_shared<Resources::ResourceManager>();
        m_sceneManager = std::make_shared<Scene::SceneManager>(
            m_entityManager, m_componentManager, m_systemManager, m_resourceManager);
        m_worldManager = std::make_shared<World::WorldManager>();
        m_musicManager = std::make_shared<Audio::MusicManager>();
        m_soundManager = std::make_shared<Audio::SoundEffectManager>();
        m_inputManager = std::make_shared<InputManager>();
        
        // Initialize managers
        if (!m_entityManager->initialize()) {
            std::cerr << "Failed to initialize EntityManager" << std::endl;
            return false;
        }
        
        if (!m_componentManager->initialize()) {
            std::cerr << "Failed to initialize ComponentManager" << std::endl;
            return false;
        }
        
        if (!m_systemManager->initialize()) {
            std::cerr << "Failed to initialize SystemManager" << std::endl;
            return false;
        }
        
        if (!m_resourceManager->initialize()) {
            std::cerr << "Failed to initialize ResourceManager" << std::endl;
            return false;
        }
        
        if (!m_worldManager->initialize()) {
            std::cerr << "Failed to initialize WorldManager" << std::endl;
            return false;
        }
        
        if (!m_musicManager->initialize()) {
            std::cerr << "Failed to initialize MusicManager" << std::endl;
            return false;
        }
        
        if (!m_soundManager->initialize()) {
            std::cerr << "Failed to initialize SoundEffectManager" << std::endl;
            return false;
        }
        
        if (!m_inputManager->initialize()) {
            std::cerr << "Failed to initialize InputManager" << std::endl;
            return false;
        }
        
        // Create save system
        m_saveManager = std::make_shared<SaveManager>();
        if (!m_saveManager->initialize("rpg_game_saves")) {
            std::cerr << "Failed to initialize SaveManager" << std::endl;
            return false;
        }
        
        m_saveIntegration = std::make_shared<SaveIntegration>();
        if (!m_saveIntegration->initialize(m_saveManager, m_entityManager, m_componentManager,
                                          m_systemManager, m_sceneManager, m_worldManager)) {
            std::cerr << "Failed to initialize SaveIntegration" << std::endl;
            return false;
        }
        
        // Set audio managers for save integration
        m_saveIntegration->setAudioManagers(m_musicManager, m_soundManager);
        
        // Configure auto-save
        AutoSaveConfig autoSaveConfig;
        autoSaveConfig.enabled = true;
        autoSaveConfig.intervalSeconds = 300.0f; // 5 minutes
        autoSaveConfig.maxAutoSaves = 5;
        autoSaveConfig.saveOnMapTransition = true;
        autoSaveConfig.saveOnLevelUp = true;
        autoSaveConfig.saveOnQuestComplete = true;
        autoSaveConfig.saveOnCombatEnd = true;
        m_saveIntegration->setAutoSaveConfig(autoSaveConfig);
        
        m_saveLoadManager = std::make_shared<SaveLoadManager>();
        if (!m_saveLoadManager->initialize(m_saveIntegration)) {
            std::cerr << "Failed to initialize SaveLoadManager" << std::endl;
            return false;
        }
        
        // Set up save/load event callbacks
        m_saveLoadManager->registerEventCallback([this](const SaveLoadEventData& eventData) {
            handleSaveLoadEvent(eventData);
        });
        
        // Configure save/load UI
        auto ui = m_saveLoadManager->getUI();
        if (ui) {
            UI::SaveLoadUIConfig uiConfig;
            uiConfig.maxSaveSlots = 10;
            uiConfig.showAutoSaves = true;
            uiConfig.showQuickSave = true;
            uiConfig.allowSlotDeletion = true;
            uiConfig.confirmOverwrite = true;
            ui->setConfig(uiConfig);
        }
        
        // Initialize engine
        if (!m_engine->initialize()) {
            std::cerr << "Failed to initialize Engine" << std::endl;
            return false;
        }
        
        std::cout << "RPG Game with Save System initialized successfully!" << std::endl;
        return true;
    }
    
    void run() {
        if (!initialize()) {
            std::cerr << "Failed to initialize game!" << std::endl;
            return;
        }
        
        std::cout << "Starting game loop..." << std::endl;
        
        // Simulate game loop
        auto lastTime = std::chrono::high_resolution_clock::now();
        bool running = true;
        int frameCount = 0;
        
        while (running && frameCount < 1000) { // Run for 1000 frames for testing
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Update save system
            m_saveLoadManager->update(deltaTime);
            
            // Simulate game events that trigger auto-save
            if (frameCount == 100) {
                std::cout << "Simulating map transition..." << std::endl;
                m_saveLoadManager->onGameEvent("map_transition");
            }
            
            if (frameCount == 200) {
                std::cout << "Simulating level up..." << std::endl;
                m_saveLoadManager->onGameEvent("level_up");
            }
            
            if (frameCount == 300) {
                std::cout << "Simulating quest complete..." << std::endl;
                m_saveLoadManager->onGameEvent("quest_complete");
            }
            
            if (frameCount == 400) {
                std::cout << "Simulating combat end..." << std::endl;
                m_saveLoadManager->onGameEvent("combat_end");
            }
            
            // Test manual save/load operations
            if (frameCount == 500) {
                std::cout << "Testing manual save..." << std::endl;
                m_saveLoadManager->saveGame(1, [](bool success, const std::string& message) {
                    std::cout << "Manual save result: " << (success ? "Success" : "Failed") 
                              << " - " << message << std::endl;
                });
            }
            
            if (frameCount == 600) {
                std::cout << "Testing quick save..." << std::endl;
                m_saveLoadManager->quickSave([](bool success, const std::string& message) {
                    std::cout << "Quick save result: " << (success ? "Success" : "Failed") 
                              << " - " << message << std::endl;
                });
            }
            
            if (frameCount == 700) {
                std::cout << "Testing load..." << std::endl;
                m_saveLoadManager->loadGame(1, [](bool success, const std::string& message) {
                    std::cout << "Load result: " << (success ? "Success" : "Failed") 
                              << " - " << message << std::endl;
                });
            }
            
            if (frameCount == 800) {
                std::cout << "Testing UI operations..." << std::endl;
                m_saveLoadManager->showSaveMenu();
                
                // Simulate UI interaction
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m_saveLoadManager->hideUI();
                
                m_saveLoadManager->showLoadMenu();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m_saveLoadManager->hideUI();
            }
            
            // Simulate frame timing
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
            frameCount++;
        }
        
        std::cout << "Game loop completed after " << frameCount << " frames." << std::endl;
        shutdown();
    }
    
    void shutdown() {
        std::cout << "Shutting down RPG Game with Save System..." << std::endl;
        
        if (m_saveLoadManager) {
            m_saveLoadManager->shutdown();
        }
        
        if (m_saveIntegration) {
            m_saveIntegration->shutdown();
        }
        
        if (m_saveManager) {
            m_saveManager->shutdown();
        }
        
        if (m_engine) {
            m_engine->shutdown();
        }
        
        std::cout << "Shutdown complete." << std::endl;
    }
    
    // Input handling for save/load operations
    void handleInput(const std::string& input) {
        if (input == "F5") {
            // Quick save
            m_saveLoadManager->quickSave([](bool success, const std::string& message) {
                std::cout << "Quick save: " << message << std::endl;
            });
        } else if (input == "F9") {
            // Quick load
            m_saveLoadManager->quickLoad([](bool success, const std::string& message) {
                std::cout << "Quick load: " << message << std::endl;
            });
        } else if (input == "ESC") {
            // Show save menu
            if (!m_saveLoadManager->isUIVisible()) {
                m_saveLoadManager->showSaveMenu();
            } else {
                m_saveLoadManager->hideUI();
            }
        }
    }
    
    // Get save/load manager for external access
    std::shared_ptr<SaveLoadManager> getSaveLoadManager() const {
        return m_saveLoadManager;
    }
    
private:
    void handleSaveLoadEvent(const SaveLoadEventData& eventData) {
        switch (eventData.type) {
            case SaveLoadEventType::SaveStarted:
                std::cout << "Save started for slot: " << eventData.slotName << std::endl;
                break;
                
            case SaveLoadEventType::SaveCompleted:
                std::cout << "Save completed for slot: " << eventData.slotName 
                          << " (took " << eventData.duration << "s)" << std::endl;
                break;
                
            case SaveLoadEventType::SaveFailed:
                std::cout << "Save failed for slot: " << eventData.slotName 
                          << " - " << eventData.errorMessage << std::endl;
                break;
                
            case SaveLoadEventType::LoadStarted:
                std::cout << "Load started for slot: " << eventData.slotName << std::endl;
                break;
                
            case SaveLoadEventType::LoadCompleted:
                std::cout << "Load completed for slot: " << eventData.slotName 
                          << " (took " << eventData.duration << "s)" << std::endl;
                break;
                
            case SaveLoadEventType::LoadFailed:
                std::cout << "Load failed for slot: " << eventData.slotName 
                          << " - " << eventData.errorMessage << std::endl;
                break;
                
            case SaveLoadEventType::AutoSaveTriggered:
                std::cout << "Auto-save triggered" << std::endl;
                break;
                
            case SaveLoadEventType::QuickSaveTriggered:
                std::cout << "Quick save triggered" << std::endl;
                break;
                
            case SaveLoadEventType::QuickLoadTriggered:
                std::cout << "Quick load triggered" << std::endl;
                break;
        }
    }
    
    // Core systems
    std::shared_ptr<EngineCore> m_engine;
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
    std::shared_ptr<SystemManager> m_systemManager;
    std::shared_ptr<Resources::ResourceManager> m_resourceManager;
    std::shared_ptr<Scene::SceneManager> m_sceneManager;
    std::shared_ptr<World::WorldManager> m_worldManager;
    std::shared_ptr<Audio::MusicManager> m_musicManager;
    std::shared_ptr<Audio::SoundEffectManager> m_soundManager;
    std::shared_ptr<InputManager> m_inputManager;
    
    // Save system
    std::shared_ptr<SaveManager> m_saveManager;
    std::shared_ptr<SaveIntegration> m_saveIntegration;
    std::shared_ptr<SaveLoadManager> m_saveLoadManager;
};

int main() {
    std::cout << "=== RPG Engine Save System Integration Example ===" << std::endl;
    
    try {
        RPGGameWithSaveSystem game;
        game.run();
        
        std::cout << "\n✅ Save system integration example completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Example failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Example failed with unknown exception" << std::endl;
        return 1;
    }
}