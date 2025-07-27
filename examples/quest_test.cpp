#include <iostream>
#include <cassert>
#include <memory>
#include "../src/components/QuestComponent.h"
#include "../src/systems/QuestSystem.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Components;
using namespace RPGEngine::Systems;
using namespace RPGEngine::Entities;

void testQuestDefinitionCreation() {
    std::cout << "Testing quest definition creation..." << std::endl;
    
    // Create a quest definition
    QuestDefinition questDef("test_quest", "Test Quest");
    questDef.description = "A simple test quest";
    questDef.category = "tutorial";
    questDef.level = 1;
    questDef.isAutoComplete = true;
    
    // Add objectives
    QuestObjective killObjective("kill_rats", "Kill 5 rats", ObjectiveType::Kill, "rat", 5);
    QuestObjective talkObjective("talk_npc", "Talk to the village elder", ObjectiveType::Talk, "elder_npc", 1);
    
    questDef.addObjective(killObjective);
    questDef.addObjective(talkObjective);
    
    // Add rewards
    QuestReward expReward("experience", "player", 100);
    QuestReward goldReward("gold", "player", 50);
    
    questDef.addReward(expReward);
    questDef.addReward(goldReward);
    
    // Register the quest definition
    QuestComponent::registerQuestDefinition(questDef);
    
    // Verify registration
    assert(QuestComponent::hasQuestDefinition("test_quest"));
    const QuestDefinition* retrievedDef = QuestComponent::getQuestDefinition("test_quest");
    assert(retrievedDef != nullptr);
    assert(retrievedDef->name == "Test Quest");
    assert(retrievedDef->objectives.size() == 2);
    assert(retrievedDef->rewards.size() == 2);
    
    std::cout << "✓ Quest definition creation test passed" << std::endl;
}

void testQuestComponent() {
    std::cout << "Testing quest component functionality..." << std::endl;
    
    // Create entity and quest component
    EntityID testEntity = 1;
    QuestComponent questComponent(testEntity);
    
    // Test quest starting
    bool started = questComponent.startQuest("test_quest", "test_system");
    assert(started);
    assert(questComponent.isQuestActive("test_quest"));
    assert(questComponent.getQuestStatus("test_quest") == QuestStatus::Active);
    
    // Test objective progress
    bool objectiveCompleted = questComponent.updateObjectiveProgress("test_quest", "kill_rats", 3);
    assert(!objectiveCompleted); // Should not be completed yet
    
    int progress = questComponent.getObjectiveProgress("test_quest", "kill_rats");
    assert(progress == 3);
    
    // Complete the kill objective
    objectiveCompleted = questComponent.updateObjectiveProgress("test_quest", "kill_rats", 2);
    assert(objectiveCompleted); // Should be completed now
    assert(questComponent.isObjectiveCompleted("test_quest", "kill_rats"));
    
    // Complete the talk objective
    questComponent.completeObjective("test_quest", "talk_npc");
    assert(questComponent.isObjectiveCompleted("test_quest", "talk_npc"));
    
    // Quest should be auto-completed since all objectives are done
    assert(questComponent.isQuestCompleted("test_quest"));
    assert(!questComponent.isQuestActive("test_quest"));
    
    std::cout << "✓ Quest component functionality test passed" << std::endl;
}

void testQuestTracking() {
    std::cout << "Testing quest tracking helpers..." << std::endl;
    
    // Create another quest for tracking
    QuestDefinition trackingQuest("tracking_quest", "Tracking Quest");
    trackingQuest.description = "Test tracking functionality";
    trackingQuest.isAutoComplete = false; // Manual completion
    
    QuestObjective collectObjective("collect_items", "Collect 3 potions", ObjectiveType::Collect, "health_potion", 3);
    QuestObjective locationObjective("visit_location", "Visit the ancient ruins", ObjectiveType::Reach, "ancient_ruins", 1);
    
    trackingQuest.addObjective(collectObjective);
    trackingQuest.addObjective(locationObjective);
    
    QuestComponent::registerQuestDefinition(trackingQuest);
    
    // Create quest component and start quest
    EntityID trackingEntity = 2;
    QuestComponent trackingComponent(trackingEntity);
    trackingComponent.startQuest("tracking_quest");
    
    // Test item collection tracking
    trackingComponent.trackItemCollection("health_potion", 2);
    assert(trackingComponent.getObjectiveProgress("tracking_quest", "collect_items") == 2);
    
    trackingComponent.trackItemCollection("health_potion", 1);
    assert(trackingComponent.isObjectiveCompleted("tracking_quest", "collect_items"));
    
    // Test location tracking
    trackingComponent.trackLocationVisit("ancient_ruins");
    assert(trackingComponent.isObjectiveCompleted("tracking_quest", "visit_location"));
    
    // Manually complete the quest
    bool completed = trackingComponent.completeQuest("tracking_quest");
    assert(completed);
    assert(trackingComponent.isQuestCompleted("tracking_quest"));
    
    std::cout << "✓ Quest tracking test passed" << std::endl;
}

void testQuestVariables() {
    std::cout << "Testing quest variables..." << std::endl;
    
    EntityID variableEntity = 3;
    QuestComponent variableComponent(variableEntity);
    variableComponent.startQuest("test_quest");
    
    // Set and get quest variables
    variableComponent.setQuestVariable("test_quest", "npc_met", "true");
    variableComponent.setQuestVariable("test_quest", "dialogue_choice", "option_1");
    
    std::string npcMet = variableComponent.getQuestVariable("test_quest", "npc_met");
    std::string dialogueChoice = variableComponent.getQuestVariable("test_quest", "dialogue_choice");
    std::string nonExistent = variableComponent.getQuestVariable("test_quest", "non_existent", "default");
    
    assert(npcMet == "true");
    assert(dialogueChoice == "option_1");
    assert(nonExistent == "default");
    
    std::cout << "✓ Quest variables test passed" << std::endl;
}

void testQuestSerialization() {
    std::cout << "Testing quest serialization..." << std::endl;
    
    // Create quest component with active quest
    EntityID serializeEntity = 4;
    QuestComponent originalComponent(serializeEntity);
    originalComponent.startQuest("test_quest");
    originalComponent.updateObjectiveProgress("test_quest", "kill_rats", 3);
    originalComponent.setQuestVariable("test_quest", "test_var", "test_value");
    
    // Serialize
    std::string serializedData = originalComponent.serialize();
    assert(!serializedData.empty());
    
    // Create new component and deserialize
    QuestComponent deserializedComponent(serializeEntity);
    bool deserializeSuccess = deserializedComponent.deserialize(serializedData);
    assert(deserializeSuccess);
    
    // Verify deserialized data
    assert(deserializedComponent.isQuestActive("test_quest"));
    assert(deserializedComponent.getObjectiveProgress("test_quest", "kill_rats") == 3);
    assert(deserializedComponent.getQuestVariable("test_quest", "test_var") == "test_value");
    
    std::cout << "✓ Quest serialization test passed" << std::endl;
}

void testQuestSystem() {
    std::cout << "Testing quest system..." << std::endl;
    
    // Create entity manager and quest system
    auto entityManager = std::make_shared<EntityManager>();
    QuestSystem questSystem(entityManager);
    
    questSystem.initialize();
    
    // Create quest definition through system
    QuestDefinition& systemQuest = questSystem.createQuestDefinition("system_quest", "System Quest", "Created through quest system");
    
    QuestObjective systemObjective("system_obj", "System objective", ObjectiveType::Custom, "system_target", 1);
    systemQuest.addObjective(systemObjective);
    
    // Test global tracking (would need entities with quest components)
    questSystem.trackKillGlobal("goblin", 1);
    questSystem.trackItemCollectionGlobal("sword", 1);
    questSystem.trackNPCInteractionGlobal("merchant");
    questSystem.trackLocationVisitGlobal("town_square");
    
    // Test validation
    bool valid = questSystem.validateActiveQuests();
    // Should be valid since we don't have any active quests with issues
    
    // Get statistics
    std::string stats = questSystem.getQuestStatistics();
    assert(!stats.empty());
    std::cout << "Quest Statistics:\n" << stats << std::endl;
    
    questSystem.shutdown();
    
    std::cout << "✓ Quest system test passed" << std::endl;
}

void testQuestPrerequisites() {
    std::cout << "Testing quest prerequisites..." << std::endl;
    
    // Create prerequisite quest
    QuestDefinition prereqQuest("prereq_quest", "Prerequisite Quest");
    prereqQuest.description = "Must be completed first";
    prereqQuest.isAutoComplete = true;
    
    QuestObjective prereqObjective("prereq_obj", "Complete prerequisite", ObjectiveType::Custom, "prereq_target", 1);
    prereqQuest.addObjective(prereqObjective);
    
    QuestComponent::registerQuestDefinition(prereqQuest);
    
    // Create main quest with prerequisite
    QuestDefinition mainQuest("main_quest", "Main Quest");
    mainQuest.description = "Requires prerequisite";
    mainQuest.prerequisites.push_back("prereq_quest");
    
    QuestObjective mainObjective("main_obj", "Main objective", ObjectiveType::Custom, "main_target", 1);
    mainQuest.addObjective(mainObjective);
    
    QuestComponent::registerQuestDefinition(mainQuest);
    
    // Test prerequisite checking
    EntityID prereqEntity = 5;
    QuestComponent prereqComponent(prereqEntity);
    
    // Should not be able to start main quest without prerequisite
    bool mainStarted = prereqComponent.startQuest("main_quest");
    assert(!mainStarted);
    
    // Complete prerequisite quest
    prereqComponent.startQuest("prereq_quest");
    prereqComponent.completeObjective("prereq_quest", "prereq_obj");
    assert(prereqComponent.isQuestCompleted("prereq_quest"));
    
    // Now should be able to start main quest
    mainStarted = prereqComponent.startQuest("main_quest");
    assert(mainStarted);
    assert(prereqComponent.isQuestActive("main_quest"));
    
    std::cout << "✓ Quest prerequisites test passed" << std::endl;
}

void testQuestTimers() {
    std::cout << "Testing quest timers..." << std::endl;
    
    // Create timed quest
    QuestDefinition timedQuest("timed_quest", "Timed Quest");
    timedQuest.description = "Must be completed within time limit";
    timedQuest.timeLimit = 5; // 5 seconds
    
    QuestObjective timedObjective("timed_obj", "Complete in time", ObjectiveType::Custom, "timed_target", 1);
    timedQuest.addObjective(timedObjective);
    
    QuestComponent::registerQuestDefinition(timedQuest);
    
    // Test timer functionality
    EntityID timedEntity = 6;
    QuestComponent timedComponent(timedEntity);
    
    timedComponent.startQuest("timed_quest");
    assert(timedComponent.isQuestActive("timed_quest"));
    
    // Simulate time passing (6 seconds - should fail the quest)
    timedComponent.updateQuestTimers(6.0f);
    
    // Quest should have failed due to timeout
    assert(!timedComponent.isQuestActive("timed_quest"));
    assert(timedComponent.getQuestStatus("timed_quest") == QuestStatus::Failed);
    
    std::cout << "✓ Quest timers test passed" << std::endl;
}

int main() {
    std::cout << "Running Quest Management Tests..." << std::endl;
    
    try {
        testQuestDefinitionCreation();
        testQuestComponent();
        testQuestTracking();
        testQuestVariables();
        testQuestSerialization();
        testQuestSystem();
        testQuestPrerequisites();
        testQuestTimers();
        
        std::cout << "\n✅ All quest management tests passed!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}