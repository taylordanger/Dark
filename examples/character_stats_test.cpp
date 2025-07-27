#include "../src/components/StatsComponent.h"
#include "../src/entities/EntityManager.h"
#include <iostream>
#include <cassert>

using namespace RPGEngine;
using namespace RPGEngine::Components;

void testBasicStats() {
    std::cout << "\n=== Testing Basic Stats ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    StatsComponent stats(player.getID());
    
    // Test initial values
    assert(stats.getCurrentHP() == 100.0f);
    assert(stats.getMaxHP() == 150.0f); // 100 base + (10 vitality * 5)
    assert(stats.getCurrentMP() == 50.0f);
    assert(stats.getMaxMP() == 80.0f); // 50 base + (10 intelligence * 3)
    assert(stats.getLevel() == 1);
    assert(stats.getCurrentExperience() == 0);
    
    std::cout << "Initial HP: " << stats.getCurrentHP() << "/" << stats.getMaxHP() << std::endl;
    std::cout << "Initial MP: " << stats.getCurrentMP() << "/" << stats.getMaxMP() << std::endl;
    std::cout << "Initial Level: " << stats.getLevel() << std::endl;
    
    // Test HP/MP modification
    float hpChange = stats.modifyHP(-30.0f);
    assert(hpChange == -30.0f);
    assert(stats.getCurrentHP() == 70.0f);
    
    float mpChange = stats.modifyMP(-20.0f);
    assert(mpChange == -20.0f);
    assert(stats.getCurrentMP() == 30.0f);
    
    std::cout << "After damage - HP: " << stats.getCurrentHP() << "/" << stats.getMaxHP() << std::endl;
    std::cout << "After spell - MP: " << stats.getCurrentMP() << "/" << stats.getMaxMP() << std::endl;
    
    // Test healing
    stats.modifyHP(50.0f);
    assert(stats.getCurrentHP() == 120.0f);
    
    stats.modifyMP(100.0f); // Should be clamped to max
    assert(stats.getCurrentMP() == stats.getMaxMP());
    
    std::cout << "After healing - HP: " << stats.getCurrentHP() << "/" << stats.getMaxHP() << std::endl;
    std::cout << "After mana restore - MP: " << stats.getCurrentMP() << "/" << stats.getMaxMP() << std::endl;
    
    std::cout << "Basic stats test passed!" << std::endl;
}

void testAttributes() {
    std::cout << "\n=== Testing Attributes ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    StatsComponent stats(player.getID());
    
    // Test initial attributes
    assert(stats.getBaseAttribute(AttributeType::Strength) == 10);
    assert(stats.getAttribute(AttributeType::Strength) == 10);
    
    std::cout << "Initial Strength: " << stats.getAttribute(AttributeType::Strength) << std::endl;
    std::cout << "Initial Dexterity: " << stats.getAttribute(AttributeType::Dexterity) << std::endl;
    std::cout << "Initial Intelligence: " << stats.getAttribute(AttributeType::Intelligence) << std::endl;
    std::cout << "Initial Vitality: " << stats.getAttribute(AttributeType::Vitality) << std::endl;
    
    // Test attribute modification
    stats.setBaseAttribute(AttributeType::Strength, 15);
    assert(stats.getBaseAttribute(AttributeType::Strength) == 15);
    assert(stats.getAttribute(AttributeType::Strength) == 15);
    
    stats.modifyBaseAttribute(AttributeType::Dexterity, 5);
    assert(stats.getBaseAttribute(AttributeType::Dexterity) == 15);
    
    std::cout << "After modification - Strength: " << stats.getAttribute(AttributeType::Strength) << std::endl;
    std::cout << "After modification - Dexterity: " << stats.getAttribute(AttributeType::Dexterity) << std::endl;
    
    // Test derived stats
    std::cout << "Attack Power: " << stats.getAttackPower() << std::endl;
    std::cout << "Magic Power: " << stats.getMagicPower() << std::endl;
    std::cout << "Defense: " << stats.getDefense() << std::endl;
    std::cout << "Accuracy: " << stats.getAccuracy() << std::endl;
    std::cout << "Critical Chance: " << (stats.getCriticalChance() * 100.0f) << "%" << std::endl;
    std::cout << "Movement Speed: " << stats.getMovementSpeed() << std::endl;
    
    std::cout << "Attributes test passed!" << std::endl;
}

void testExperienceAndLeveling() {
    std::cout << "\n=== Testing Experience and Leveling ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    StatsComponent stats(player.getID());
    
    // Set level up callback
    stats.setLevelUpCallback([](int oldLevel, int newLevel) {
        std::cout << "LEVEL UP! " << oldLevel << " -> " << newLevel << std::endl;
    });
    
    // Test initial experience
    assert(stats.getLevel() == 1);
    assert(stats.getCurrentExperience() == 0);
    assert(stats.getExperienceToNextLevel() == 100);
    
    std::cout << "Initial Level: " << stats.getLevel() << std::endl;
    std::cout << "Experience to next level: " << stats.getExperienceToNextLevel() << std::endl;
    
    // Add experience (not enough to level up)
    bool leveledUp = stats.addExperience(50);
    assert(!leveledUp);
    assert(stats.getLevel() == 1);
    assert(stats.getCurrentExperience() == 50);
    assert(stats.getExperienceToNextLevel() == 50);
    
    std::cout << "After 50 exp - Level: " << stats.getLevel() 
              << ", Exp: " << stats.getCurrentExperience() 
              << ", To next: " << stats.getExperienceToNextLevel() << std::endl;
    
    // Add enough experience to level up
    leveledUp = stats.addExperience(60);
    assert(leveledUp);
    assert(stats.getLevel() == 2);
    assert(stats.getCurrentExperience() == 10); // 50 + 60 - 100 = 10
    
    std::cout << "After level up - Level: " << stats.getLevel() 
              << ", Exp: " << stats.getCurrentExperience() 
              << ", To next: " << stats.getExperienceToNextLevel() << std::endl;
    
    // Test multiple level ups
    leveledUp = stats.addExperience(500);
    assert(leveledUp);
    assert(stats.getLevel() > 2);
    
    std::cout << "After big exp gain - Level: " << stats.getLevel() 
              << ", Exp: " << stats.getCurrentExperience() 
              << ", To next: " << stats.getExperienceToNextLevel() << std::endl;
    
    // Test experience percentage
    float expPercentage = stats.getExperiencePercentage();
    std::cout << "Experience progress: " << (expPercentage * 100.0f) << "%" << std::endl;
    
    std::cout << "Experience and leveling test passed!" << std::endl;
}

void testStatModifiers() {
    std::cout << "\n=== Testing Stat Modifiers ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    StatsComponent stats(player.getID());
    
    // Test initial HP
    float initialMaxHP = stats.getMaxHP();
    std::cout << "Initial Max HP: " << initialMaxHP << std::endl;
    
    // Add flat HP modifier
    StatModifier hpBonus("hp_potion", "consumable", ModifierType::Flat, 50.0f, 30.0f);
    stats.addModifier("hp", hpBonus);
    
    float modifiedMaxHP = stats.getMaxHP();
    std::cout << "Max HP with flat bonus: " << modifiedMaxHP << std::endl;
    assert(modifiedMaxHP == initialMaxHP + 50.0f);
    
    // Add percentage HP modifier
    StatModifier hpPercentage("vitality_ring", "equipment", ModifierType::Percentage, 20.0f);
    stats.addModifier("hp", hpPercentage);
    
    float percentageMaxHP = stats.getMaxHP();
    std::cout << "Max HP with percentage bonus: " << percentageMaxHP << std::endl;
    
    // Add multiplier HP modifier
    StatModifier hpMultiplier("berserker_rage", "spell", ModifierType::Multiplier, 1.5f, 10.0f);
    stats.addModifier("hp", hpMultiplier);
    
    float multipliedMaxHP = stats.getMaxHP();
    std::cout << "Max HP with multiplier: " << multipliedMaxHP << std::endl;
    
    // Test attribute modifiers
    int initialStrength = stats.getAttribute(AttributeType::Strength);
    std::cout << "Initial Strength: " << initialStrength << std::endl;
    
    StatModifier strengthBonus("power_gauntlets", "equipment", ModifierType::Flat, 5.0f);
    stats.addModifier("strength", strengthBonus);
    
    int modifiedStrength = stats.getAttribute(AttributeType::Strength);
    std::cout << "Strength with equipment: " << modifiedStrength << std::endl;
    assert(modifiedStrength == initialStrength + 5);
    
    // Test modifier removal
    bool removed = stats.removeModifier("hp", "hp_potion");
    assert(removed);
    
    float hpAfterRemoval = stats.getMaxHP();
    std::cout << "Max HP after removing potion: " << hpAfterRemoval << std::endl;
    
    // Test removing modifiers by source
    stats.removeModifiersFromSource("equipment");
    
    int strengthAfterRemoval = stats.getAttribute(AttributeType::Strength);
    std::cout << "Strength after removing equipment: " << strengthAfterRemoval << std::endl;
    assert(strengthAfterRemoval == initialStrength);
    
    // Test modifier expiration
    StatModifier tempBonus("temp_buff", "spell", ModifierType::Flat, 10.0f, 0.1f);
    stats.addModifier("strength", tempBonus);
    
    int strengthWithTemp = stats.getAttribute(AttributeType::Strength);
    std::cout << "Strength with temp buff: " << strengthWithTemp << std::endl;
    
    // Update modifiers to expire the temporary one
    stats.updateModifiers(0.2f);
    
    int strengthAfterExpiry = stats.getAttribute(AttributeType::Strength);
    std::cout << "Strength after temp buff expires: " << strengthAfterExpiry << std::endl;
    assert(strengthAfterExpiry == initialStrength);
    
    std::cout << "Stat modifiers test passed!" << std::endl;
}

void testCallbacks() {
    std::cout << "\n=== Testing Callbacks ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    StatsComponent stats(player.getID());
    
    bool deathCalled = false;
    bool statChangeCalled = false;
    std::string lastChangedStat;
    
    // Set callbacks
    stats.setDeathCallback([&deathCalled]() {
        std::cout << "Death callback triggered!" << std::endl;
        deathCalled = true;
    });
    
    stats.setStatChangeCallback([&statChangeCalled, &lastChangedStat](const std::string& stat) {
        std::cout << "Stat changed: " << stat << std::endl;
        statChangeCalled = true;
        lastChangedStat = stat;
    });
    
    // Test stat change callback
    stats.modifyHP(-10.0f);
    assert(statChangeCalled);
    assert(lastChangedStat == "hp");
    
    // Reset flags
    statChangeCalled = false;
    lastChangedStat.clear();
    
    // Test death callback
    stats.setCurrentHP(0.0f);
    assert(deathCalled);
    assert(!stats.isAlive());
    
    std::cout << "Callbacks test passed!" << std::endl;
}

void testSerialization() {
    std::cout << "\n=== Testing Serialization ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    StatsComponent stats1(player.getID());
    
    // Modify stats
    stats1.setBaseMaxHP(200.0f);
    stats1.setLevel(5);
    stats1.addExperience(50);
    stats1.setBaseAttribute(AttributeType::Strength, 20);
    stats1.setCurrentHP(150.0f); // Set HP after level changes
    
    // Add modifiers
    StatModifier modifier("test_mod", "test", ModifierType::Flat, 10.0f);
    stats1.addModifier("hp", modifier);
    
    // Serialize
    std::string serialized = stats1.serialize();
    std::cout << "Serialized data length: " << serialized.length() << std::endl;
    
    // Create new component and deserialize
    StatsComponent stats2(player.getID());
    bool success = stats2.deserialize(serialized);
    assert(success);
    
    // Verify deserialized data
    std::cout << "Actual HP: " << stats2.getCurrentHP() << std::endl;
    std::cout << "Actual Base Max HP: " << stats2.getBaseMaxHP() << std::endl;
    std::cout << "Actual Level: " << stats2.getLevel() << std::endl;
    std::cout << "Actual Experience: " << stats2.getCurrentExperience() << std::endl;
    std::cout << "Actual Strength: " << stats2.getBaseAttribute(AttributeType::Strength) << std::endl;
    
    assert(stats2.getBaseMaxHP() == 200.0f);
    assert(stats2.getCurrentHP() == 150.0f);
    assert(stats2.getLevel() == 5);
    assert(stats2.getCurrentExperience() == 50);
    assert(stats2.getBaseAttribute(AttributeType::Strength) == 20);
    assert(stats2.hasModifier("hp", "test_mod"));
    
    std::cout << "Deserialized HP: " << stats2.getCurrentHP() << "/" << stats2.getMaxHP() << std::endl;
    std::cout << "Deserialized Level: " << stats2.getLevel() << std::endl;
    std::cout << "Deserialized Strength: " << stats2.getAttribute(AttributeType::Strength) << std::endl;
    
    std::cout << "Serialization test passed!" << std::endl;
}

int main() {
    std::cout << "Starting Character Stats System Tests..." << std::endl;
    
    try {
        testBasicStats();
        testAttributes();
        testExperienceAndLeveling();
        testStatModifiers();
        testCallbacks();
        testSerialization();
        
        std::cout << "\n=== All Character Stats Tests Passed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}