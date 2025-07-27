#pragma once

#include "System.h"
#include "../components/QuestComponent.h"
#include "../entities/EntityManager.h"
#include "../entities/Entity.h"
#include <memory>
#include <functional>

namespace RPGEngine {
namespace Systems {

using RPGEngine::System;
using RPGEngine::Components::EntityId;

/**
 * Quest system
 * Manages quest updates, validation, and integration with other systems
 */
class QuestSystem : public System {
public:
    /**
     * Constructor
     * @param entityManager Entity manager reference
     */
    explicit QuestSystem(std::shared_ptr<RPGEngine::EntityManager> entityManager);
    
    /**
     * Destructor
     */
    ~QuestSystem();
    
    /**
     * Initialize system
     */
    bool initialize() override;
    
    /**
     * Update system
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) override;
    
    /**
     * Shutdown system
     */
    void shutdown() override;
    
    // Quest management helpers
    
    /**
     * Load quest definitions from file
     * @param filename Quest definition file path
     * @return true if successful
     */
    bool loadQuestDefinitions(const std::string& filename);
    
    /**
     * Save quest definitions to file
     * @param filename Quest definition file path
     * @return true if successful
     */
    bool saveQuestDefinitions(const std::string& filename);
    
    /**
     * Create quest definition
     * @param questId Quest ID
     * @param name Quest name
     * @param description Quest description
     * @return Quest definition reference
     */
    Components::QuestDefinition& createQuestDefinition(const std::string& questId, 
                                                      const std::string& name, 
                                                      const std::string& description);
    
    /**
     * Register quest definition
     * @param definition Quest definition
     */
    void registerQuestDefinition(const Components::QuestDefinition& definition);
    
    // Global quest tracking
    
    /**
     * Track kill globally (for all entities with QuestComponent)
     * @param enemyType Enemy type that was killed
     * @param count Number killed
     */
    void trackKillGlobal(const std::string& enemyType, int count = 1);
    
    /**
     * Track item collection globally
     * @param itemId Item ID that was collected
     * @param count Number collected
     */
    void trackItemCollectionGlobal(const std::string& itemId, int count = 1);
    
    /**
     * Track NPC interaction globally
     * @param npcId NPC ID that was talked to
     */
    void trackNPCInteractionGlobal(const std::string& npcId);
    
    /**
     * Track location visit globally
     * @param locationId Location ID that was visited
     */
    void trackLocationVisitGlobal(const std::string& locationId);
    
    /**
     * Track custom objective globally
     * @param objectiveType Custom objective type
     * @param target Target identifier
     * @param count Progress amount
     */
    void trackCustomObjectiveGlobal(const std::string& objectiveType, const std::string& target, int count = 1);
    
    // Quest validation and integrity
    
    /**
     * Validate all active quests
     * @return true if all quests are valid
     */
    bool validateActiveQuests();
    
    /**
     * Fix quest integrity issues
     * @return Number of issues fixed
     */
    int fixQuestIntegrity();
    
    /**
     * Get quest statistics
     * @return Quest statistics as string
     */
    std::string getQuestStatistics() const;
    
    // Event callbacks
    
    /**
     * Set global quest started callback
     * @param callback Function called when any quest is started
     */
    void setGlobalQuestStartedCallback(std::function<void(EntityId, const std::string&)> callback) {
        m_globalQuestStartedCallback = callback;
    }
    
    /**
     * Set global quest completed callback
     * @param callback Function called when any quest is completed
     */
    void setGlobalQuestCompletedCallback(std::function<void(EntityId, const std::string&)> callback) {
        m_globalQuestCompletedCallback = callback;
    }
    
    /**
     * Set global quest failed callback
     * @param callback Function called when any quest fails
     */
    void setGlobalQuestFailedCallback(std::function<void(EntityId, const std::string&, const std::string&)> callback) {
        m_globalQuestFailedCallback = callback;
    }
    
    /**
     * Set global objective completed callback
     * @param callback Function called when any objective is completed
     */
    void setGlobalObjectiveCompletedCallback(std::function<void(EntityId, const std::string&, const std::string&)> callback) {
        m_globalObjectiveCompletedCallback = callback;
    }
    
    /**
     * Set global reward given callback
     * @param callback Function called when any reward is given
     */
    void setGlobalRewardGivenCallback(std::function<void(EntityId, const Components::QuestReward&)> callback) {
        m_globalRewardGivenCallback = callback;
    }
    
protected:
    /**
     * Called during update
     * @param deltaTime Time elapsed since last update
     */
    void onUpdate(float deltaTime) override;

private:
    /**
     * Setup quest component callbacks for entity
     * @param entityId Entity ID
     * @param questComponent Quest component
     */
    void setupQuestCallbacks(EntityId entityId, Components::QuestComponent* questComponent);
    
    /**
     * Parse quest definition from JSON
     * @param json JSON object
     * @return Quest definition
     */
    Components::QuestDefinition parseQuestDefinition(const std::string& json);
    
    /**
     * Convert quest definition to JSON
     * @param definition Quest definition
     * @return JSON string
     */
    std::string questDefinitionToJson(const Components::QuestDefinition& definition);
    
    std::shared_ptr<RPGEngine::EntityManager> m_entityManager;
    
    // Global callbacks
    std::function<void(EntityId, const std::string&)> m_globalQuestStartedCallback;
    std::function<void(EntityId, const std::string&)> m_globalQuestCompletedCallback;
    std::function<void(EntityId, const std::string&, const std::string&)> m_globalQuestFailedCallback;
    std::function<void(EntityId, const std::string&, const std::string&)> m_globalObjectiveCompletedCallback;
    std::function<void(EntityId, const Components::QuestReward&)> m_globalRewardGivenCallback;
    
    // Quest validation data
    std::vector<std::string> m_validationErrors;
    
    bool m_initialized;
};

} // namespace Systems
} // namespace RPGEngine