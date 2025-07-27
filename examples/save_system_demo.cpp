#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <string>

#include "../src/save/SaveManager.h"
#include "../src/save/SaveIntegration.h"

using namespace Engine::Save;

/**
 * Comprehensive save system demonstration
 * Shows all features of the save/load integration
 */
class SaveSystemDemo {
public:
    SaveSystemDemo() = default;
    ~SaveSystemDemo() = default;
    
    void run() {
        std::cout << "=== RPG Engine Save System Demo ===" << std::endl;
        
        if (!initialize()) {
            std::cerr << "Failed to initialize save system!" << std::endl;
            return;
        }
        
        // Demonstrate basic save/load operations
        demonstrateBasicOperations();
        
        // Demonstrate multiple save slots
        demonstrateMultipleSlots();
        
        // Demonstrate auto-save functionality
        demonstrateAutoSave();
        
        // Demonstrate save file management
        demonstrateSaveFileManagement();
        
        // Demonstrate error handling
        demonstrateErrorHandling();
        
        shutdown();
        std::cout << "\n✅ Save system demo completed successfully!" << std::endl;
    }
    
private:
    bool initialize() {
        std::cout << "\n--- Initializing Save System ---" << std::endl;
        
        m_saveManager = std::make_shared<SaveManager>();
        if (!m_saveManager->initialize("demo_saves")) {
            std::cerr << "Failed to initialize SaveManager" << std::endl;
            return false;
        }
        
        std::cout << "✓ Save system initialized successfully" << std::endl;
        return true;
    }
    
    void shutdown() {
        std::cout << "\n--- Shutting Down Save System ---" << std::endl;
        
        if (m_saveManager) {
            m_saveManager->shutdown();
        }
        
        std::cout << "✓ Save system shutdown complete" << std::endl;
    }
    
    void demonstrateBasicOperations() {
        std::cout << "\n--- Basic Save/Load Operations ---" << std::endl;
        
        // Create sample game data
        SaveData gameData;
        gameData.version = "1.0";
        gameData.player.stats.level = 15;
        gameData.player.stats.hp = 85;
        gameData.player.stats.maxHp = 100;
        gameData.player.stats.mp = 45;
        gameData.player.stats.maxMp = 60;
        gameData.player.stats.experience = 2500;
        gameData.player.position.x = 150.5f;
        gameData.player.position.y = 200.3f;
        
        // Add some inventory items
        gameData.player.inventory.push_back({"sword_iron", 1});
        gameData.player.inventory.push_back({"potion_health", 5});
        gameData.player.inventory.push_back({"key_bronze", 2});
        
        // Add equipment
        gameData.player.equipment["weapon"] = "sword_iron";
        gameData.player.equipment["armor"] = "leather_armor";
        
        // Set world data
        gameData.world.currentMap = "forest_clearing";
        gameData.world.completedQuests.push_back("tutorial");
        gameData.world.completedQuests.push_back("first_monster");
        gameData.world.gameFlags["met_wizard"] = true;
        gameData.world.gameFlags["found_secret_cave"] = false;
        gameData.world.discoveredLocations["town_center"] = true;
        gameData.world.discoveredLocations["forest_clearing"] = true;
        
        std::cout << "Created sample game data:" << std::endl;
        printGameData(gameData);
        
        // Save the game
        std::cout << "\nSaving game to slot 'demo_basic'..." << std::endl;
        SaveResult saveResult = m_saveManager->saveGame(gameData, "demo_basic");
        
        if (saveResult == SaveResult::Success) {
            std::cout << "✓ Game saved successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to save game: " << static_cast<int>(saveResult) << std::endl;
            return;
        }
        
        // Load the game
        std::cout << "\nLoading game from slot 'demo_basic'..." << std::endl;
        SaveData loadedData;
        LoadResult loadResult = m_saveManager->loadGame(loadedData, "demo_basic");
        
        if (loadResult == LoadResult::Success) {
            std::cout << "✓ Game loaded successfully" << std::endl;
            std::cout << "Loaded game data:" << std::endl;
            printGameData(loadedData);
            
            // Verify data integrity
            if (verifyGameData(gameData, loadedData)) {
                std::cout << "✓ Data integrity verified" << std::endl;
            } else {
                std::cout << "✗ Data integrity check failed" << std::endl;
            }
        } else {
            std::cout << "✗ Failed to load game: " << static_cast<int>(loadResult) << std::endl;
        }
    }
    
    void demonstrateMultipleSlots() {
        std::cout << "\n--- Multiple Save Slots ---" << std::endl;
        
        // Create different game states for different slots
        for (int i = 0; i < 5; ++i) {
            SaveData gameData;
            gameData.version = "1.0";
            gameData.player.stats.level = 10 + i * 5;
            gameData.player.stats.hp = 80 + i * 10;
            gameData.player.stats.maxHp = 100 + i * 10;
            gameData.player.position.x = 100.0f + i * 50.0f;
            gameData.player.position.y = 200.0f + i * 30.0f;
            gameData.world.currentMap = "area_" + std::to_string(i + 1);
            
            std::string slotName = "slot_" + std::to_string(i);
            std::cout << "Saving to " << slotName << " (Level " << gameData.player.stats.level << ")..." << std::endl;
            
            SaveResult result = m_saveManager->saveGame(gameData, slotName);
            if (result == SaveResult::Success) {
                std::cout << "  ✓ Saved successfully" << std::endl;
            } else {
                std::cout << "  ✗ Save failed" << std::endl;
            }
        }
        
        // List all save files
        std::cout << "\nListing all save files:" << std::endl;
        auto saveList = m_saveManager->getSaveList();
        
        for (const auto& saveInfo : saveList) {
            std::cout << "  " << saveInfo.slotName << ": Level " << saveInfo.playerLevel 
                      << ", Map: " << saveInfo.currentMap 
                      << ", Size: " << saveInfo.fileSize << " bytes" << std::endl;
        }
        
        // Load from a specific slot
        std::cout << "\nLoading from slot_2..." << std::endl;
        SaveData loadedData;
        LoadResult loadResult = m_saveManager->loadGame(loadedData, "slot_2");
        
        if (loadResult == LoadResult::Success) {
            std::cout << "✓ Loaded: Level " << loadedData.player.stats.level 
                      << ", Map: " << loadedData.world.currentMap << std::endl;
        }
    }
    
    void demonstrateAutoSave() {
        std::cout << "\n--- Auto-Save Functionality ---" << std::endl;
        
        // Configure auto-save
        std::cout << "Configuring auto-save..." << std::endl;
        m_saveManager->setAutoSaveEnabled(true);
        m_saveManager->setAutoSaveInterval(2.0f); // 2 seconds for demo
        
        std::cout << "Auto-save enabled: " << (m_saveManager->isAutoSaveEnabled() ? "Yes" : "No") << std::endl;
        std::cout << "Auto-save interval: " << m_saveManager->getAutoSaveInterval() << " seconds" << std::endl;
        
        // Create game data that changes over time
        SaveData gameData;
        gameData.version = "1.0";
        gameData.player.stats.level = 1;
        gameData.world.currentMap = "starting_area";
        
        // Simulate game progression with auto-saves
        std::cout << "\nSimulating game progression with auto-saves..." << std::endl;
        
        for (int i = 0; i < 6; ++i) {
            // Update game state
            gameData.player.stats.level = 1 + i;
            gameData.player.stats.experience = i * 100;
            gameData.world.currentMap = "area_" + std::to_string(i + 1);
            
            std::cout << "Game state: Level " << gameData.player.stats.level 
                      << ", Map: " << gameData.world.currentMap << std::endl;
            
            // Manual save to current state
            m_saveManager->saveGame(gameData, "current_state");
            
            // Trigger auto-save every few iterations
            if (i % 2 == 0) {
                std::cout << "  → Auto-save triggered!" << std::endl;
                SaveResult autoSaveResult = m_saveManager->autoSave(gameData);
                if (autoSaveResult == SaveResult::Success) {
                    std::cout << "  ✓ Auto-save completed" << std::endl;
                } else {
                    std::cout << "  ✗ Auto-save failed" << std::endl;
                }
            } else {
                std::cout << "  Auto-save not triggered this iteration" << std::endl;
            }
            
            // Wait a bit to simulate game time
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Show auto-save files
        std::cout << "\nAuto-save files created:" << std::endl;
        auto saveList = m_saveManager->getSaveList();
        
        for (const auto& saveInfo : saveList) {
            if (saveInfo.slotName.find("autosave") != std::string::npos) {
                std::cout << "  " << saveInfo.slotName << ": Level " << saveInfo.playerLevel 
                          << ", Map: " << saveInfo.currentMap << std::endl;
            }
        }
    }
    
    void demonstrateSaveFileManagement() {
        std::cout << "\n--- Save File Management ---" << std::endl;
        
        // Get detailed save information
        std::cout << "Detailed save file information:" << std::endl;
        auto saveList = m_saveManager->getSaveList();
        
        for (const auto& saveInfo : saveList) {
            std::cout << "  Slot: " << saveInfo.slotName << std::endl;
            std::cout << "    Player Level: " << saveInfo.playerLevel << std::endl;
            std::cout << "    Current Map: " << saveInfo.currentMap << std::endl;
            std::cout << "    Timestamp: " << saveInfo.timestamp << std::endl;
            std::cout << "    File Size: " << saveInfo.fileSize << " bytes" << std::endl;
            std::cout << std::endl;
        }
        
        // Test save file validation
        std::cout << "Testing save file validation..." << std::endl;
        
        SaveManager::SaveInfo info;
        if (m_saveManager->getSaveInfo("demo_basic", info)) {
            std::cout << "✓ Save file 'demo_basic' is valid" << std::endl;
        } else {
            std::cout << "✗ Save file 'demo_basic' is invalid or not found" << std::endl;
        }
        
        if (m_saveManager->getSaveInfo("nonexistent_save", info)) {
            std::cout << "✗ Nonexistent save file reported as valid" << std::endl;
        } else {
            std::cout << "✓ Nonexistent save file correctly reported as invalid" << std::endl;
        }
        
        // Test save file deletion
        std::cout << "\nTesting save file deletion..." << std::endl;
        
        // Create a temporary save to delete
        SaveData tempData;
        tempData.version = "1.0";
        tempData.player.stats.level = 99;
        tempData.world.currentMap = "temp_area";
        
        m_saveManager->saveGame(tempData, "temp_delete_me");
        std::cout << "Created temporary save 'temp_delete_me'" << std::endl;
        
        // Verify it exists
        if (m_saveManager->getSaveInfo("temp_delete_me", info)) {
            std::cout << "✓ Temporary save exists" << std::endl;
            
            // Delete it
            if (m_saveManager->deleteSave("temp_delete_me")) {
                std::cout << "✓ Temporary save deleted successfully" << std::endl;
                
                // Verify it's gone
                if (!m_saveManager->getSaveInfo("temp_delete_me", info)) {
                    std::cout << "✓ Temporary save no longer exists" << std::endl;
                } else {
                    std::cout << "✗ Temporary save still exists after deletion" << std::endl;
                }
            } else {
                std::cout << "✗ Failed to delete temporary save" << std::endl;
            }
        }
    }
    
    void demonstrateErrorHandling() {
        std::cout << "\n--- Error Handling ---" << std::endl;
        
        // Test loading from non-existent save
        std::cout << "Testing load from non-existent save..." << std::endl;
        SaveData loadData;
        LoadResult result = m_saveManager->loadGame(loadData, "does_not_exist");
        
        if (result == LoadResult::FileNotFound) {
            std::cout << "✓ Correctly handled non-existent save file" << std::endl;
        } else {
            std::cout << "✗ Unexpected result for non-existent save: " << static_cast<int>(result) << std::endl;
        }
        
        // Test invalid save data
        std::cout << "\nTesting invalid save data handling..." << std::endl;
        SaveData invalidData;
        invalidData.version = ""; // Invalid version
        
        SaveResult saveResult = m_saveManager->saveGame(invalidData, "invalid_test");
        if (saveResult != SaveResult::Success) {
            std::cout << "✓ Correctly rejected invalid save data" << std::endl;
        } else {
            std::cout << "✗ Accepted invalid save data" << std::endl;
        }
        
        // Test error messages
        std::cout << "\nTesting error message reporting..." << std::endl;
        std::string lastError = m_saveManager->getLastError();
        if (!lastError.empty()) {
            std::cout << "Last error message: " << lastError << std::endl;
        } else {
            std::cout << "No error message available" << std::endl;
        }
    }
    
    void printGameData(const SaveData& data) {
        std::cout << "  Version: " << data.version << std::endl;
        std::cout << "  Player Level: " << data.player.stats.level << std::endl;
        std::cout << "  Player HP: " << data.player.stats.hp << "/" << data.player.stats.maxHp << std::endl;
        std::cout << "  Player MP: " << data.player.stats.mp << "/" << data.player.stats.maxMp << std::endl;
        std::cout << "  Player Experience: " << data.player.stats.experience << std::endl;
        std::cout << "  Player Position: (" << data.player.position.x << ", " << data.player.position.y << ")" << std::endl;
        std::cout << "  Current Map: " << data.world.currentMap << std::endl;
        std::cout << "  Inventory Items: " << data.player.inventory.size() << std::endl;
        std::cout << "  Equipment Items: " << data.player.equipment.size() << std::endl;
        std::cout << "  Completed Quests: " << data.world.completedQuests.size() << std::endl;
        std::cout << "  Game Flags: " << data.world.gameFlags.size() << std::endl;
    }
    
    bool verifyGameData(const SaveData& original, const SaveData& loaded) {
        return original.version == loaded.version &&
               original.player.stats.level == loaded.player.stats.level &&
               original.player.stats.hp == loaded.player.stats.hp &&
               original.player.stats.maxHp == loaded.player.stats.maxHp &&
               original.player.stats.mp == loaded.player.stats.mp &&
               original.player.stats.maxMp == loaded.player.stats.maxMp &&
               original.player.stats.experience == loaded.player.stats.experience &&
               original.player.position.x == loaded.player.position.x &&
               original.player.position.y == loaded.player.position.y &&
               original.world.currentMap == loaded.world.currentMap &&
               original.player.inventory.size() == loaded.player.inventory.size() &&
               original.player.equipment.size() == loaded.player.equipment.size() &&
               original.world.completedQuests.size() == loaded.world.completedQuests.size() &&
               original.world.gameFlags.size() == loaded.world.gameFlags.size();
    }
    
    std::shared_ptr<SaveManager> m_saveManager;
};

int main() {
    try {
        SaveSystemDemo demo;
        demo.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Demo failed with unknown exception" << std::endl;
        return 1;
    }
}