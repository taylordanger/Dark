#include "../src/ui/MainMenuUI.h"
#include "../src/ui/GameHUD.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;

// Mock UIRenderer for testing
class MockUIRenderer {
public:
    void beginFrame() {
        std::cout << "  [UIRenderer] Begin frame" << std::endl;
    }
    
    void endFrame() {
        std::cout << "  [UIRenderer] End frame" << std::endl;
    }
    
    std::shared_ptr<UI::UIPanel> drawPanel(const UI::UIRect& bounds, const std::string& id) {
        std::cout << "  [UIRenderer] Drawing panel '" << id << "' at (" << bounds.x << ", " << bounds.y 
                  << ") size (" << bounds.width << ", " << bounds.height << ")" << std::endl;
        return std::make_shared<UI::UIPanel>(id);
    }
    
    bool drawButton(const UI::UIRect& bounds, const std::string& text, const std::string& id) {
        std::cout << "  [UIRenderer] Drawing button '" << text << "' (id: " << id << ") at (" 
                  << bounds.x << ", " << bounds.y << ")" << std::endl;
        return false; // Simulate no clicks for testing
    }
    
    void drawText(const UI::UIRect& bounds, const std::string& text, UI::UIAlignment alignment, 
                  float fontSize, const std::string& id) {
        std::cout << "  [UIRenderer] Drawing text '" << text << "' (id: " << id << ") at (" 
                  << bounds.x << ", " << bounds.y << ") size " << fontSize << std::endl;
    }
    
    void drawProgressBar(const UI::UIRect& bounds, float value, float minValue, float maxValue, const std::string& id) {
        float percentage = (value - minValue) / (maxValue - minValue) * 100.0f;
        std::cout << "  [UIRenderer] Drawing progress bar '" << id << "' at (" << bounds.x << ", " << bounds.y 
                  << ") progress " << percentage << "%" << std::endl;
    }
    
    bool drawCheckbox(const UI::UIRect& bounds, const std::string& label, bool checked, const std::string& id) {
        std::cout << "  [UIRenderer] Drawing checkbox '" << label << "' (id: " << id << ") checked: " 
                  << (checked ? "yes" : "no") << std::endl;
        return checked; // Return current state for testing
    }
    
    float drawSlider(const UI::UIRect& bounds, float value, float minValue, float maxValue, const std::string& id) {
        std::cout << "  [UIRenderer] Drawing slider '" << id << "' at (" << bounds.x << ", " << bounds.y 
                  << ") value " << value << std::endl;
        return value; // Return current value for testing
    }
};

int main() {
    std::cout << "=== Game-Specific UI Test ===" << std::endl;
    
    try {
        // Create mock UI renderer
        auto mockUIRenderer = std::make_shared<MockUIRenderer>();
        
        // Test Main Menu UI structures
        std::cout << "\n--- Testing Main Menu UI Structures ---" << std::endl;
        
        // Test main menu callbacks structure
        UI::MainMenuCallbacks menuCallbacks;
        menuCallbacks.onNewGame = []() { std::cout << "  [Callback] New Game selected" << std::endl; };
        menuCallbacks.onLoadGame = []() { std::cout << "  [Callback] Load Game selected" << std::endl; };
        menuCallbacks.onSettings = []() { std::cout << "  [Callback] Settings selected" << std::endl; };
        menuCallbacks.onCredits = []() { std::cout << "  [Callback] Credits selected" << std::endl; };
        menuCallbacks.onQuit = []() { std::cout << "  [Callback] Quit selected" << std::endl; };
        menuCallbacks.onBack = []() { std::cout << "  [Callback] Back selected" << std::endl; };
        
        std::cout << "✓ Main menu callbacks structure created" << std::endl;
        
        // Test callback execution
        if (menuCallbacks.onNewGame) {
            menuCallbacks.onNewGame();
        }
        if (menuCallbacks.onSettings) {
            menuCallbacks.onSettings();
        }
        
        std::cout << "✓ Main menu callbacks tested" << std::endl;
        
        // Test Game HUD structures
        std::cout << "\n--- Testing Game HUD Structures ---" << std::endl;
        
        // Configure HUD visibility
        UI::HUDVisibility hudVisibility;
        hudVisibility.showHealthBar = true;
        hudVisibility.showManaBar = true;
        hudVisibility.showExperienceBar = true;
        hudVisibility.showInventoryQuickSlots = true;
        hudVisibility.showQuestTracker = true;
        hudVisibility.showStatusEffects = true;
        hudVisibility.showMinimap = true;
        hudVisibility.showClock = true;
        
        std::cout << "✓ HUD visibility structure created" << std::endl;
        
        // Set up quick slots
        std::vector<UI::QuickSlotItem> quickSlots;
        quickSlots.emplace_back("potion_health", "Health Potion", 5, true);
        quickSlots.emplace_back("potion_mana", "Mana Potion", 3, true);
        quickSlots.emplace_back("scroll_fireball", "Fireball Scroll", 1, true);
        quickSlots.emplace_back("", "", 0, false); // Empty slot
        quickSlots.emplace_back("sword_iron", "Iron Sword", 1, false);
        
        std::cout << "✓ Quick slots created (" << quickSlots.size() << " slots)" << std::endl;
        
        // Set up active quest
        UI::HUDQuestInfo questInfo;
        questInfo.questId = "main_quest_001";
        questInfo.questName = "Find the Ancient Artifact";
        questInfo.currentObjective = "Search the old ruins for clues";
        questInfo.completedObjectives = 2;
        questInfo.totalObjectives = 5;
        
        std::cout << "✓ Active quest info created" << std::endl;
        
        // Set up status effects
        std::vector<UI::HUDStatusEffect> statusEffects;
        statusEffects.emplace_back("buff_strength", "Strength Boost", 45.0f, true);
        statusEffects.emplace_back("debuff_poison", "Poison", 12.0f, false);
        statusEffects.emplace_back("buff_speed", "Haste", 30.0f, true);
        
        std::cout << "✓ Status effects created (" << statusEffects.size() << " effects)" << std::endl;
        
        // Test HUD visibility settings
        hudVisibility.showMinimap = false;
        hudVisibility.showClock = false;
        std::cout << "✓ HUD visibility updated (minimap and clock hidden)" << std::endl;
        
        // Test UI structures
        std::cout << "\n--- Testing UI Data Structures ---" << std::endl;
        
        // Test HUD visibility structure
        UI::HUDVisibility testVisibility;
        std::cout << "✓ HUD visibility structure created (default values)" << std::endl;
        std::cout << "  - Health bar: " << (testVisibility.showHealthBar ? "visible" : "hidden") << std::endl;
        std::cout << "  - Mana bar: " << (testVisibility.showManaBar ? "visible" : "hidden") << std::endl;
        std::cout << "  - Quest tracker: " << (testVisibility.showQuestTracker ? "visible" : "hidden") << std::endl;
        
        // Test quick slot item structure
        UI::QuickSlotItem testItem("test_item", "Test Item", 10, true);
        std::cout << "✓ Quick slot item created: " << testItem.itemName << " (qty: " << testItem.quantity << ")" << std::endl;
        
        // Test quest info structure
        UI::HUDQuestInfo testQuest;
        testQuest.questName = "Test Quest";
        testQuest.currentObjective = "Test objective";
        testQuest.completedObjectives = 1;
        testQuest.totalObjectives = 3;
        std::cout << "✓ Quest info created: " << testQuest.questName << " (" 
                  << testQuest.completedObjectives << "/" << testQuest.totalObjectives << ")" << std::endl;
        
        // Test status effect structure
        UI::HUDStatusEffect testEffect("test_effect", "Test Effect", 60.0f, true);
        std::cout << "✓ Status effect created: " << testEffect.effectName << " (time: " 
                  << testEffect.remainingTime << "s, beneficial: " << (testEffect.isBeneficial ? "yes" : "no") << ")" << std::endl;
        
        // Test main menu state enum
        std::cout << "✓ Main menu states:" << std::endl;
        std::cout << "  - MainMenu: " << static_cast<int>(UI::MainMenuState::MainMenu) << std::endl;
        std::cout << "  - NewGame: " << static_cast<int>(UI::MainMenuState::NewGame) << std::endl;
        std::cout << "  - LoadGame: " << static_cast<int>(UI::MainMenuState::LoadGame) << std::endl;
        std::cout << "  - Settings: " << static_cast<int>(UI::MainMenuState::Settings) << std::endl;
        std::cout << "  - Credits: " << static_cast<int>(UI::MainMenuState::Credits) << std::endl;
        
        std::cout << "\n=== Game-Specific UI Test Completed Successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during game-specific UI test: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}