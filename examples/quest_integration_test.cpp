#include <iostream>
#include <memory>
#include "../src/components/QuestComponent.h"
#include "../src/components/DialogueComponent.h"
#include "../src/systems/QuestSystem.h"
#include "../src/systems/QuestDialogueIntegration.h"
#include "../src/systems/QuestWorldIntegration.h"
#include "../src/ui/QuestUI.h"
#include "../src/ui/DialogueUI.h"
#include "../src/entities/EntityManager.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/input/InputManager.h"
#include "../src/world/WorldManager.h"
#include "../src/core/Event.h"
#include "../src/resources/ResourceManager.h"
#include "../src/components/ComponentManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Components;
using namespace RPGEngine::Systems;
using namespace RPGEngine::UI;

int main() {
    std::cout << "=== Quest Integration Test ===" << std::endl;
    
    // Create entity manager
    auto entityManager = std::make_shared<EntityManager>();
    
    // Create quest system
    auto questSystem = std::make_shared<QuestSystem>(entityManager);
    questSystem->initialize();
    
    // Create quest-dialogue integration
    auto questDialogueIntegration = std::make_shared<QuestDialogueIntegration>(entityManager);
    questDialogueIntegration->initialize();
    
    // Create mock renderer and input manager for UI systems
    auto mockRenderer = std::make_shared<Graphics::SpriteRenderer>();
    auto mockInputManager = std::make_shared<Input::InputManager>();
    
    // Create UI systems
    auto questUI = std::make_shared<QuestUI>(mockRenderer, mockInputManager);
    auto dialogueUI = std::make_shared<DialogueUI>(mockRenderer, mockInputManager);
    
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
    
    // Set up UI components
    questUI->setQuestComponent(playerQuestComponent);
    dialogueUI->setDialogueComponent(npcDialogueComponent);
    
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
    
    std::cout << "\n2. Testing quest-dialogue integration..." << std::endl;
    
    // Create a dialogue tree that integrates with quests
    DialogueTree questDialogue("sage_dialogue", "Sage Dialogue");
    questDialogue.startNodeId = "greeting";
    
    // Greeting node
    DialogueNode greetingNode("greeting", DialogueNodeType::Text);
    greetingNode.speaker = "Sage";
    greetingNode.text = "Greetings, traveler. I sense great potential in you.";
    greetingNode.nextNodeId = "main_choice";
    questDialogue.addNode(greetingNode);
    
    // Main choice node
    DialogueNode mainChoiceNode("main_choice", DialogueNodeType::Choice);
    mainChoiceNode.text = "What brings you to me?";
    
    DialogueChoice questChoice("ask_quest", "Do you have any tasks for me?", "check_quest");
    DialogueChoice statusChoice("quest_status", "How am I doing with the artifact quest?", "quest_progress");
    // Add condition to only show status choice if quest is active
    statusChoice.conditions.push_back(DialogueCondition("quest_active", "find_artifact", "==", "true"));
    
    DialogueChoice completeChoice("complete_quest", "I found the artifact!", "complete_artifact_quest");
    // Add condition to only show complete choice if artifact objective is done
    completeChoice.conditions.push_back(DialogueCondition("objective_completed", "find_artifact:find_artifact", "==", "true"));
    
    DialogueChoice goodbyeChoice("goodbye", "Farewell", "end");
    
    mainChoiceNode.choices.push_back(questChoice);
    mainChoiceNode.choices.push_back(statusChoice);
    mainChoiceNode.choices.push_back(completeChoice);
    mainChoiceNode.choices.push_back(goodbyeChoice);
    questDialogue.addNode(mainChoiceNode);
    
    // Check quest node
    DialogueNode checkQuestNode("check_quest", DialogueNodeType::Condition);
    checkQuestNode.conditions.push_back(DialogueCondition("quest_active", "find_artifact", "==", "false", true)); // negate = true
    checkQuestNode.nextNodeId = "give_quest";
    questDialogue.addNode(checkQuestNode);
    
    // Give quest node
    DialogueNode giveQuestNode("give_quest", DialogueNodeType::Text);
    giveQuestNode.speaker = "Sage";
    giveQuestNode.text = "Indeed! I need someone to find a lost artifact in the ancient ruins. Will you help?";
    giveQuestNode.actions.push_back(DialogueAction("start_quest", "find_artifact", "sage"));
    giveQuestNode.actions.push_back(DialogueAction("track_npc_interaction", "sage_npc", ""));
    giveQuestNode.nextNodeId = "quest_given";
    questDialogue.addNode(giveQuestNode);
    
    // Quest given confirmation
    DialogueNode questGivenNode("quest_given", DialogueNodeType::Text);
    questGivenNode.speaker = "Sage";
    questGivenNode.text = "Excellent! The artifact should be somewhere in the eastern ruins. Be careful!";
    questGivenNode.nextNodeId = "end";
    questDialogue.addNode(questGivenNode);
    
    // Quest progress node
    DialogueNode progressNode("quest_progress", DialogueNodeType::Text);
    progressNode.speaker = "Sage";
    progressNode.text = "The artifact still eludes you, I see. Keep searching the eastern ruins.";
    progressNode.nextNodeId = "end";
    questDialogue.addNode(progressNode);
    
    // Complete quest node
    DialogueNode completeQuestNode("complete_artifact_quest", DialogueNodeType::Text);
    completeQuestNode.speaker = "Sage";
    completeQuestNode.text = "Wonderful! You have found the artifact. Your reward is well deserved.";
    completeQuestNode.actions.push_back(DialogueAction("complete_quest", "find_artifact", ""));
    completeQuestNode.nextNodeId = "end";
    questDialogue.addNode(completeQuestNode);
    
    // End node
    DialogueNode endNode("end", DialogueNodeType::End);
    endNode.text = "May your journey be blessed.";
    questDialogue.addNode(endNode);
    
    // Add dialogue tree to component
    npcDialogueComponent->addDialogueTree(questDialogue);
    
    std::cout << "Created dialogue tree with quest integration" << std::endl;
    
    std::cout << "\n3. Simulating quest-dialogue interaction..." << std::endl;
    
    // Start dialogue
    std::cout << "\n--- Starting dialogue with Sage ---" << std::endl;
    npcDialogueComponent->startDialogue("sage_dialogue");
    
    // Simulate dialogue progression
    std::cout << "Current node: " << npcDialogueComponent->getCurrentNode()->text << std::endl;
    
    // Advance to choice
    npcDialogueComponent->advanceDialogue();
    std::cout << "Available choices:" << std::endl;
    auto choices = npcDialogueComponent->getAvailableChoices();
    for (size_t i = 0; i < choices.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << choices[i].text << std::endl;
    }
    
    // Choose to ask for quest
    std::cout << "\nChoosing: Ask for quest" << std::endl;
    npcDialogueComponent->advanceDialogue("ask_quest");
    
    // This should start the quest
    std::cout << "Quest status after dialogue: " << 
        (playerQuestComponent->isQuestActive("find_artifact") ? "Active" : "Not Active") << std::endl;
    
    std::cout << "\n4. Testing quest progress tracking..." << std::endl;
    
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
    
    std::cout << "\n5. Testing quest completion through dialogue..." << std::endl;
    
    // Start dialogue again to complete quest
    std::cout << "\n--- Returning to Sage with artifact ---" << std::endl;
    npcDialogueComponent->startDialogue("sage_dialogue");
    npcDialogueComponent->advanceDialogue(); // Go to choices
    
    // Show available choices (should now include completion option)
    choices = npcDialogueComponent->getAvailableChoices();
    std::cout << "Available choices:" << std::endl;
    for (size_t i = 0; i < choices.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << choices[i].text << std::endl;
    }
    
    // Choose to complete quest
    std::cout << "\nChoosing: Complete quest" << std::endl;
    npcDialogueComponent->advanceDialogue("complete_quest");
    
    // Check if quest is completed
    std::cout << "Quest status after completion: " << 
        (playerQuestComponent->isQuestCompleted("find_artifact") ? "Completed" : "Not Completed") << std::endl;
    
    std::cout << "\n6. Testing quest notifications..." << std::endl;
    
    // Test quest UI notifications
    questUI->showQuestStartedNotification("find_artifact");
    questUI->showObjectiveCompletedNotification("find_artifact", "talk_to_sage");
    questUI->showQuestCompletedNotification("find_artifact");
    
    // Test reward notification
    QuestReward testReward("experience", "player", 500);
    questUI->showRewardReceivedNotification(testReward);
    
    std::cout << "\n7. Testing quest UI display..." << std::endl;
    
    // Create another active quest for UI testing
    auto& dailyQuestDef = questSystem->createQuestDefinition("daily_herbs", "Collect Daily Herbs", 
        "Collect 10 healing herbs for the village healer.");
    dailyQuestDef.category = "daily";
    dailyQuestDef.level = 1;
    
    QuestObjective herbObjective("collect_herbs", "Collect Healing Herbs", ObjectiveType::Collect, "healing_herb", 10);
    dailyQuestDef.addObjective(herbObjective);
    
    // Start the daily quest
    playerQuestComponent->startQuest("daily_herbs", "healer");
    
    // Simulate some progress
    playerQuestComponent->trackItemCollection("healing_herb", 3);
    
    std::cout << "Active quests:" << std::endl;
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
    
    std::cout << "\n8. Testing world event integration..." << std::endl;
    
    // Create mock world manager and event dispatcher
    auto mockResourceManager = std::make_shared<Resources::ResourceManager>();
    auto mockComponentManager = std::make_shared<ComponentManager>();
    auto worldManager = std::make_shared<World::WorldManager>(mockResourceManager, entityManager, mockComponentManager);
    auto eventDispatcher = std::make_shared<EventDispatcher>();
    
    // Create quest world integration
    auto questWorldIntegration = std::make_shared<QuestWorldIntegration>(questSystem, worldManager, questDialogueIntegration);
    questWorldIntegration->setQuestUI(questUI);
    questWorldIntegration->setEventDispatcher(eventDispatcher);
    questWorldIntegration->initialize();
    
    // Test world event handling
    std::cout << "Triggering world events..." << std::endl;
    
    // Simulate map transition
    questWorldIntegration->triggerWorldEvent("map_transition", "eastern_ruins");
    
    // Simulate item collection
    questWorldIntegration->triggerWorldEvent("item_collected", "ancient_artifact");
    
    // Simulate enemy kill
    questWorldIntegration->triggerWorldEvent("enemy_killed", "skeleton_warrior");
    
    // Test custom event handler
    questWorldIntegration->registerCustomEventHandler("custom_event", 
        [](const std::string& data, Components::EntityId entityId) {
            std::cout << "Custom event handled: " << data << " for entity " << entityId << std::endl;
        });
    
    questWorldIntegration->triggerWorldEvent("custom_event", "test_data", playerId);
    
    std::cout << "\n9. Testing enhanced quest UI features..." << std::endl;
    
    // Test quest tracking
    questUI->trackQuest("find_artifact");
    questUI->trackQuest("daily_herbs");
    
    std::cout << "Tracked quests:" << std::endl;
    for (const auto& questId : questUI->getTrackedQuestIds()) {
        std::cout << "  " << questId << std::endl;
    }
    
    // Test progress notifications
    questUI->showQuestProgressNotification("daily_herbs", "collect_herbs", 5, 10);
    
    // Test multiple rewards notification
    std::vector<Components::QuestReward> multipleRewards = {
        Components::QuestReward("experience", "player", 500),
        Components::QuestReward("gold", "player", 100),
        Components::QuestReward("item", "magic_ring", 1)
    };
    questUI->showMultipleRewardsNotification(multipleRewards, "Find the Lost Artifact");
    
    // Test quest completion with rewards
    questUI->showQuestCompletionWithRewards("find_artifact", multipleRewards);
    
    std::cout << "\n10. Testing global quest event callbacks..." << std::endl;
    
    // Set up global quest event callback
    questWorldIntegration->setGlobalQuestEventCallback(
        [](Components::EntityId entityId, const std::string& eventType, const std::string& data) {
            std::cout << "Global quest event: " << eventType << " for entity " << entityId << " with data: " << data << std::endl;
        });
    
    // Start another quest to trigger global callback
    auto& testQuestDef = questSystem->createQuestDefinition("test_global", "Test Global Quest", 
        "A quest to test global callbacks.");
    testQuestDef.category = "side";
    
    QuestObjective testObjective("test_obj", "Test Objective", ObjectiveType::Custom, "test_target", 1);
    testQuestDef.addObjective(testObjective);
    
    playerQuestComponent->startQuest("test_global", "system");
    playerQuestComponent->completeQuest("test_global", true);
    
    std::cout << "\n=== Enhanced Quest Integration Test Complete ===" << std::endl;
    
    // Cleanup
    questWorldIntegration->shutdown();
    questSystem->shutdown();
    questDialogueIntegration->shutdown();
    
    return 0;
}