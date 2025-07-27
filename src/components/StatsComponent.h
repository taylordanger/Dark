#pragma once

#include "Component.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace RPGEngine {
namespace Components {

/**
 * Attribute type enumeration
 */
enum class AttributeType {
    Strength,       // Physical power
    Dexterity,      // Agility and precision
    Intelligence,   // Magical power and mana
    Vitality,       // Health and endurance
    Luck,           // Critical hit chance and item drops
    Charisma        // Social interactions and prices
};

/**
 * Stat modifier type
 */
enum class ModifierType {
    Flat,           // Flat bonus/penalty (+10 HP)
    Percentage,     // Percentage bonus/penalty (+10% HP)
    Multiplier      // Multiplier (x1.5 damage)
};

/**
 * Stat modifier structure
 */
struct StatModifier {
    std::string id;             // Unique modifier ID
    std::string source;         // Source of modifier (equipment, spell, etc.)
    ModifierType type;          // Type of modifier
    float value;                // Modifier value
    float duration;             // Duration in seconds (-1 for permanent)
    bool stackable;             // Whether multiple instances can stack
    
    StatModifier(const std::string& modId, const std::string& modSource, 
                ModifierType modType, float modValue, float modDuration = -1.0f, bool stack = false)
        : id(modId), source(modSource), type(modType), value(modValue), 
          duration(modDuration), stackable(stack) {}
};

/**
 * Experience level structure
 */
struct ExperienceLevel {
    int level;
    int experienceRequired;
    int experienceTotal;
    
    ExperienceLevel(int lvl = 1, int required = 100, int total = 0)
        : level(lvl), experienceRequired(required), experienceTotal(total) {}
};

/**
 * Stats component
 * Manages character statistics, attributes, and progression
 */
class StatsComponent : public Component<StatsComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     */
    explicit StatsComponent(EntityId entityId);
    
    /**
     * Destructor
     */
    ~StatsComponent();
    
    // Core stats
    
    /**
     * Get current health points
     * @return Current HP
     */
    float getCurrentHP() const { return m_currentHP; }
    
    /**
     * Get maximum health points (with modifiers)
     * @return Maximum HP
     */
    float getMaxHP() const;
    
    /**
     * Get base maximum health points (without modifiers)
     * @return Base maximum HP
     */
    float getBaseMaxHP() const { return m_baseMaxHP; }
    
    /**
     * Set base maximum health points
     * @param maxHP Base maximum HP
     */
    void setBaseMaxHP(float maxHP);
    
    /**
     * Get current mana points
     * @return Current MP
     */
    float getCurrentMP() const { return m_currentMP; }
    
    /**
     * Get maximum mana points (with modifiers)
     * @return Maximum MP
     */
    float getMaxMP() const;
    
    /**
     * Get base maximum mana points (without modifiers)
     * @return Base maximum MP
     */
    float getBaseMaxMP() const { return m_baseMaxMP; }
    
    /**
     * Set base maximum mana points
     * @param maxMP Base maximum MP
     */
    void setBaseMaxMP(float maxMP);
    
    /**
     * Modify current HP
     * @param amount Amount to add/subtract
     * @return Actual amount changed
     */
    float modifyHP(float amount);
    
    /**
     * Modify current MP
     * @param amount Amount to add/subtract
     * @return Actual amount changed
     */
    float modifyMP(float amount);
    
    /**
     * Set current HP
     * @param hp New HP value (clamped to 0-maxHP)
     */
    void setCurrentHP(float hp);
    
    /**
     * Set current MP
     * @param mp New MP value (clamped to 0-maxMP)
     */
    void setCurrentMP(float mp);
    
    /**
     * Check if character is alive
     * @return true if HP > 0
     */
    bool isAlive() const { return m_currentHP > 0.0f; }
    
    /**
     * Get HP percentage
     * @return HP as percentage (0.0 to 1.0)
     */
    float getHPPercentage() const;
    
    /**
     * Get MP percentage
     * @return MP as percentage (0.0 to 1.0)
     */
    float getMPPercentage() const;
    
    // Level and experience
    
    /**
     * Get current level
     * @return Current level
     */
    int getLevel() const { return m_experienceLevel.level; }
    
    /**
     * Get current experience points
     * @return Current experience
     */
    int getCurrentExperience() const { return m_currentExperience; }
    
    /**
     * Get experience required for next level
     * @return Experience required
     */
    int getExperienceToNextLevel() const;
    
    /**
     * Get total experience for current level
     * @return Total experience required for current level
     */
    int getTotalExperienceForLevel() const { return m_experienceLevel.experienceTotal; }
    
    /**
     * Add experience points
     * @param experience Experience to add
     * @return true if level up occurred
     */
    bool addExperience(int experience);
    
    /**
     * Set level directly
     * @param level New level
     */
    void setLevel(int level);
    
    /**
     * Get experience percentage for current level
     * @return Experience progress as percentage (0.0 to 1.0)
     */
    float getExperiencePercentage() const;
    
    // Attributes
    
    /**
     * Get attribute value (with modifiers)
     * @param attribute Attribute type
     * @return Attribute value
     */
    int getAttribute(AttributeType attribute) const;
    
    /**
     * Get base attribute value (without modifiers)
     * @param attribute Attribute type
     * @return Base attribute value
     */
    int getBaseAttribute(AttributeType attribute) const;
    
    /**
     * Set base attribute value
     * @param attribute Attribute type
     * @param value New base value
     */
    void setBaseAttribute(AttributeType attribute, int value);
    
    /**
     * Modify base attribute
     * @param attribute Attribute type
     * @param amount Amount to add/subtract
     */
    void modifyBaseAttribute(AttributeType attribute, int amount);
    
    /**
     * Get all base attributes
     * @return Map of attribute types to base values
     */
    const std::unordered_map<AttributeType, int>& getBaseAttributes() const { return m_baseAttributes; }
    
    // Stat modifiers
    
    /**
     * Add stat modifier
     * @param stat Stat name (e.g., "hp", "mp", "strength")
     * @param modifier Modifier to add
     */
    void addModifier(const std::string& stat, const StatModifier& modifier);
    
    /**
     * Remove stat modifier
     * @param stat Stat name
     * @param modifierId Modifier ID to remove
     * @return true if modifier was removed
     */
    bool removeModifier(const std::string& stat, const std::string& modifierId);
    
    /**
     * Remove all modifiers from source
     * @param source Source to remove modifiers from
     */
    void removeModifiersFromSource(const std::string& source);
    
    /**
     * Get modifiers for stat
     * @param stat Stat name
     * @return Vector of modifiers
     */
    std::vector<StatModifier> getModifiers(const std::string& stat) const;
    
    /**
     * Check if has modifier
     * @param stat Stat name
     * @param modifierId Modifier ID
     * @return true if modifier exists
     */
    bool hasModifier(const std::string& stat, const std::string& modifierId) const;
    
    /**
     * Update modifiers (remove expired ones)
     * @param deltaTime Time since last update
     */
    void updateModifiers(float deltaTime);
    
    // Derived stats
    
    /**
     * Get attack power
     * @return Attack power based on strength and modifiers
     */
    int getAttackPower() const;
    
    /**
     * Get magic power
     * @return Magic power based on intelligence and modifiers
     */
    int getMagicPower() const;
    
    /**
     * Get defense
     * @return Defense based on vitality and modifiers
     */
    int getDefense() const;
    
    /**
     * Get magic defense
     * @return Magic defense based on intelligence and modifiers
     */
    int getMagicDefense() const;
    
    /**
     * Get accuracy
     * @return Accuracy based on dexterity and modifiers
     */
    int getAccuracy() const;
    
    /**
     * Get evasion
     * @return Evasion based on dexterity and modifiers
     */
    int getEvasion() const;
    
    /**
     * Get critical hit chance
     * @return Critical hit chance as percentage (0.0 to 1.0)
     */
    float getCriticalChance() const;
    
    /**
     * Get movement speed
     * @return Movement speed based on dexterity and modifiers
     */
    float getMovementSpeed() const;
    
    // Events and callbacks
    
    /**
     * Set level up callback
     * @param callback Function called when level up occurs
     */
    void setLevelUpCallback(std::function<void(int, int)> callback) {
        m_levelUpCallback = callback;
    }
    
    /**
     * Set death callback
     * @param callback Function called when HP reaches 0
     */
    void setDeathCallback(std::function<void()> callback) {
        m_deathCallback = callback;
    }
    
    /**
     * Set stat change callback
     * @param callback Function called when stats change
     */
    void setStatChangeCallback(std::function<void(const std::string&)> callback) {
        m_statChangeCallback = callback;
    }
    
    // Serialization
    
    /**
     * Serialize component data
     * @return Serialized data
     */
    std::string serialize() const;
    
    /**
     * Deserialize component data
     * @param data Serialized data
     * @return true if successful
     */
    bool deserialize(const std::string& data);
    
private:
    /**
     * Calculate modified stat value
     * @param baseStat Base stat value
     * @param stat Stat name for modifiers
     * @return Modified stat value
     */
    float calculateModifiedStat(float baseStat, const std::string& stat) const;
    
    /**
     * Calculate experience required for level
     * @param level Target level
     * @return Experience required
     */
    int calculateExperienceForLevel(int level) const;
    
    /**
     * Trigger level up
     * @param oldLevel Previous level
     * @param newLevel New level
     */
    void triggerLevelUp(int oldLevel, int newLevel);
    
    /**
     * Trigger death
     */
    void triggerDeath();
    
    /**
     * Trigger stat change
     * @param statName Name of changed stat
     */
    void triggerStatChange(const std::string& statName);
    
    // Core stats
    float m_baseMaxHP;
    float m_currentHP;
    float m_baseMaxMP;
    float m_currentMP;
    
    // Level and experience
    ExperienceLevel m_experienceLevel;
    int m_currentExperience;
    
    // Attributes
    std::unordered_map<AttributeType, int> m_baseAttributes;
    
    // Stat modifiers
    std::unordered_map<std::string, std::vector<StatModifier>> m_modifiers;
    
    // Callbacks
    std::function<void(int, int)> m_levelUpCallback;        // (oldLevel, newLevel)
    std::function<void()> m_deathCallback;
    std::function<void(const std::string&)> m_statChangeCallback;  // (statName)
    
    // Configuration
    float m_baseMovementSpeed;
    float m_baseCriticalChance;
};

} // namespace Components
} // namespace RPGEngine