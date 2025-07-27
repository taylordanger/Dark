#include <iostream>
#include <memory>
#include <unordered_map>

#include "../src/save/SaveIntegration.h"
#include "../src/save/SystemStateSerializer.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/scene/SceneManager.h"
#include "../src/world/WorldManager.h"
#include "../src/resources/ResourceManager.h"

using namespace Engine::Save;
using namespace RPGEngine;

/**
 * Example system that demonstrates save/load integration
 */
class ExampleGameSystem {
public:
    ExampleGameSystem() : m_initialized(false) {}
    
    bool initialize() {
        if (m_initialized) {
            return true;
        }
        
        // Initialize system state
        m_gameFlags["tutorial_completed"] = false;
        m_gameFlags["first_boss_defeated"] = false;
        m_gameFlags["secret_area_discovered"] = false;
        
        m_gameStats["total_playtime"] = 0.0f;
        m_gameStats["enemies_defeated"] = 0.0f;
        m_gameStats["items_collected"] = 0.0f;
        m_gameStats["experience_gained"] = 0.0f;
        
        m_gameSettings["difficulty"] = "normal";
        m_gameSettings["language"] = "english";
        m_gameSettings["last_checkpoint"] = "starting_area";
        
        m_initialized = true;
        return true;
    }
    
    void update(float deltaTime) {
        if (!m_initialized) {
            return;
        }
        
        // Update playtime
        m_gameStats["total_playtime"] += deltaTime;
    }
    
    void registerSaveLoad(std::shared_ptr<SaveIntegration> saveIntegration) {
        if (!saveIntegration) {
            return;
        }
        
        // Register boolean flags
        SystemStateRegistry::registerBooleanSystem(
            saveIntegration,
            "ExampleGameSystem_Flags",
            [this]() -> std::unordered_map<std::string, bool> {
                return m_gameFlags;
            },
            [this](const std::unordered_map<std::string, bool>& data) -> bool {
                m_gameFlags = data;
                std::cout << "Restored " << data.size() << " game flags" << std::endl;
                return true;
            }
        );
        
        // Register numeric stats
        SystemStateRegistry::registerNumericSystem(
            saveIntegration,
            "ExampleGameSystem_Stats",
            [this]() -> std::unordered_map<std::string, float> {
                return m_gameStats;
            },
            [this](const std::unordered_map<std::string, float>& data) -> bool {
                m_gameStats = data;
                std::cout << "Restored " << data.size() << " game stats" << std::endl;
                return true;
            }
        );
        
        // Register string settings
        SystemStateRegistry::registerKeyValueSystem(
            saveIntegration,
            "ExampleGameSystem_Settings",
            [this]() -> std::unordered_map<std::string, std::string> {
                return m_gameSettings;
            },
            [this](const std::unordered_map<std::string, std::string>& data) -> bool {
                m_gameSettings = data;
                std::cout << "Restored " << data.size() << " game settings" << std::endl;
                return true;
            }
        );
        
        std::cout << "ExampleGameSystem registered for save/load" << std::endl;
    }
    
    // Game state modification methods for testing
    void completeFlag(const std::string& flagName) {
        m_gameFlags[flagName] = true;
        std::cout << "Flag completed: " << flagName << std::endl;
    }
    
    void addStat(const std::string& statName, float value) {
        m_gameStats[statName] += value;
        std::cout << "Stat updated: " << statName << " = " << m_gameStats[statName] << std::endl;
    }
    
    void setSetting(const std::string& settingName, const std::string& value) {
        m_gameSettings[settingName] = value;
        std::cout << "Setting updated: " << settingName << " = " << value << std::endl;
    }
    
    void printState() const {
        std::cout << "\n=== ExampleGameSystem State ===" << std::endl;
        
        std::cout << "Flags:" << std::endl;
        for (const auto& [key, value] : m_gameFlags) {
            std::cout << "  " << key << ": " << (value ? "true" : "false") << std::endl;
        }
        
        std::cout << "Stats:" << std::endl;
        for (const auto& [key, value] : m_gameStats) {
            std::cout << "  " << key << ": " << value << std::endl;
        }
        
        std::cout << "Settings:" << std::endl;
        for (const auto& [key, value] : m_gameSettings) {
            std::cout << "  " << key << ": " << value << std::endl;
        }
        
        std::cout << "==============================\n" << std::endl;
    }
    
private:
    bool m_initialized;
    std::unordered_map<std::string, bool> m_gameFlags;
    std::unordered_map<std::string, float> m_gameStats;
    std::unordered_map<std::string, std::string> m_gameSettings;
};

/**
 * Another example system with custom serialization
 */
class CustomSerializationSystem {
public:
    struct ComplexData {
        int id;
        std::string name;
        std::vector<float> values;
        
        // Custom JSON serialization
        nlohmann::json toJson() const {
            nlohmann::json json;
            json["id"] = id;
            json["name"] = name;
            json["values"] = values;
            return json;
        }
        
        // Custom JSON deserialization
        bool fromJson(const nlohmann::json& json) {
            try {
                id = json["id"];
                name = json["name"];
                values = json["values"].get<std::vector<float>>();
                return true;
            } catch (const std::exception& e) {
                return false;
            }
        }
    };
    
    CustomSerializationSystem() : m_initialized(false) {}
    
    bool initialize() {
        if (m_initialized) {
            return true;
        }
        
        // Initialize complex data
        m_complexData.id = 12345;
        m_complexData.name = "CustomSystem";
        m_complexData.values = {1.0f, 2.5f, 3.7f, 4.2f};
        
        m_initialized = true;
        return true;
    }
    
    void registerSaveLoad(std::shared_ptr<SaveIntegration> saveIntegration) {
        if (!saveIntegration) {
            return;
        }
        
        // Register custom serialization
        SystemStateRegistry::registerSystem(
            saveIntegration,
            "CustomSerializationSystem",
            [this]() -> std::string {
                nlohmann::json json;
                json["complexData"] = m_complexData.toJson();
                return json.dump();
            },
            [this](const std::string& jsonStr) -> bool {
                try {
                    nlohmann::json json = nlohmann::json::parse(jsonStr);
                    return m_complexData.fromJson(json["complexData"]);
                } catch (const std::exception& e) {
                    std::cerr << "Failed to deserialize CustomSerializationSystem: " << e.what() << std::endl;
                    return false;
                }
            }
        );
        
        std::cout << "CustomSerializationSystem registered for save/load" << std::endl;
    }
    
    void modifyData() {
        m_complexData.id += 1;
        m_complexData.name += "_modified";
        m_complexData.values.push_back(5.5f);
        std::cout << "CustomSerializationSystem data modified" << std::endl;
    }
    
    void printState() const {
        std::cout << "\n=== CustomSerializationSystem State ===" << std::endl;
        std::cout << "ID: " << m_complexData.id << std::endl;
        std::cout << "Name: " << m_complexData.name << std::endl;
        std::cout << "Values: ";
        for (float value : m_complexData.values) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    
private:
    bool m_initialized;
    ComplexData m_complexData;
};

int main() {
    std::cout << "=== System Save Registration Example ===" << std::endl;
    
    try {
        // Create all required managers
        auto entityManager = std::make_shared<EntityManager>();
        auto componentManager = std::make_shared<ComponentManager>();
        auto systemManager = std::make_shared<SystemManager>();
        auto resourceManager = std::make_shared<Resources::ResourceManager>();
        auto sceneManager = std::make_shared<Scene::SceneManager>(entityManager, componentManager, systemManager, resourceManager);
        auto worldManager = std::make_shared<World::WorldManager>();
        
        // Initialize managers
        entityManager->initialize();
        componentManager->initialize();
        systemManager->initialize();
        resourceManager->initialize();
        worldManager->initialize();
        
        // Create save system
        auto saveManager = std::make_shared<SaveManager>();
        saveManager->initialize("system_registration_test");
        
        auto saveIntegration = std::make_shared<SaveIntegration>();
        saveIntegration->initialize(saveManager, entityManager, componentManager, 
                                   systemManager, sceneManager, worldManager);
        
        // Create example systems
        auto exampleSystem = std::make_shared<ExampleGameSystem>();
        auto customSystem = std::make_shared<CustomSerializationSystem>();
        
        exampleSystem->initialize();
        customSystem->initialize();
        
        // Register systems for save/load
        exampleSystem->registerSaveLoad(saveIntegration);
        customSystem->registerSaveLoad(saveIntegration);
        
        // Modify system states
        std::cout << "\n--- Modifying system states ---" << std::endl;
        exampleSystem->completeFlag("tutorial_completed");
        exampleSystem->completeFlag("first_boss_defeated");
        exampleSystem->addStat("enemies_defeated", 15.0f);
        exampleSystem->addStat("items_collected", 8.0f);
        exampleSystem->setSetting("difficulty", "hard");
        exampleSystem->setSetting("last_checkpoint", "forest_entrance");
        
        customSystem->modifyData();
        
        // Print initial states
        std::cout << "\n--- Initial States ---" << std::endl;
        exampleSystem->printState();
        customSystem->printState();
        
        // Save game state
        std::cout << "--- Saving game state ---" << std::endl;
        SaveIntegrationResult saveResult = saveIntegration->saveGameState("system_test");
        if (saveResult == SaveIntegrationResult::Success) {
            std::cout << "✓ Game state saved successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to save game state" << std::endl;
            return 1;
        }
        
        // Modify states again to show difference
        std::cout << "\n--- Modifying states again ---" << std::endl;
        exampleSystem->completeFlag("secret_area_discovered");
        exampleSystem->addStat("experience_gained", 1000.0f);
        exampleSystem->setSetting("language", "spanish");
        
        customSystem->modifyData();
        
        // Print modified states
        std::cout << "\n--- Modified States ---" << std::endl;
        exampleSystem->printState();
        customSystem->printState();
        
        // Load game state
        std::cout << "--- Loading game state ---" << std::endl;
        LoadResult loadResult = saveIntegration->loadGameState("system_test");
        if (loadResult == LoadResult::Success) {
            std::cout << "✓ Game state loaded successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to load game state" << std::endl;
            return 1;
        }
        
        // Print restored states
        std::cout << "\n--- Restored States ---" << std::endl;
        exampleSystem->printState();
        customSystem->printState();
        
        // Cleanup
        saveIntegration->shutdown();
        saveManager->shutdown();
        
        std::cout << "✅ System save registration example completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Example failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Example failed with unknown exception" << std::endl;
        return 1;
    }
}