#include <iostream>
#include <cassert>
#include <memory>
#include <thread>
#include <chrono>

#include "../src/save/SaveManager.h"
#include "../src/save/SaveIntegration.h"

using namespace Engine::Save;

void testBasicSaveLoad() {
    std::cout << "Testing basic save/load functionality..." << std::endl;
    
    auto saveManager = std::make_shared<SaveManager>();
    bool result = saveManager->initialize("test_saves_integration");
    assert(result);
    
    // Test basic save/load
    SaveData testData;
    testData.version = "1.0";
    testData.player.stats.level = 5;
    testData.world.currentMap = "test_map";
    
    SaveResult saveResult = saveManager->saveGame(testData, "integration_test");
    assert(saveResult == SaveResult::Success);
    
    SaveData loadedData;
    LoadResult loadResult = saveManager->loadGame(loadedData, "integration_test");
    assert(loadResult == LoadResult::Success);
    assert(loadedData.player.stats.level == 5);
    assert(loadedData.world.currentMap == "test_map");
    
    std::cout << "✓ Basic save/load works correctly" << std::endl;
    
    saveManager->shutdown();
}

void testAutoSaveSettings() {
    std::cout << "Testing auto-save settings..." << std::endl;
    
    auto saveManager = std::make_shared<SaveManager>();
    saveManager->initialize("test_saves_integration");
    
    // Test auto-save configuration
    assert(saveManager->isAutoSaveEnabled());
    
    saveManager->setAutoSaveEnabled(false);
    assert(!saveManager->isAutoSaveEnabled());
    
    saveManager->setAutoSaveInterval(120.0f);
    assert(saveManager->getAutoSaveInterval() == 120.0f);
    
    std::cout << "✓ Auto-save settings work correctly" << std::endl;
    
    saveManager->shutdown();
}

void testMultipleSlots() {
    std::cout << "Testing multiple save slots..." << std::endl;
    
    auto saveManager = std::make_shared<SaveManager>();
    saveManager->initialize("test_saves_integration");
    
    // Test saving to multiple slots
    for (int i = 0; i < 5; ++i) {
        SaveData testData;
        testData.version = "1.0";
        testData.player.stats.level = i + 1;
        testData.world.currentMap = "map_" + std::to_string(i);
        
        SaveResult result = saveManager->saveGame(testData, i);
        assert(result == SaveResult::Success);
    }
    
    // Test loading from multiple slots
    for (int i = 0; i < 5; ++i) {
        SaveData loadedData;
        LoadResult result = saveManager->loadGame(loadedData, i);
        assert(result == LoadResult::Success);
        assert(loadedData.player.stats.level == i + 1);
        assert(loadedData.world.currentMap == "map_" + std::to_string(i));
    }
    
    std::cout << "✓ Multiple save slots work correctly" << std::endl;
    
    saveManager->shutdown();
}

void testSaveSlotInfo() {
    std::cout << "Testing save slot information..." << std::endl;
    
    auto saveManager = std::make_shared<SaveManager>();
    saveManager->initialize("test_saves_integration");
    
    // Create test save
    SaveData testData;
    testData.version = "1.0";
    testData.player.stats.level = 10;
    testData.world.currentMap = "info_test_map";
    
    SaveResult saveResult = saveManager->saveGame(testData, "info_test");
    assert(saveResult == SaveResult::Success);
    
    // Get save info
    SaveManager::SaveInfo info;
    bool hasInfo = saveManager->getSaveInfo("info_test", info);
    assert(hasInfo);
    assert(info.slotName == "info_test");
    assert(info.playerLevel == 10);
    assert(info.currentMap == "info_test_map");
    
    // Get save list
    auto saveList = saveManager->getSaveList();
    assert(!saveList.empty());
    
    std::cout << "✓ Save slot information works correctly" << std::endl;
    
    saveManager->shutdown();
}

int main() {
    std::cout << "=== Save/Load Integration Test Suite ===" << std::endl;
    
    try {
        testBasicSaveLoad();
        testAutoSaveSettings();
        testMultipleSlots();
        testSaveSlotInfo();
        
        std::cout << "\n✅ All save/load integration tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}