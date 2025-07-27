#include <iostream>
#include <memory>
#include "../src/core/ConfigurationManager.h"
#include "../src/entities/EntityFactory.h"
#include "../src/entities/EntityManager.h"

using namespace RPGEngine;

void testConfigurationManager() {
    std::cout << "\n=== Testing Configuration Manager ===" << std::endl;
    
    Core::ConfigurationManager config;
    
    // Test loading from string
    std::string jsonConfig = R"({
        "game": {
            "title": "Test RPG",
            "version": "1.0.0",
            "settings": {
                "fullscreen": false,
                "resolution": {
                    "width": 1920,
                    "height": 1080
                },
                "audio": {
                    "masterVolume": 0.8,
                    "musicVolume": 0.6,
                    "sfxVolume": 0.7
                }
            }
        },
        "player": {
            "startingLevel": 1,
            "startingHealth": 100,
            "startingMana": 50
        }
    })";
    
    bool result = config.loadFromString(jsonConfig);
    std::cout << "Config loading result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    
    // Test getting values
    std::string title = config.getString("game.title", "Unknown");
    bool fullscreen = config.getBool("game.settings.fullscreen", true);
    int width = config.getInt("game.settings.resolution.width", 800);
    float volume = config.getFloat("game.settings.audio.masterVolume", 1.0f);
    
    std::cout << "Game title: " << title << std::endl;
    std::cout << "Fullscreen: " << (fullscreen ? "true" : "false") << std::endl;
    std::cout << "Resolution width: " << width << std::endl;
    std::cout << "Master volume: " << volume << std::endl;
    
    // Test setting values
    config.set("game.settings.fullscreen", Core::ConfigValue(true));
    config.set("player.startingLevel", Core::ConfigValue(5));
    
    std::cout << "Updated fullscreen: " << (config.getBool("game.settings.fullscreen") ? "true" : "false") << std::endl;
    std::cout << "Updated starting level: " << config.getInt("player.startingLevel") << std::endl;
    
    // Test change callbacks
    config.addChangeCallback("game.settings.fullscreen", [](const std::string& path, const Core::ConfigValue& value) {
        std::cout << "Config changed - " << path << ": " << (value.asBool() ? "true" : "false") << std::endl;
    });
    
    config.set("game.settings.fullscreen", Core::ConfigValue(false));
}

void testEntityFactory() {
    std::cout << "\n=== Testing Entity Factory ===" << std::endl;
    
    auto entityManager = std::make_shared<Entities::EntityManager>();
    Entities::EntityFactory factory(entityManager);
    
    // Create entity templates configuration
    std::string templatesConfig = R"({
        "templates": {
            "player": {
                "components": {
                    "StatsComponent": {
                        "health": 100,
                        "maxHealth": 100,
                        "mana": 50,
                        "maxMana": 50,
                        "level": 1,
                        "experience": 0
                    },
                    "InventoryComponent": {
                        "capacity": 20,
                        "items": [
                            {"id": "sword_basic", "quantity": 1},
                            {"id": "potion_health", "quantity": 3}
                        ]
                    },
                    "ScriptComponent": {
                        "script": "function onInitialize() print('Player initialized!') end"
                    }
                }
            },
            "npc_guard": {
                "components": {
                    "StatsComponent": {
                        "health": 80,
                        "maxHealth": 80,
                        "level": 3
                    },
                    "ScriptComponent": {
                        "script": "function onInitialize() print('Guard initialized!') end"
                    }
                }
            }
        }
    })";
    
    Core::ConfigurationManager config;
    if (config.loadFromString(templatesConfig)) {
        factory.loadTemplatesFromConfig(config);
        
        // Test creating entities from templates
        auto playerId = factory.createEntity("player");
        auto guardId = factory.createEntity("npc_guard");
        
        std::cout << "Created player entity: " << playerId << std::endl;
        std::cout << "Created guard entity: " << guardId << std::endl;
        
        // Test creating entity with overrides
        Core::ConfigValue overrides;
        nlohmann::json overrideJson = nlohmann::json::parse(R"({
            "components": {
                "StatsComponent": {
                    "level": 10,
                    "health": 200,
                    "maxHealth": 200
                }
            }
        })");
        overrides = Core::ConfigValue(overrideJson);
        
        auto eliteGuardId = factory.createEntity("npc_guard", overrides);
        std::cout << "Created elite guard entity: " << eliteGuardId << std::endl;
    }
}

void testDataDrivenEntityCreation() {
    std::cout << "\n=== Testing Data-Driven Entity Creation ===" << std::endl;
    
    auto entityManager = std::make_shared<Entities::EntityManager>();
    Entities::EntityFactory factory(entityManager);
    
    // Create entities directly from configuration
    std::string entitiesConfig = R"({
        "entities": [
            {
                "template": "player",
                "components": {
                    "StatsComponent": {
                        "level": 5,
                        "health": 150,
                        "maxHealth": 150
                    }
                }
            },
            {
                "components": {
                    "StatsComponent": {
                        "health": 60,
                        "maxHealth": 60,
                        "level": 2
                    },
                    "ScriptComponent": {
                        "script": "function onInitialize() print('Custom entity initialized!') end"
                    }
                }
            }
        ]
    })";
    
    Core::ConfigurationManager config;
    if (config.loadFromString(entitiesConfig)) {
        auto entities = factory.createEntitiesFromConfig(config.get("entities"));
        std::cout << "Created " << entities.size() << " entities from configuration" << std::endl;
        
        for (auto entityId : entities) {
            std::cout << "Entity ID: " << entityId << std::endl;
        }
    }
}

void testConfigurationSaveLoad() {
    std::cout << "\n=== Testing Configuration Save/Load ===" << std::endl;
    
    Core::ConfigurationManager config;
    
    // Create some configuration
    config.set("test.string", Core::ConfigValue("Hello World"));
    config.set("test.number", Core::ConfigValue(42));
    config.set("test.boolean", Core::ConfigValue(true));
    config.set("test.float", Core::ConfigValue(3.14f));
    
    // Save to file
    std::string filename = "test_config.json";
    bool saveResult = config.saveToFile(filename);
    std::cout << "Save result: " << (saveResult ? "SUCCESS" : "FAILED") << std::endl;
    
    // Load from file
    Core::ConfigurationManager loadedConfig;
    bool loadResult = loadedConfig.loadFromFile(filename);
    std::cout << "Load result: " << (loadResult ? "SUCCESS" : "FAILED") << std::endl;
    
    if (loadResult) {
        std::cout << "Loaded string: " << loadedConfig.getString("test.string") << std::endl;
        std::cout << "Loaded number: " << loadedConfig.getInt("test.number") << std::endl;
        std::cout << "Loaded boolean: " << (loadedConfig.getBool("test.boolean") ? "true" : "false") << std::endl;
        std::cout << "Loaded float: " << loadedConfig.getFloat("test.float") << std::endl;
    }
    
    // Clean up
    std::remove(filename.c_str());
}

int main() {
    std::cout << "RPG Engine Data-Driven Configuration Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testConfigurationManager();
    testEntityFactory();
    testDataDrivenEntityCreation();
    testConfigurationSaveLoad();

    std::cout << "\nAll configuration tests completed!" << std::endl;
    return 0;
}