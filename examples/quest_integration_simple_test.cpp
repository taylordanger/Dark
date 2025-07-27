#include <iostream>
#include <memory>
#include "../src/components/QuestComponent.h"
#include "../src/components/DialogueComponent.h"
#include "../src/systems/QuestSystem.h"
#include "../src/systems/QuestDialogueIntegration.h"
#include "../src/systems/QuestWorldIntegration.h"
#include "../src/entities/EntityManager.h"
#include "../src/core/Event.h"

using namespace RPGEngine;
using namespace RPGEngine::Components;
using namespace RPGEngine::Systems;

// Mock classes for testing
class MockResourceManager {
public:
    MockResourceManager() = default;
    ~MockResourceManager() = default;
};

class MockComponentManager {
public:
    MockComponentManager() = default;
    ~MockComponentManager() = default;
};

class MockWorldManager {
public:
    MockWorldManager(std::shared_ptr<MockResourceManager> resourceManager,
                    std::shared_ptr<EntityManager> entityManager,
                    std::shared_ptr<MockComponentManager> componentManager)
        : m_resourceManager(resourceManager)
        , m_entityManager(entityManager)
        , m_componentManager(componentManager) {}
    
    ~MockWorldManager() = default;
    
private:
    std::shared_ptr<MockResourceManager> m_resourceManager;
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<MockComponentManager> m_componentManager;
};

int main() {
    std::cout << "=== Simple Quest Integration Test ===" << std::endl;
    
    // Create entity manager
    auto entityManager = std::make_shared<EntityManager>();
    
    // Create quest system
    auto questSystem = std::make_shared<QuestSystem>(entityManager);
    questSystem->initialize();
    
    // Create quest-dialogue integration
    auto questDialogueIntegration = std::make_shared<QuestDialogueIntegration>(entityManager);
    questDialogueIntegration->initialize();
    
    // Create mock managers for world integration
    auto mockResourceManager = std::make_shared<MockResourceManager>();
    auto mockComponentManager = std::make_shared<MockComponentManager>();
    auto mockWorldManager = std::make_shared<MockWorldManager>(mockResourceManager, entityManager, mockComponentManager);
    
    // Create event dispatcher
    auto eventDispatcher = std::make_shared<EventDispatcher>();
    
    // Create test entities
    EntityId playerId = 1;
    EntityId npcId = 2;
    
    // Create quest component for player
    auto playerQuestComponent = std::make_shared<QuestComponent>(playerId);
    
    // Create dialogue component for NPC
    auto npcDialogueComponent = std::make_shared<DialogueComponent>(npcId);
    
    // Register components with integration system
    questDialogueIntegration->registerQuestComponent(playerId, playerQuestComponent);
    questDialogueIntegration->registerDialogueComponent(npcId, npcDialogueComponent);
    
    std::cout << "\n1. Creating quest definitions..." << std::endl;
    
    // Create a test quest definition
    auto& questDef = questSystem->createQuestDefinition("find_artifact", "Find the Lost Artifact", 
        "A mysterious artifact has been lost in the ancient ruins. Find it and bring it back.");
    questDef.category = "main";
    questDef.level = 5;
    
    // Add objectives
    QuestObjective talkObjective("talk_to_sage", "Talk to the Sage", ObjectiveType::Talk, "sage_npc", 1);
    QuestObjective findObjective("find_artifact", "Find the Lost Artifact", ObjectiveType::Collect, "ancient_artifact", 1);
    QuestObjective returnObjective("return_artifact", "Return to the Sage", ObjectiveType::Deliver, "sage_npc", 1);
    
    questDef.addObjective(talkObjective);
    questDef.addObjective(findObjective);
    questDef.addObjective(returnObjective);
    
    // Add rewards
    QuestReward expReward("experience", "player", 500);
    QuestReward goldReward("gold", "player", 100);
    QuestReward itemReward("item", "magic_ring", 1);
    
    questDef.addReward(expReward);
    questDef.addReward(goldReward);
    questDef.addReward(itemReward);
    
    std::cout << "Created quest: " << questDef.name << " with " << questDef.objectives.size() << " objectives" << std::endl;
    
    std::cout << "\n2. Testing quest start and progress..." << std::endl;
    
    // Start the quest
    bool questStarted = playerQuestComponent->startQuest("find_artifact", "sage");
    std::cout << "Quest started: " << (questStarted ? "Yes" : "No") << std::endl;
    
    // Check quest status
    std::cout << "Quest status: " << (playerQuestComponent->isQuestActive("find_artifact") ? "Active" : "Not Active") << std::endl;
    
    std::cout << "\n3. Testing quest progress tracking..." << std::endl;
    
    // Simulate talking to sage (completing first objective)
    std::cout << "Tracking NPC interaction with sage..." << std::endl;
    playerQuestComponent->trackNPCInteraction("sage_npc");
    
    // Simulate finding the artifact
    std::cout << "Simulating artifact collection..." << std::endl;
    playerQuestComponent->trackItemCollection("ancient_artifact");
    
    // Check quest progress
    const ActiveQuest* activeQuest = playerQuestComponent->getActiveQuest("find_artifact");
    if (activeQuest) {
        std::cout << "Quest progress: " << (activeQuest->getCompletionPercentage() * 100) << "%" << std::endl;
        std::cout << "Can complete quest: " << (activeQuest->canComplete() ? "Yes" : "No") << std::endl;
        
        for (const auto& objective : activeQuest->objectives) {
            std::cout << "  " << objective.description << ": " << 
                (objective.isCompleted ? "✓" : "○") << 
                " (" << objective.currentCount << "/" << objective.requiredCount << ")" << std::endl;
        }
    }
    
    std::cout << "\n4. Testing world event integration..." << std::endl;
    
    // Test world event handling
    std::cout << "Triggering world events..." << std::endl;
    
    // Simulate map transition
    questDialogueIntegration->handleWorldEvent("map_transition", "eastern_ruins");
    
    // Simulate item collection
    questDialogueIntegration->handleWorldEvent("item_collected", "ancient_artifact");
    
    // Simulate enemy kill
    questDialogueIntegration->handleWorldEvent("enemy_killed", "skeleton_warrior");
    
    // Test custom objective
    questDialogueIntegration->handleWorldEvent("custom_objective", "kill:skeleton_warrior:1");
    
    std::cout << "\n5. Testing quest completion..." << std::endl;
    
    // Complete the final objective first
    std::cout << "Completing final objective (return to sage)..." << std::endl;
    // The return objective is a "Deliver" type, so we need to track it differently
    playerQuestComponent->updateObjectiveProgress("find_artifact", "return_artifact", 1);
    
    // Check quest progress again
    activeQuest = playerQuestComponent->getActiveQuest("find_artifact");
    if (activeQuest) {
        std::cout << "Quest progress after final objective: " << (activeQuest->getCompletionPercentage() * 100) << "%" << std::endl;
        std::cout << "Can complete quest: " << (activeQuest->canComplete() ? "Yes" : "No") << std::endl;
    }
    
    // Complete the quest
    bool questCompleted = playerQuestComponent->completeQuest("find_artifact", true);
    std::cout << "Quest completed: " << (questCompleted ? "Yes" : "No") << std::endl;
    
    // Check if quest is completed
    std::cout << "Quest status after completion: " << 
        (playerQuestComponent->isQuestCompleted("find_artifact") ? "Completed" : "Not Completed") << std::endl;
    
    std::cout << "\n6. Testing global quest callbacks..." << std::endl;
    
    // Set up global quest event callback
    questSystem->setGlobalQuestStartedCallback(
        [](EntityId entityId, const std::string& questId) {
            std::cout << "Global callback - Quest started: " << questId << " for entity " << entityId << std::endl;
        });
    
    questSystem->setGlobalQuestCompletedCallback(
        [](EntityId entityId, const std::string& questId) {
            std::cout << "Global callback - Quest completed: " << questId << " for entity " << entityId << std::endl;
        });
    
    questSystem->setGlobalObjectiveCompletedCallback(
        [](EntityId entityId, const std::string& questId, const std::string& objectiveId) {
            std::cout << "Global callback - Objective completed: " << objectiveId << " in quest " << questId << " for entity " << entityId << std::endl;
        });
    
    questSystem->setGlobalRewardGivenCallback(
        [](EntityId entityId, const QuestReward& reward) {
            std::cout << "Global callback - Reward given: " << reward.type << " " << reward.target << " x" << reward.amount << " to entity " << entityId << std::endl;
        });
    
    // Start another quest to trigger global callbacks
    auto& testQuestDef = questSystem->createQuestDefinition("test_global", "Test Global Quest", 
        "A quest to test global callbacks.");
    testQuestDef.category = "side";
    
    QuestObjective testObjective("test_obj", "Test Objective", ObjectiveType::Custom, "test_target", 1);
    testQuestDef.addObjective(testObjective);
    
    QuestReward testReward("experience", "player", 100);
    testQuestDef.addReward(testReward);
    
    playerQuestComponent->startQuest("test_global", "system");
    playerQuestComponent->updateObjectiveProgress("test_global", "test_obj", 1);
    playerQuestComponent->completeQuest("test_global", true);
    
    std::cout << "\n7. Testing quest statistics..." << std::endl;
    
    std::cout << "Quest statistics:" << std::endl;
    std::cout << questSystem->getQuestStatistics() << std::endl;
    
    std::cout << "\nActive quests:" << std::endl;
    const auto& activeQuests = playerQuestComponent->getActiveQuests();
    for (const auto& quest : activeQuests) {
        const QuestDefinition* questDef = QuestComponent::getQuestDefinition(quest.questId);
        std::string questName = questDef ? questDef->name : quest.questId;
        std::cout << "  " << questName << " (" << (quest.getCompletionPercentage() * 100) << "% complete)" << std::endl;
    }
    
    std::cout << "\nCompleted quests:" << std::endl;
    const auto& completedQuests = playerQuestComponent->getCompletedQuests();
    for (const auto& questId : completedQuests) {
        const QuestDefinition* questDef = QuestComponent::getQuestDefinition(questId);
        std::string questName = questDef ? questDef->name : questId;
        std::cout << "  " << questName << std::endl;
    }
    
    std::cout << "\n=== Simple Quest Integration Test Complete ===" << std::endl;
    
    // Cleanup
    questSystem->shutdown();
    questDialogueIntegration->shutdown();
    
    return 0;
}