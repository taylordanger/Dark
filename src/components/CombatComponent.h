#pragma once

#include "Component.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace RPGEngine {
namespace Components {

/**
 * Combat action type enumeration
 */
enum class CombatActionType {
    Attack,         // Physical attack
    Magic,          // Magic spell
    Item,           // Use item
    Defend,         // Defend/guard
    Skill,          // Special skill/ability
    Escape          // Attempt to escape
};

/**
 * Combat status effect type
 */
enum class StatusEffectType {
    Poison,         // Damage over time
    Burn,           // Fire damage over time
    Freeze,         // Cannot act
    Stun,           // Cannot act for one turn
    Sleep,          // Cannot act until damaged
    Blind,          // Reduced accuracy
    Silence,        // Cannot use magic
    Haste,          // Increased speed
    Slow,           // Decreased speed
    Strength,       // Increased attack power
    Weakness,       // Decreased attack power
    Shield,         // Increased defense
    Vulnerable      // Decreased defense
};

/**
 * Status effect structure
 */
struct StatusEffect {
    StatusEffectType type;
    std::string name;
    std::string description;
    float duration;         // Duration in turns (-1 for permanent)
    float intensity;        // Effect strength
    bool beneficial;        // Whether this is a positive effect
    std::string source;     // Source of the effect (skill name, item, etc.)
    
    StatusEffect(StatusEffectType t, const std::string& n, float dur = 3.0f, float intens = 1.0f, bool benef = false)
        : type(t), name(n), duration(dur), intensity(intens), beneficial(benef) {}
};

/**
 * Combat skill/ability structure
 */
struct CombatSkill {
    std::string id;
    std::string name;
    std::string description;
    CombatActionType type;
    float mpCost;           // Mana cost
    float damage;           // Base damage
    float accuracy;         // Hit chance (0.0 to 1.0)
    float criticalChance;   // Critical hit chance (0.0 to 1.0)
    bool targetsSelf;       // Whether skill targets the user
    bool targetsAll;        // Whether skill targets all enemies/allies
    std::vector<StatusEffect> statusEffects;  // Status effects applied
    
    CombatSkill(const std::string& skillId, const std::string& skillName, CombatActionType actionType)
        : id(skillId), name(skillName), type(actionType), mpCost(0.0f), damage(0.0f), 
          accuracy(1.0f), criticalChance(0.0f), targetsSelf(false), targetsAll(false) {}
};

/**
 * Combat action structure
 */
struct CombatAction {
    EntityId actor;                 // Entity performing the action
    EntityId target;                // Target entity
    CombatActionType type;          // Type of action
    std::string skillId;            // Skill/spell/item ID
    float damage;                   // Calculated damage
    bool hit;                       // Whether action hit
    bool critical;                  // Whether action was critical
    std::vector<StatusEffect> appliedEffects;  // Status effects applied
    std::string description;        // Action description for UI
    
    CombatAction(EntityId a, EntityId t, CombatActionType actionType, const std::string& skill = "")
        : actor(a), target(t), type(actionType), skillId(skill), damage(0.0f), 
          hit(false), critical(false) {}
};

/**
 * Combat component
 * Manages combat-specific data for entities
 */
class CombatComponent : public Component<CombatComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     */
    explicit CombatComponent(EntityId entityId);
    
    /**
     * Destructor
     */
    ~CombatComponent();
    
    // Combat state
    
    /**
     * Check if entity is in combat
     * @return true if in combat
     */
    bool isInCombat() const { return m_inCombat; }
    
    /**
     * Set combat state
     * @param inCombat true if entering combat, false if leaving
     */
    void setInCombat(bool inCombat) { m_inCombat = inCombat; }
    
    /**
     * Get turn order priority
     * @return Turn order value (higher = goes first)
     */
    int getTurnOrder() const { return m_turnOrder; }
    
    /**
     * Set turn order priority
     * @param turnOrder Turn order value
     */
    void setTurnOrder(int turnOrder) { m_turnOrder = turnOrder; }
    
    /**
     * Check if it's this entity's turn
     * @return true if it's this entity's turn
     */
    bool isMyTurn() const { return m_isMyTurn; }
    
    /**
     * Set turn state
     * @param isMyTurn true if it's this entity's turn
     */
    void setMyTurn(bool isMyTurn) { m_isMyTurn = isMyTurn; }
    
    /**
     * Check if entity has acted this turn
     * @return true if entity has acted
     */
    bool hasActed() const { return m_hasActed; }
    
    /**
     * Set acted state
     * @param hasActed true if entity has acted
     */
    void setHasActed(bool hasActed) { m_hasActed = hasActed; }
    
    // Skills and abilities
    
    /**
     * Add combat skill
     * @param skill Skill to add
     */
    void addSkill(const CombatSkill& skill);
    
    /**
     * Remove combat skill
     * @param skillId Skill ID to remove
     * @return true if skill was removed
     */
    bool removeSkill(const std::string& skillId);
    
    /**
     * Get combat skill
     * @param skillId Skill ID
     * @return Pointer to skill, or nullptr if not found
     */
    const CombatSkill* getSkill(const std::string& skillId) const;
    
    /**
     * Get all skills
     * @return Vector of all skills
     */
    const std::vector<CombatSkill>& getSkills() const { return m_skills; }
    
    /**
     * Get available skills (can be used)
     * @return Vector of available skills
     */
    std::vector<const CombatSkill*> getAvailableSkills() const;
    
    // Status effects
    
    /**
     * Add status effect
     * @param effect Status effect to add
     */
    void addStatusEffect(const StatusEffect& effect);
    
    /**
     * Remove status effect
     * @param effectType Type of effect to remove
     * @return true if effect was removed
     */
    bool removeStatusEffect(StatusEffectType effectType);
    
    /**
     * Remove all status effects
     */
    void clearStatusEffects();
    
    /**
     * Get status effect
     * @param effectType Type of effect
     * @return Pointer to effect, or nullptr if not found
     */
    const StatusEffect* getStatusEffect(StatusEffectType effectType) const;
    
    /**
     * Get all status effects
     * @return Vector of all active status effects
     */
    const std::vector<StatusEffect>& getStatusEffects() const { return m_statusEffects; }
    
    /**
     * Check if has status effect
     * @param effectType Type of effect to check
     * @return true if effect is active
     */
    bool hasStatusEffect(StatusEffectType effectType) const;
    
    /**
     * Update status effects (reduce duration)
     * @param deltaTime Time since last update
     */
    void updateStatusEffects(float deltaTime);
    
    // Combat modifiers
    
    /**
     * Get attack power modifier from status effects
     * @return Attack power multiplier
     */
    float getAttackPowerModifier() const;
    
    /**
     * Get defense modifier from status effects
     * @return Defense multiplier
     */
    float getDefenseModifier() const;
    
    /**
     * Get accuracy modifier from status effects
     * @return Accuracy multiplier
     */
    float getAccuracyModifier() const;
    
    /**
     * Get speed modifier from status effects
     * @return Speed multiplier
     */
    float getSpeedModifier() const;
    
    /**
     * Check if can act (not stunned, frozen, etc.)
     * @return true if can perform actions
     */
    bool canAct() const;
    
    /**
     * Check if can use magic (not silenced)
     * @return true if can use magic
     */
    bool canUseMagic() const;
    
    // AI behavior (for enemies)
    
    /**
     * Set AI behavior pattern
     * @param behavior AI behavior identifier
     */
    void setAIBehavior(const std::string& behavior) { m_aiBehavior = behavior; }
    
    /**
     * Get AI behavior pattern
     * @return AI behavior identifier
     */
    const std::string& getAIBehavior() const { return m_aiBehavior; }
    
    /**
     * Set aggression level
     * @param aggression Aggression level (0.0 to 1.0)
     */
    void setAggression(float aggression) { m_aggression = aggression; }
    
    /**
     * Get aggression level
     * @return Aggression level
     */
    float getAggression() const { return m_aggression; }
    
    // Events and callbacks
    
    /**
     * Set action callback
     * @param callback Function called when action is performed
     */
    void setActionCallback(std::function<void(const CombatAction&)> callback) {
        m_actionCallback = callback;
    }
    
    /**
     * Set status effect callback
     * @param callback Function called when status effect is applied/removed
     */
    void setStatusEffectCallback(std::function<void(const StatusEffect&, bool)> callback) {
        m_statusEffectCallback = callback;
    }
    
    /**
     * Trigger action callback
     * @param action Combat action performed
     */
    void triggerActionCallback(const CombatAction& action);
    
    /**
     * Trigger status effect callback
     * @param effect Status effect
     * @param applied true if applied, false if removed
     */
    void triggerStatusEffectCallback(const StatusEffect& effect, bool applied);
    
private:
    // Combat state
    bool m_inCombat;
    int m_turnOrder;
    bool m_isMyTurn;
    bool m_hasActed;
    
    // Skills and abilities
    std::vector<CombatSkill> m_skills;
    
    // Status effects
    std::vector<StatusEffect> m_statusEffects;
    
    // AI behavior
    std::string m_aiBehavior;
    float m_aggression;
    
    // Callbacks
    std::function<void(const CombatAction&)> m_actionCallback;
    std::function<void(const StatusEffect&, bool)> m_statusEffectCallback;
};

} // namespace Components
} // namespace RPGEngine