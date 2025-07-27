#include "StatsComponent.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>

namespace RPGEngine {
namespace Components {

StatsComponent::StatsComponent(EntityId entityId)
    : Component<StatsComponent>(entityId)
    , m_baseMaxHP(100.0f)
    , m_currentHP(100.0f)
    , m_baseMaxMP(50.0f)
    , m_currentMP(50.0f)
    , m_experienceLevel(1, 100, 0)
    , m_currentExperience(0)
    , m_baseMovementSpeed(100.0f)
    , m_baseCriticalChance(0.05f)
{
    // Initialize default attributes
    m_baseAttributes[AttributeType::Strength] = 10;
    m_baseAttributes[AttributeType::Dexterity] = 10;
    m_baseAttributes[AttributeType::Intelligence] = 10;
    m_baseAttributes[AttributeType::Vitality] = 10;
    m_baseAttributes[AttributeType::Luck] = 10;
    m_baseAttributes[AttributeType::Charisma] = 10;
}

StatsComponent::~StatsComponent() {
    // Cleanup handled automatically
}

float StatsComponent::getMaxHP() const {
    float baseHP = m_baseMaxHP + (getAttribute(AttributeType::Vitality) * 5.0f);
    return calculateModifiedStat(baseHP, "hp");
}

void StatsComponent::setBaseMaxHP(float maxHP) {
    float oldMaxHP = getMaxHP();
    m_baseMaxHP = std::max(1.0f, maxHP);
    
    // Adjust current HP proportionally
    if (oldMaxHP > 0.0f) {
        float ratio = m_currentHP / oldMaxHP;
        m_currentHP = getMaxHP() * ratio;
    } else {
        m_currentHP = getMaxHP();
    }
    
    triggerStatChange("hp");
}

float StatsComponent::getMaxMP() const {
    float baseMP = m_baseMaxMP + (getAttribute(AttributeType::Intelligence) * 3.0f);
    return calculateModifiedStat(baseMP, "mp");
}

void StatsComponent::setBaseMaxMP(float maxMP) {
    float oldMaxMP = getMaxMP();
    m_baseMaxMP = std::max(0.0f, maxMP);
    
    // Adjust current MP proportionally
    if (oldMaxMP > 0.0f) {
        float ratio = m_currentMP / oldMaxMP;
        m_currentMP = getMaxMP() * ratio;
    } else {
        m_currentMP = getMaxMP();
    }
    
    triggerStatChange("mp");
}

float StatsComponent::modifyHP(float amount) {
    float oldHP = m_currentHP;
    setCurrentHP(m_currentHP + amount);
    return m_currentHP - oldHP;
}

float StatsComponent::modifyMP(float amount) {
    float oldMP = m_currentMP;
    setCurrentMP(m_currentMP + amount);
    return m_currentMP - oldMP;
}

void StatsComponent::setCurrentHP(float hp) {
    float oldHP = m_currentHP;
    m_currentHP = std::max(0.0f, std::min(hp, getMaxHP()));
    
    if (m_currentHP != oldHP) {
        triggerStatChange("hp");
        
        // Check for death
        if (oldHP > 0.0f && m_currentHP <= 0.0f) {
            triggerDeath();
        }
    }
}

void StatsComponent::setCurrentMP(float mp) {
    float oldMP = m_currentMP;
    m_currentMP = std::max(0.0f, std::min(mp, getMaxMP()));
    
    if (m_currentMP != oldMP) {
        triggerStatChange("mp");
    }
}

float StatsComponent::getHPPercentage() const {
    float maxHP = getMaxHP();
    return (maxHP > 0.0f) ? (m_currentHP / maxHP) : 0.0f;
}

float StatsComponent::getMPPercentage() const {
    float maxMP = getMaxMP();
    return (maxMP > 0.0f) ? (m_currentMP / maxMP) : 0.0f;
}

int StatsComponent::getExperienceToNextLevel() const {
    return m_experienceLevel.experienceRequired - m_currentExperience;
}

bool StatsComponent::addExperience(int experience) {
    if (experience <= 0) {
        return false;
    }
    
    int oldLevel = m_experienceLevel.level;
    m_currentExperience += experience;
    
    // Check for level up
    bool leveledUp = false;
    while (m_currentExperience >= m_experienceLevel.experienceRequired) {
        m_currentExperience -= m_experienceLevel.experienceRequired;
        m_experienceLevel.level++;
        
        // Calculate next level requirements
        m_experienceLevel.experienceTotal += m_experienceLevel.experienceRequired;
        m_experienceLevel.experienceRequired = calculateExperienceForLevel(m_experienceLevel.level + 1) - 
                                               calculateExperienceForLevel(m_experienceLevel.level);
        
        leveledUp = true;
    }
    
    if (leveledUp) {
        triggerLevelUp(oldLevel, m_experienceLevel.level);
        triggerStatChange("level");
    }
    
    triggerStatChange("experience");
    return leveledUp;
}

void StatsComponent::setLevel(int level) {
    if (level < 1) {
        level = 1;
    }
    
    int oldLevel = m_experienceLevel.level;
    m_experienceLevel.level = level;
    
    // Recalculate experience requirements
    m_experienceLevel.experienceTotal = calculateExperienceForLevel(level);
    m_experienceLevel.experienceRequired = calculateExperienceForLevel(level + 1) - 
                                          calculateExperienceForLevel(level);
    m_currentExperience = 0;
    
    if (oldLevel != level) {
        triggerLevelUp(oldLevel, level);
        triggerStatChange("level");
    }
}

float StatsComponent::getExperiencePercentage() const {
    return (m_experienceLevel.experienceRequired > 0) ? 
           (static_cast<float>(m_currentExperience) / static_cast<float>(m_experienceLevel.experienceRequired)) : 0.0f;
}

int StatsComponent::getAttribute(AttributeType attribute) const {
    int baseValue = getBaseAttribute(attribute);
    
    // Convert attribute type to string for modifier lookup
    std::string attributeName;
    switch (attribute) {
        case AttributeType::Strength: attributeName = "strength"; break;
        case AttributeType::Dexterity: attributeName = "dexterity"; break;
        case AttributeType::Intelligence: attributeName = "intelligence"; break;
        case AttributeType::Vitality: attributeName = "vitality"; break;
        case AttributeType::Luck: attributeName = "luck"; break;
        case AttributeType::Charisma: attributeName = "charisma"; break;
    }
    
    return static_cast<int>(calculateModifiedStat(static_cast<float>(baseValue), attributeName));
}

int StatsComponent::getBaseAttribute(AttributeType attribute) const {
    auto it = m_baseAttributes.find(attribute);
    return (it != m_baseAttributes.end()) ? it->second : 10;
}

void StatsComponent::setBaseAttribute(AttributeType attribute, int value) {
    int oldValue = getBaseAttribute(attribute);
    m_baseAttributes[attribute] = std::max(1, value);
    
    if (oldValue != value) {
        // Update derived stats that depend on this attribute
        switch (attribute) {
            case AttributeType::Vitality:
                triggerStatChange("hp");
                break;
            case AttributeType::Intelligence:
                triggerStatChange("mp");
                break;
            default:
                break;
        }
        
        triggerStatChange("attributes");
    }
}

void StatsComponent::modifyBaseAttribute(AttributeType attribute, int amount) {
    int currentValue = getBaseAttribute(attribute);
    setBaseAttribute(attribute, currentValue + amount);
}

void StatsComponent::addModifier(const std::string& stat, const StatModifier& modifier) {
    auto& modifiers = m_modifiers[stat];
    
    // Check if modifier already exists and is not stackable
    if (!modifier.stackable) {
        auto it = std::find_if(modifiers.begin(), modifiers.end(),
            [&modifier](const StatModifier& existing) {
                return existing.id == modifier.id;
            });
        
        if (it != modifiers.end()) {
            // Replace existing modifier
            *it = modifier;
        } else {
            modifiers.push_back(modifier);
        }
    } else {
        // Add stackable modifier
        modifiers.push_back(modifier);
    }
    
    triggerStatChange(stat);
    
    std::cout << "Added modifier '" << modifier.id << "' to stat '" << stat 
              << "' (value: " << modifier.value << ")" << std::endl;
}

bool StatsComponent::removeModifier(const std::string& stat, const std::string& modifierId) {
    auto it = m_modifiers.find(stat);
    if (it == m_modifiers.end()) {
        return false;
    }
    
    auto& modifiers = it->second;
    auto modIt = std::find_if(modifiers.begin(), modifiers.end(),
        [&modifierId](const StatModifier& modifier) {
            return modifier.id == modifierId;
        });
    
    if (modIt != modifiers.end()) {
        modifiers.erase(modIt);
        
        // Remove empty modifier list
        if (modifiers.empty()) {
            m_modifiers.erase(it);
        }
        
        triggerStatChange(stat);
        
        std::cout << "Removed modifier '" << modifierId << "' from stat '" << stat << "'" << std::endl;
        return true;
    }
    
    return false;
}

void StatsComponent::removeModifiersFromSource(const std::string& source) {
    std::vector<std::string> statsToUpdate;
    
    for (auto& pair : m_modifiers) {
        auto& modifiers = pair.second;
        
        auto it = std::remove_if(modifiers.begin(), modifiers.end(),
            [&source](const StatModifier& modifier) {
                return modifier.source == source;
            });
        
        if (it != modifiers.end()) {
            modifiers.erase(it, modifiers.end());
            statsToUpdate.push_back(pair.first);
        }
    }
    
    // Remove empty modifier lists
    auto mapIt = m_modifiers.begin();
    while (mapIt != m_modifiers.end()) {
        if (mapIt->second.empty()) {
            mapIt = m_modifiers.erase(mapIt);
        } else {
            ++mapIt;
        }
    }
    
    // Trigger stat change events
    for (const auto& stat : statsToUpdate) {
        triggerStatChange(stat);
    }
    
    if (!statsToUpdate.empty()) {
        std::cout << "Removed all modifiers from source '" << source << "'" << std::endl;
    }
}

std::vector<StatModifier> StatsComponent::getModifiers(const std::string& stat) const {
    auto it = m_modifiers.find(stat);
    return (it != m_modifiers.end()) ? it->second : std::vector<StatModifier>();
}

bool StatsComponent::hasModifier(const std::string& stat, const std::string& modifierId) const {
    auto it = m_modifiers.find(stat);
    if (it == m_modifiers.end()) {
        return false;
    }
    
    const auto& modifiers = it->second;
    return std::find_if(modifiers.begin(), modifiers.end(),
        [&modifierId](const StatModifier& modifier) {
            return modifier.id == modifierId;
        }) != modifiers.end();
}

void StatsComponent::updateModifiers(float deltaTime) {
    std::vector<std::string> statsToUpdate;
    
    for (auto& pair : m_modifiers) {
        auto& modifiers = pair.second;
        
        auto it = std::remove_if(modifiers.begin(), modifiers.end(),
            [deltaTime](StatModifier& modifier) {
                if (modifier.duration > 0.0f) {
                    modifier.duration -= deltaTime;
                    return modifier.duration <= 0.0f;
                }
                return false;
            });
        
        if (it != modifiers.end()) {
            modifiers.erase(it, modifiers.end());
            statsToUpdate.push_back(pair.first);
        }
    }
    
    // Remove empty modifier lists
    auto mapIt = m_modifiers.begin();
    while (mapIt != m_modifiers.end()) {
        if (mapIt->second.empty()) {
            mapIt = m_modifiers.erase(mapIt);
        } else {
            ++mapIt;
        }
    }
    
    // Trigger stat change events
    for (const auto& stat : statsToUpdate) {
        triggerStatChange(stat);
    }
}

int StatsComponent::getAttackPower() const {
    float basePower = getAttribute(AttributeType::Strength) * 2.0f;
    return static_cast<int>(calculateModifiedStat(basePower, "attack_power"));
}

int StatsComponent::getMagicPower() const {
    float basePower = getAttribute(AttributeType::Intelligence) * 2.0f;
    return static_cast<int>(calculateModifiedStat(basePower, "magic_power"));
}

int StatsComponent::getDefense() const {
    float baseDefense = getAttribute(AttributeType::Vitality) * 1.5f;
    return static_cast<int>(calculateModifiedStat(baseDefense, "defense"));
}

int StatsComponent::getMagicDefense() const {
    float baseDefense = getAttribute(AttributeType::Intelligence) * 1.5f;
    return static_cast<int>(calculateModifiedStat(baseDefense, "magic_defense"));
}

int StatsComponent::getAccuracy() const {
    float baseAccuracy = 75.0f + (getAttribute(AttributeType::Dexterity) * 2.0f);
    return static_cast<int>(calculateModifiedStat(baseAccuracy, "accuracy"));
}

int StatsComponent::getEvasion() const {
    float baseEvasion = getAttribute(AttributeType::Dexterity) * 1.5f;
    return static_cast<int>(calculateModifiedStat(baseEvasion, "evasion"));
}

float StatsComponent::getCriticalChance() const {
    float baseCritical = m_baseCriticalChance + (getAttribute(AttributeType::Luck) * 0.01f);
    return calculateModifiedStat(baseCritical, "critical_chance");
}

float StatsComponent::getMovementSpeed() const {
    float baseSpeed = m_baseMovementSpeed + (getAttribute(AttributeType::Dexterity) * 2.0f);
    return calculateModifiedStat(baseSpeed, "movement_speed");
}

std::string StatsComponent::serialize() const {
    std::ostringstream oss;
    
    // Serialize basic stats
    oss << m_baseMaxHP << "," << m_currentHP << "," << m_baseMaxMP << "," << m_currentMP << ",";
    oss << m_experienceLevel.level << "," << m_currentExperience << ",";
    
    // Serialize attributes
    for (const auto& pair : m_baseAttributes) {
        oss << static_cast<int>(pair.first) << ":" << pair.second << ";";
    }
    oss << ",";
    
    // Serialize modifiers
    for (const auto& statPair : m_modifiers) {
        oss << statPair.first << ":";
        for (const auto& modifier : statPair.second) {
            oss << modifier.id << "|" << modifier.source << "|" 
                << static_cast<int>(modifier.type) << "|" << modifier.value << "|" 
                << modifier.duration << "|" << (modifier.stackable ? 1 : 0) << ";";
        }
        oss << ",";
    }
    
    return oss.str();
}

bool StatsComponent::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    
    try {
        // Deserialize basic stats
        std::getline(iss, token, ',');
        m_baseMaxHP = std::stof(token);
        
        std::getline(iss, token, ',');
        m_currentHP = std::stof(token);
        
        std::getline(iss, token, ',');
        m_baseMaxMP = std::stof(token);
        
        std::getline(iss, token, ',');
        m_currentMP = std::stof(token);
        
        std::getline(iss, token, ',');
        m_experienceLevel.level = std::stoi(token);
        
        std::getline(iss, token, ',');
        m_currentExperience = std::stoi(token);
        
        // Deserialize attributes
        std::getline(iss, token, ',');
        if (!token.empty()) {
            std::istringstream attrStream(token);
            std::string attrToken;
            
            while (std::getline(attrStream, attrToken, ';')) {
                if (!attrToken.empty()) {
                    size_t colonPos = attrToken.find(':');
                    if (colonPos != std::string::npos) {
                        int attrType = std::stoi(attrToken.substr(0, colonPos));
                        int attrValue = std::stoi(attrToken.substr(colonPos + 1));
                        m_baseAttributes[static_cast<AttributeType>(attrType)] = attrValue;
                    }
                }
            }
        }
        
        // Deserialize modifiers
        m_modifiers.clear();
        std::string modifierData;
        while (std::getline(iss, modifierData, ',')) {
            if (!modifierData.empty()) {
                size_t colonPos = modifierData.find(':');
                if (colonPos != std::string::npos) {
                    std::string statName = modifierData.substr(0, colonPos);
                    std::string modifiersStr = modifierData.substr(colonPos + 1);
                    
                    std::istringstream modStream(modifiersStr);
                    std::string modToken;
                    
                    while (std::getline(modStream, modToken, ';')) {
                        if (!modToken.empty()) {
                            std::istringstream modParts(modToken);
                            std::string part;
                            std::vector<std::string> parts;
                            
                            while (std::getline(modParts, part, '|')) {
                                parts.push_back(part);
                            }
                            
                            if (parts.size() >= 6) {
                                StatModifier modifier(
                                    parts[0], // id
                                    parts[1], // source
                                    static_cast<ModifierType>(std::stoi(parts[2])), // type
                                    std::stof(parts[3]), // value
                                    std::stof(parts[4]), // duration
                                    parts[5] == "1" // stackable
                                );
                                
                                m_modifiers[statName].push_back(modifier);
                            }
                        }
                    }
                }
            }
        }
        
        // Recalculate experience requirements
        m_experienceLevel.experienceTotal = calculateExperienceForLevel(m_experienceLevel.level);
        m_experienceLevel.experienceRequired = calculateExperienceForLevel(m_experienceLevel.level + 1) - 
                                              calculateExperienceForLevel(m_experienceLevel.level);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize StatsComponent: " << e.what() << std::endl;
        return false;
    }
}

float StatsComponent::calculateModifiedStat(float baseStat, const std::string& stat) const {
    auto it = m_modifiers.find(stat);
    if (it == m_modifiers.end()) {
        return baseStat;
    }
    
    float result = baseStat;
    float flatBonus = 0.0f;
    float percentageMultiplier = 1.0f;
    float totalMultiplier = 1.0f;
    
    // Apply modifiers in order: flat -> percentage -> multiplier
    for (const auto& modifier : it->second) {
        switch (modifier.type) {
            case ModifierType::Flat:
                flatBonus += modifier.value;
                break;
            case ModifierType::Percentage:
                percentageMultiplier += (modifier.value / 100.0f);
                break;
            case ModifierType::Multiplier:
                totalMultiplier *= modifier.value;
                break;
        }
    }
    
    // Apply modifiers: (base + flat) * percentage * multiplier
    result = (result + flatBonus) * percentageMultiplier * totalMultiplier;
    
    return std::max(0.0f, result);
}

int StatsComponent::calculateExperienceForLevel(int level) const {
    // Exponential experience curve: level^2 * 50
    return (level - 1) * (level - 1) * 50;
}

void StatsComponent::triggerLevelUp(int oldLevel, int newLevel) {
    std::cout << "Level up! " << oldLevel << " -> " << newLevel << std::endl;
    
    // Increase attributes on level up
    int attributeIncrease = newLevel - oldLevel;
    for (auto& pair : m_baseAttributes) {
        pair.second += attributeIncrease;
    }
    
    // Restore HP and MP on level up
    setCurrentHP(getMaxHP());
    setCurrentMP(getMaxMP());
    
    if (m_levelUpCallback) {
        m_levelUpCallback(oldLevel, newLevel);
    }
}

void StatsComponent::triggerDeath() {
    std::cout << "Character has died!" << std::endl;
    
    if (m_deathCallback) {
        m_deathCallback();
    }
}

void StatsComponent::triggerStatChange(const std::string& statName) {
    if (m_statChangeCallback) {
        m_statChangeCallback(statName);
    }
}

} // namespace Components
} // namespace RPGEngine