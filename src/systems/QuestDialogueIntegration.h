#pragma once

#include "../components/QuestComponent.h"
#include "../components/DialogueComponent.h"
#include "../entities/EntityManager.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

namespace RPGEngine {
namespace Systems {

/**
 * Quest-Dialogue Integration System
 * Handles integration between quest and dialogue systems
 */
class QuestDialogueIntegration {
public:
    /**
     * Constructor
     * @param entityManager Entity manager reference
     */
    explicit QuestDialogueIntegration(std::shared_ptr<RPGEngine::EntityManager> entityManager);
    
    /**
     * Destructor
     */
    ~QuestDialogueIntegration();
    
    /**
     * Initialize integration
     */
    bool initialize();
    
    /**
     * Shutdown integration
     */
    void shutdown();
    
    /**
     * Register quest component for integration
     * @param entityId Entity ID
     * @param questComponent Quest component
     */
    void registerQuestComponent(Components::EntityId entityId, 
                               std::shared_ptr<Components::QuestComponent> questComponent);
    
    /**
     * Register dialogue component for integration
     * @param entityId Entity ID
     * @param dialogueComponent Dialogue component
     */
    void registerDialogueComponent(Components::EntityId entityId, 
                                  std::shared_ptr<Components::DialogueComponent> dialogueComponent);
    
    /**
     * Unregister quest component
     * @param entityId Entity ID
     */
    void unregisterQuestComponent(Components::EntityId entityId);
    
    /**
     * Unregister dialogue component
     * @param entityId Entity ID
     */
    void unregisterDialogueComponent(Components::EntityId entityId);
    
    /**
     * Setup dialogue actions for quest integration
     * @param dialogueComponent Dialogue component to setup
     */
    void setupDialogueQuestActions(std::shared_ptr<Components::DialogueComponent> dialogueComponent);
    
    /**
     * Setup quest callbacks for dialogue integration
     * @param questComponent Quest component to setup
     */
    void setupQuestDialogueCallbacks(std::shared_ptr<Components::QuestComponent> questComponent);
    
    /**
     * Handle dialogue action execution
     * @param entityId Entity ID
     * @param action Dialogue action
     * @return true if action was handled
     */
    bool handleDialogueAction(Components::EntityId entityId, const Components::DialogueAction& action);
    
    /**
     * Handle dialogue condition evaluation
     * @param entityId Entity ID
     * @param condition Dialogue condition
     * @return true if condition is met
     */
    bool handleDialogueCondition(Components::EntityId entityId, const Components::DialogueCondition& condition);
    
    /**
     * Start quest from dialogue
     * @param entityId Entity ID
     * @param questId Quest ID
     * @param startedBy Who started the quest
     * @return true if quest was started
     */
    bool startQuestFromDialogue(Components::EntityId entityId, const std::string& questId, const std::string& startedBy = "dialogue");
    
    /**
     * Complete quest from dialogue
     * @param entityId Entity ID
     * @param questId Quest ID
     * @return true if quest was completed
     */
    bool completeQuestFromDialogue(Components::EntityId entityId, const std::string& questId);
    
    /**
     * Update quest objective from dialogue
     * @param entityId Entity ID
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @param progress Progress amount
     * @return true if objective was updated
     */
    bool updateQuestObjectiveFromDialogue(Components::EntityId entityId, const std::string& questId, 
                                         const std::string& objectiveId, int progress = 1);
    
    /**
     * Set quest variable from dialogue
     * @param entityId Entity ID
     * @param questId Quest ID
     * @param key Variable key
     * @param value Variable value
     * @return true if variable was set
     */
    bool setQuestVariableFromDialogue(Components::EntityId entityId, const std::string& questId, 
                                     const std::string& key, const std::string& value);
    
    /**
     * Check if quest is active for dialogue condition
     * @param entityId Entity ID
     * @param questId Quest ID
     * @return true if quest is active
     */
    bool isQuestActiveForDialogue(Components::EntityId entityId, const std::string& questId) const;
    
    /**
     * Check if quest is completed for dialogue condition
     * @param entityId Entity ID
     * @param questId Quest ID
     * @return true if quest is completed
     */
    bool isQuestCompletedForDialogue(Components::EntityId entityId, const std::string& questId) const;
    
    /**
     * Check if objective is completed for dialogue condition
     * @param entityId Entity ID
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @return true if objective is completed
     */
    bool isObjectiveCompletedForDialogue(Components::EntityId entityId, const std::string& questId, 
                                        const std::string& objectiveId) const;
    
    /**
     * Get quest variable for dialogue condition
     * @param entityId Entity ID
     * @param questId Quest ID
     * @param key Variable key
     * @param defaultValue Default value
     * @return Variable value
     */
    std::string getQuestVariableForDialogue(Components::EntityId entityId, const std::string& questId, 
                                           const std::string& key, const std::string& defaultValue = "") const;
    
    /**
     * Set global quest event callback
     * @param callback Function called for quest events
     */
    void setQuestEventCallback(std::function<void(Components::EntityId, const std::string&, const std::string&)> callback) {
        m_questEventCallback = callback;
    }
    
    /**
     * Set global dialogue event callback
     * @param callback Function called for dialogue events
     */
    void setDialogueEventCallback(std::function<void(Components::EntityId, const std::string&, const std::string&)> callback) {
        m_dialogueEventCallback = callback;
    }
    
    /**
     * Handle world event for quest integration
     * @param eventType Type of world event
     * @param eventData Event data
     * @param entityId Entity ID (optional)
     */
    void handleWorldEvent(const std::string& eventType, const std::string& eventData, Components::EntityId entityId = 0);
    
    /**
     * Set world event handler
     * @param handler Function to handle world events
     */
    void setWorldEventHandler(std::function<void(const std::string&, const std::string&, Components::EntityId)> handler) {
        m_worldEventHandler = handler;
    }
    
private:
    /**
     * Get quest component for entity
     * @param entityId Entity ID
     * @return Quest component or nullptr
     */
    std::shared_ptr<Components::QuestComponent> getQuestComponent(Components::EntityId entityId) const;
    
    /**
     * Get dialogue component for entity
     * @param entityId Entity ID
     * @return Dialogue component or nullptr
     */
    std::shared_ptr<Components::DialogueComponent> getDialogueComponent(Components::EntityId entityId) const;
    
    /**
     * Trigger quest event
     * @param entityId Entity ID
     * @param eventType Event type
     * @param data Event data
     */
    void triggerQuestEvent(Components::EntityId entityId, const std::string& eventType, const std::string& data);
    
    /**
     * Trigger dialogue event
     * @param entityId Entity ID
     * @param eventType Event type
     * @param data Event data
     */
    void triggerDialogueEvent(Components::EntityId entityId, const std::string& eventType, const std::string& data);
    
    std::shared_ptr<RPGEngine::EntityManager> m_entityManager;
    
    // Component registries
    std::unordered_map<Components::EntityId, std::shared_ptr<Components::QuestComponent>> m_questComponents;
    std::unordered_map<Components::EntityId, std::shared_ptr<Components::DialogueComponent>> m_dialogueComponents;
    
    // Event callbacks
    std::function<void(Components::EntityId, const std::string&, const std::string&)> m_questEventCallback;
    std::function<void(Components::EntityId, const std::string&, const std::string&)> m_dialogueEventCallback;
    std::function<void(const std::string&, const std::string&, Components::EntityId)> m_worldEventHandler;
    
    bool m_initialized;
};

} // namespace Systems
} // namespace RPGEngine