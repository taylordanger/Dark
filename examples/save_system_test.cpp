#include <iostream>
#include <cassert>
#include "../src/save/SaveManager.h"
#include "../src/save/JsonSaveSerializer.h"

using namespace Engine::Save;

void testBasicSaveLoad() {
    std::cout << "Testing basic save/load functionality..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Create test save data
    SaveData testData;
    testData.version = "1.0";
    testData.player.position.x = 100.0f;
    testData.player.position.y = 200.0f;
    testData.player.stats.hp = 80;
    testData.player.stats.maxHp = 100;
    testData.player.stats.level = 5;
    testData.player.stats.experience = 1250;
    
    // Add inventory items
    PlayerData::InventoryItem sword;
    sword.id = "iron_sword";
    sword.quantity = 1;
    testData.player.inventory.push_back(sword);
    
    PlayerData::InventoryItem potion;
    potion.id = "health_potion";
    potion.quantity = 3;
    testData.player.inventory.push_back(potion);
    
    // Add world data
    testData.world.currentMap = "town_center";
    testData.world.completedQuests.push_back("tutorial");
    testData.world.completedQuests.push_back("first_quest");
    testData.world.gameFlags["met_wizard"] = true;
    testData.world.gameFlags["has_key"] = false;
    
    // Test save
    SaveResult saveResult = saveManager.saveGame(testData, "test_slot");
    assert(saveResult == SaveResult::Success);
    std::cout << "✓ Save successful" << std::endl;
    
    // Test load
    SaveData loadedData;
    LoadResult loadResult = saveManager.loadGame(loadedData, "test_slot");
    assert(loadResult == LoadResult::Success);
    std::cout << "✓ Load successful" << std::endl;
    
    // Verify data integrity
    assert(loadedData.version == testData.version);
    assert(loadedData.player.position.x == testData.player.position.x);
    assert(loadedData.player.position.y == testData.player.position.y);
    assert(loadedData.player.stats.hp == testData.player.stats.hp);
    assert(loadedData.player.stats.level == testData.player.stats.level);
    assert(loadedData.player.inventory.size() == 2);
    assert(loadedData.world.currentMap == testData.world.currentMap);
    assert(loadedData.world.completedQuests.size() == 2);
    assert(loadedData.world.gameFlags["met_wizard"] == true);
    std::cout << "✓ Data integrity verified" << std::endl;
    
    saveManager.shutdown();
}

void testVersioning() {
    std::cout << "Testing save file versioning..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Test with different version
    SaveData testData;
    testData.version = "2.0";
    testData.player.stats.level = 10;
    
    SaveResult saveResult = saveManager.saveGame(testData, "version_test");
    assert(saveResult == SaveResult::Success);
    
    SaveData loadedData;
    LoadResult loadResult = saveManager.loadGame(loadedData, "version_test");
    assert(loadResult == LoadResult::Success);
    assert(loadedData.version == "2.0");
    std::cout << "✓ Version handling works" << std::endl;
    
    saveManager.shutdown();
}

void testValidation() {
    std::cout << "Testing save data validation..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Test invalid data
    SaveData invalidData;
    invalidData.version = ""; // Empty version should fail
    
    SaveResult result = saveManager.saveGame(invalidData, "invalid_test");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ Empty version validation works" << std::endl;
    
    // Test invalid HP
    SaveData invalidHpData;
    invalidHpData.version = "1.0";
    invalidHpData.player.stats.hp = -10; // Negative HP should fail
    
    result = saveManager.saveGame(invalidHpData, "invalid_hp_test");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ Invalid HP validation works" << std::endl;
    
    // Test invalid level
    SaveData invalidLevelData;
    invalidLevelData.version = "1.0";
    invalidLevelData.player.stats.level = 0; // Level < 1 should fail
    
    result = saveManager.saveGame(invalidLevelData, "invalid_level_test");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ Invalid level validation works" << std::endl;
    
    saveManager.shutdown();
}

void testMultipleSlots() {
    std::cout << "Testing multiple save slots..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Create different save data for different slots
    for (int i = 0; i < 3; ++i) {
        SaveData slotData;
        slotData.version = "1.0";
        slotData.player.stats.level = i + 1;
        slotData.world.currentMap = "map_" + std::to_string(i);
        
        SaveResult result = saveManager.saveGame(slotData, i);
        assert(result == SaveResult::Success);
        
        // Verify slot exists
        assert(saveManager.saveExists(i));
    }
    std::cout << "✓ Multiple slots created successfully" << std::endl;
    
    // Load and verify each slot
    for (int i = 0; i < 3; ++i) {
        SaveData loadedData;
        LoadResult result = saveManager.loadGame(loadedData, i);
        assert(result == LoadResult::Success);
        assert(loadedData.player.stats.level == i + 1);
        assert(loadedData.world.currentMap == "map_" + std::to_string(i));
    }
    std::cout << "✓ Multiple slots loaded and verified" << std::endl;
    
    saveManager.shutdown();
}

void testAutoSave() {
    std::cout << "Testing auto-save functionality..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    assert(saveManager.isAutoSaveEnabled());
    
    SaveData autoSaveData;
    autoSaveData.version = "1.0";
    autoSaveData.player.stats.level = 99;
    autoSaveData.world.currentMap = "final_dungeon";
    
    SaveResult result = saveManager.autoSave(autoSaveData);
    assert(result == SaveResult::Success);
    std::cout << "✓ Auto-save successful" << std::endl;
    
    SaveData loadedData;
    LoadResult loadResult = saveManager.loadAutoSave(loadedData);
    assert(loadResult == LoadResult::Success);
    assert(loadedData.player.stats.level == 99);
    assert(loadedData.world.currentMap == "final_dungeon");
    std::cout << "✓ Auto-save load successful" << std::endl;
    
    saveManager.shutdown();
}

void testSaveInfo() {
    std::cout << "Testing save info functionality..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    SaveData testData;
    testData.version = "1.0";
    testData.player.stats.level = 42;
    testData.world.currentMap = "test_map";
    
    SaveResult result = saveManager.saveGame(testData, "info_test");
    assert(result == SaveResult::Success);
    
    SaveManager::SaveInfo info;
    bool hasInfo = saveManager.getSaveInfo("info_test", info);
    assert(hasInfo);
    assert(info.slotName == "info_test");
    assert(info.version == "1.0");
    assert(info.playerLevel == 42);
    assert(info.currentMap == "test_map");
    assert(!info.timestamp.empty());
    std::cout << "✓ Save info retrieval works" << std::endl;
    
    auto saveList = saveManager.getSaveList();
    assert(!saveList.empty());
    std::cout << "✓ Save list retrieval works (found " << saveList.size() << " saves)" << std::endl;
    
    saveManager.shutdown();
}

void testErrorHandling() {
    std::cout << "Testing error handling..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Test loading non-existent save
    SaveData loadData;
    LoadResult result = saveManager.loadGame(loadData, "non_existent");
    assert(result == LoadResult::FileNotFound);
    assert(!saveManager.getLastError().empty());
    std::cout << "✓ Non-existent file error handling works" << std::endl;
    
    // Test invalid slot number
    SaveData testData;
    testData.version = "1.0";
    SaveResult saveResult = saveManager.saveGame(testData, -1);
    assert(saveResult == SaveResult::ValidationError);
    std::cout << "✓ Invalid slot number error handling works" << std::endl;
    
    saveManager.shutdown();
}

void testEnhancedValidation() {
    std::cout << "Testing enhanced validation..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Test unsupported version
    SaveData unsupportedVersionData;
    unsupportedVersionData.version = "99.0";
    SaveResult result = saveManager.saveGame(unsupportedVersionData, "unsupported_version");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ Unsupported version validation works" << std::endl;
    
    // Test invalid level range
    SaveData invalidLevelData;
    invalidLevelData.version = "1.0";
    invalidLevelData.player.stats.level = 1000; // Too high
    result = saveManager.saveGame(invalidLevelData, "invalid_level_high");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ High level validation works" << std::endl;
    
    // Test invalid inventory item
    SaveData invalidInventoryData;
    invalidInventoryData.version = "1.0";
    invalidInventoryData.player.stats.level = 1;
    PlayerData::InventoryItem invalidItem;
    invalidItem.id = ""; // Empty ID should fail
    invalidItem.quantity = 1;
    invalidInventoryData.player.inventory.push_back(invalidItem);
    result = saveManager.saveGame(invalidInventoryData, "invalid_inventory");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ Empty inventory item ID validation works" << std::endl;
    
    // Test empty current map
    SaveData emptyMapData;
    emptyMapData.version = "1.0";
    emptyMapData.player.stats.level = 1;
    emptyMapData.world.currentMap = ""; // Empty map should fail
    result = saveManager.saveGame(emptyMapData, "empty_map");
    assert(result == SaveResult::ValidationError);
    std::cout << "✓ Empty current map validation works" << std::endl;
    
    saveManager.shutdown();
}

void testChecksumValidation() {
    std::cout << "Testing checksum validation..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    assert(saveManager.isChecksumValidationEnabled());
    
    // Create valid save data
    SaveData testData;
    testData.version = "1.0";
    testData.player.stats.level = 5;
    testData.world.currentMap = "test_map";
    
    SaveResult saveResult = saveManager.saveGame(testData, "checksum_test");
    assert(saveResult == SaveResult::Success);
    std::cout << "✓ Save with checksum successful" << std::endl;
    
    // Load and verify
    SaveData loadedData;
    LoadResult loadResult = saveManager.loadGame(loadedData, "checksum_test");
    assert(loadResult == LoadResult::Success);
    assert(loadedData.player.stats.level == 5);
    std::cout << "✓ Load with checksum verification successful" << std::endl;
    
    saveManager.shutdown();
}

void testVersionMigration() {
    std::cout << "Testing version migration..." << std::endl;
    
    SaveManager saveManager;
    assert(saveManager.initialize("test_saves"));
    
    // Create v1.0 save data
    SaveData v1Data;
    v1Data.version = "1.0";
    v1Data.player.stats.level = 10;
    v1Data.world.currentMap = "old_map";
    
    // Migrate to v2.0
    bool migrationResult = saveManager.migrateSaveData(v1Data, "1.0", "2.0");
    assert(migrationResult);
    assert(v1Data.version == "2.0");
    assert(v1Data.world.discoveredLocations["starting_town"] == true);
    std::cout << "✓ Version migration from 1.0 to 2.0 works" << std::endl;
    
    // Test unsupported migration
    SaveData unsupportedData;
    unsupportedData.version = "1.0";
    bool unsupportedMigration = saveManager.migrateSaveData(unsupportedData, "1.0", "99.0");
    assert(!unsupportedMigration);
    std::cout << "✓ Unsupported migration properly rejected" << std::endl;
    
    saveManager.shutdown();
}

int main() {
    std::cout << "=== Save System Test Suite ===" << std::endl;
    
    try {
        testBasicSaveLoad();
        testVersioning();
        testValidation();
        testMultipleSlots();
        testAutoSave();
        testSaveInfo();
        testErrorHandling();
        testEnhancedValidation();
        testChecksumValidation();
        testVersionMigration();
        
        std::cout << "\n✅ All save system tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}