#include <iostream>
#include <memory>
#include "../src/core/ConfigurationManager.h"

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

void testConfigurationMerging() {
    std::cout << "\n=== Testing Configuration Merging ===" << std::endl;
    
    Core::ConfigurationManager config1;
    Core::ConfigurationManager config2;
    
    // Setup first config
    config1.loadFromString(R"({
        "game": {
            "title": "Base Game",
            "version": "1.0.0"
        },
        "settings": {
            "audio": true,
            "graphics": "high"
        }
    })");
    
    // Setup second config
    config2.loadFromString(R"({
        "game": {
            "title": "Modded Game",
            "author": "Player"
        },
        "settings": {
            "graphics": "ultra",
            "difficulty": "hard"
        }
    })");
    
    std::cout << "Before merge - Title: " << config1.getString("game.title") << std::endl;
    std::cout << "Before merge - Graphics: " << config1.getString("settings.graphics") << std::endl;
    
    // Merge configs
    config1.merge(config2);
    
    std::cout << "After merge - Title: " << config1.getString("game.title") << std::endl;
    std::cout << "After merge - Author: " << config1.getString("game.author") << std::endl;
    std::cout << "After merge - Graphics: " << config1.getString("settings.graphics") << std::endl;
    std::cout << "After merge - Difficulty: " << config1.getString("settings.difficulty") << std::endl;
    std::cout << "After merge - Audio: " << (config1.getBool("settings.audio") ? "true" : "false") << std::endl;
}

void testConfigurationArrays() {
    std::cout << "\n=== Testing Configuration Arrays ===" << std::endl;
    
    Core::ConfigurationManager config;
    
    std::string arrayConfig = R"({
        "items": [
            {"id": "sword", "damage": 10, "rarity": "common"},
            {"id": "shield", "defense": 5, "rarity": "uncommon"},
            {"id": "potion", "healing": 50, "rarity": "common"}
        ],
        "levels": [1, 5, 10, 15, 20]
    })";
    
    bool result = config.loadFromString(arrayConfig);
    std::cout << "Array config loading: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    
    if (result) {
        Core::ConfigValue items = config.get("items");
        if (items.isArray()) {
            std::cout << "Items array size: " << items.size() << std::endl;
            
            for (size_t i = 0; i < items.size(); ++i) {
                Core::ConfigValue item = items[i];
                if (item.isObject()) {
                    std::cout << "Item " << i << ": " 
                              << item["id"].asString() << " (rarity: " 
                              << item["rarity"].asString() << ")" << std::endl;
                }
            }
        }
        
        Core::ConfigValue levels = config.get("levels");
        if (levels.isArray()) {
            std::cout << "Levels: ";
            for (size_t i = 0; i < levels.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << levels[i].asInt();
            }
            std::cout << std::endl;
        }
    }
}

int main() {
    std::cout << "RPG Engine Data-Driven Configuration Simple Test" << std::endl;
    std::cout << "===============================================" << std::endl;

    testConfigurationManager();
    testConfigurationSaveLoad();
    testConfigurationMerging();
    testConfigurationArrays();

    std::cout << "\nAll configuration tests completed!" << std::endl;
    return 0;
}