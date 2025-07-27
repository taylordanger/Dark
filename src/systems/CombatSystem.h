#pragma once

#include "System.h"
#include "../components/CombatComponent.h"
#include "../components/StatsComponent.h"
#include "../entities/EntityManager.h"
#include "../components/ComponentManager.h"
#include <queue>
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>

namespace RPGEngine {
namespace Systems {

/**
 * Combat state enumeration
 */
enum class CombatState {
    Inactive,       // No combat active
    Initializing,   // Setting up combat
    TurnStart,      // Starting a new turn
    PlayerTurn,     // Player's turn to act
    EnemyTurn,      // Enemy's turn to act
    Processing,     // Processing actions
    TurnEnd,        // Ending current turn
    Victory,        // Combat won
    Defeat,         // Combat lost
    Escaped         // Escaped from combat
};

/**
 * Combat participant structure
 */
struct CombatParticipant {
    EntityId entity;
    std::string name;
    bool isPlayer;
    bool isAlive;
    int turnOrder;
    bool hasActed;
    
    CombatParticipant(EntityId e, const std::string& n, bool player = false)
        : entity(e), name(n), isPlayer(player), isAlive(true), turnOrder(0), hasActed(false) {}
};

/**
 * Combat encounter structure
 */
struct CombatEncounter {
    std::string encounterId;
    std::vector<CombatParticipant> participants;
    std::queue<Components::CombatAction> actionQueue;
    CombatState state;
    int currentTurnIndex;
    int turnNumber;
    float turnTimer;
    
    CombatEncounter(const std::string& id)
        : encounterId(id), state(CombatState::Inactive), currentTurnIndex(0), 
          turnNumber(1), turnTimer(0.0f) {}
};

/**
 * Combat system
 * Manages turn-based combat mechanics, action processing, and combat state
 */
class CombatSystem : public System {
public:
    /**
     * Constructor
     */
    CombatSystem();
    
    /**
     * Destructor
     */
    ~CombatSystem();
    
    /**
     * Set entity manager
     * @param entityManager Entity manager
     */
    void setEntityManager(std::shared_ptr<EntityManager> entityManager) {
        m_entityManager = entityManager;
    }
    
    /**
     * Set component manager
     * @param componentManager Component manager
     */
    void setComponentManager(std::shared_ptr<ComponentManager> componentManager) {
        m_componentManager = componentManager;
    }
    
    // Combat management
    
    /**
     * Start combat encounter
     * @param encounterId Unique encounter identifier
     * @param playerEntities Player entities
     * @param enemyEntities Enemy entities
     * @return true if combat was started successfully
     */
    bool startCombat(const std::string& encounterId, 
                     const std::vector<EntityId>& playerEntities,
                     const std::vector<EntityId>& enemyEntities);
    
    /**
     * End current combat
     * @param victory Whether player won
     */
    void endCombat(bool victory);
    
    /**
     * Check if combat is active
     * @return true if combat is active
     */
    bool isCombatActive() const { return m_currentEncounter && m_currentEncounter->state != CombatState::Inactive; }
    
    /**
     * Get current combat state
     * @return Combat state
     */
    CombatState getCombatState() const { 
        return m_currentEncounter ? m_currentEncounter->state : CombatState::Inactive; 
    }
    
    /**
     * Get current encounter
     * @return Pointer to current encounter, or nullptr if no combat active
     */
    const CombatEncounter* getCurrentEncounter() const { return m_currentEncounter.get(); }
    
    // Turn management
    
    /**
     * Get current turn participant
     * @return Pointer to current participant, or nullptr if no active turn
     */
    const CombatParticipant* getCurrentTurnParticipant() const;
    
    /**
     * Advance to next turn
     */
    void nextTurn();
    
    /**
     * End current turn
     */
    void endTurn();
    
    /**
     * Get turn order
     * @return Vector of participants in turn order
     */
    std::vector<const CombatParticipant*> getTurnOrder() const;
    
    // Action management
    
    /**
     * Queue combat action
     * @param action Combat action to queue
     */
    void queueAction(const Components::CombatAction& action);
    
    /**
     * Process action queue
     */
    void processActionQueue();
    
    /**
     * Execute single combat action
     * @param action Action to execute
     * @return true if action was executed successfully
     */
    bool executeAction(const Components::CombatAction& action);
    
    /**
     * Execute attack action
     * @param action Attack action to execute
     * @return true if action was executed successfully
     */
    bool executeAttackAction(Components::CombatAction& action);
    
    /**
     * Execute skill/magic action
     * @param action Skill action to execute
     * @return true if action was executed successfully
     */
    bool executeSkillAction(Components::CombatAction& action);
    
    /**
     * Execute item action
     * @param action Item action to execute
     * @return true if action was executed successfully
     */
    bool executeItemAction(Components::CombatAction& action);
    
    /**
     * Execute defend action
     * @param action Defend action to execute
     * @return true if action was executed successfully
     */
    bool executeDefendAction(Components::CombatAction& action);
    
    /**
     * Execute escape action
     * @param action Escape action to execute
     * @return true if action was executed successfully
     */
    bool executeEscapeAction(Components::CombatAction& action);
    
    /**
     * Get available actions for entity
     * @param entity Entity to get actions for
     * @return Vector of available combat skills
     */
    std::vector<const Components::CombatSkill*> getAvailableActions(EntityId entity) const;
    
    /**
     * Get valid targets for action
     * @param actor Entity performing action
     * @param skill Skill being used
     * @return Vector of valid target entities
     */
    std::vector<EntityId> getValidTargets(EntityId actor, const Components::CombatSkill& skill) const;
    
    // Combat calculations
    
    /**
     * Calculate damage for action
     * @param action Combat action
     * @return Calculated damage amount
     */
    float calculateDamage(const Components::CombatAction& action) const;
    
    /**
     * Calculate hit chance
     * @param actor Attacking entity
     * @param target Target entity
     * @param skill Skill being used
     * @return Hit chance (0.0 to 1.0)
     */
    float calculateHitChance(EntityId actor, EntityId target, const Components::CombatSkill& skill) const;
    
    /**
     * Calculate critical hit chance
     * @param actor Attacking entity
     * @param skill Skill being used
     * @return Critical hit chance (0.0 to 1.0)
     */
    float calculateCriticalChance(EntityId actor, const Components::CombatSkill& skill) const;
    
    /**
     * Apply damage to entity
     * @param target Target entity
     * @param damage Damage amount
     * @param damageType Type of damage (for resistances)
     * @return Actual damage dealt
     */
    float applyDamage(EntityId target, float damage, const std::string& damageType = "physical");
    
    /**
     * Apply healing to entity
     * @param target Target entity
     * @param healing Healing amount
     * @return Actual healing applied
     */
    float applyHealing(EntityId target, float healing);
    
    // AI behavior
    
    /**
     * Process AI turn for entity
     * @param entity AI entity
     */
    void processAITurn(EntityId entity);
    
    /**
     * Get AI action for entity
     * @param entity AI entity
     * @return AI-selected combat action
     */
    Components::CombatAction getAIAction(EntityId entity) const;
    
    /**
     * Evaluate target priority for AI
     * @param aiEntity AI entity
     * @param target Potential target
     * @return Priority score (higher = more priority)
     */
    float evaluateTargetPriority(EntityId aiEntity, EntityId target) const;
    
    // Events and callbacks
    
    /**
     * Set combat start callback
     * @param callback Function called when combat starts
     */
    void setCombatStartCallback(std::function<void(const std::string&)> callback) {
        m_combatStartCallback = callback;
    }
    
    /**
     * Set combat end callback
     * @param callback Function called when combat ends (victory, escaped)
     */
    void setCombatEndCallback(std::function<void(bool, bool)> callback) {
        m_combatEndCallback = callback;
    }
    
    /**
     * Set turn start callback
     * @param callback Function called when turn starts (entity, turnNumber)
     */
    void setTurnStartCallback(std::function<void(EntityId, int)> callback) {
        m_turnStartCallback = callback;
    }
    
    /**
     * Set action executed callback
     * @param callback Function called when action is executed
     */
    void setActionExecutedCallback(std::function<void(const Components::CombatAction&)> callback) {
        m_actionExecutedCallback = callback;
    }
    
protected:
    // System overrides
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
public:
    /**
     * Apply status effect damage/healing
     * @param entity Entity to apply effects to
     */
    void applyStatusEffects(EntityId entity);
    
    /**
     * Apply equipment stat modifiers to entity
     * @param entity Entity to apply modifiers to
     */
    void applyEquipmentModifiers(EntityId entity);
    
    /**
     * Remove equipment stat modifiers from entity
     * @param entity Entity to remove modifiers from
     */
    void removeEquipmentModifiers(EntityId entity);

protected:

private:
    /**
     * Initialize combat encounter
     * @param encounter Encounter to initialize
     */
    void initializeEncounter(CombatEncounter& encounter);
    
    /**
     * Calculate turn order for participants
     * @param participants Combat participants
     */
    void calculateTurnOrder(std::vector<CombatParticipant>& participants);
    
    /**
     * Update combat state
     * @param deltaTime Time since last update
     */
    void updateCombatState(float deltaTime);
    
    /**
     * Check for combat end conditions
     */
    void checkCombatEndConditions();
    
    /**
     * Get participant by entity
     * @param entity Entity to find
     * @return Pointer to participant, or nullptr if not found
     */
    CombatParticipant* getParticipant(EntityId entity);
    
    /**
     * Get participant by entity (const version)
     * @param entity Entity to find
     * @return Pointer to participant, or nullptr if not found
     */
    const CombatParticipant* getParticipant(EntityId entity) const;
    
    /**
     * Update participant status
     * @param participant Participant to update
     */
    void updateParticipantStatus(CombatParticipant& participant);
    
    /**
     * Generate random number
     * @param min Minimum value
     * @param max Maximum value
     * @return Random number between min and max
     */
    float randomFloat(float min, float max);
    
    /**
     * Generate random integer
     * @param min Minimum value
     * @param max Maximum value
     * @return Random integer between min and max
     */
    int randomInt(int min, int max);
    
    // Managers
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // Combat state
    std::unique_ptr<CombatEncounter> m_currentEncounter;
    
    // Random number generation
    std::random_device m_randomDevice;
    std::mt19937 m_randomGenerator;
    
    // Callbacks
    std::function<void(const std::string&)> m_combatStartCallback;
    std::function<void(bool, bool)> m_combatEndCallback;
    std::function<void(EntityId, int)> m_turnStartCallback;
    std::function<void(const Components::CombatAction&)> m_actionExecutedCallback;
    
    // Configuration
    float m_turnTimeLimit;      // Time limit per turn (0 = no limit)
    bool m_autoEndTurn;         // Automatically end turn when time limit reached
    float m_actionDelay;        // Delay between actions for visual effects
};

} // namespace Systems
} // namespace RPGEngine