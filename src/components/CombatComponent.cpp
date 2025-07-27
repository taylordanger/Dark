#include "CombatComponent.h"
#include <algorithm>

namespace RPGEngine {
namespace Components {

CombatComponent::CombatComponent(EntityId entityId)
    : Component<CombatComponent>(entityId)
    , m_inCombat(false)
    , m_turnOrder(0)
    , m_isMyTurn(false)
    , m_hasActed(false)
    , m_aiBehavior("default")
    , m_aggression(0.5f)
{
}

CombatComponent::~CombatComponent() = default;

void CombatComponent::addSkill(const CombatSkill& skill) {
    // Check if skill already exists
    auto it = std::find_if(m_skills.begin(), m_skills.end(),
        [&skill](const CombatSkill& existing) {
            return existing.id == skill.id;
        });
    
    if (it != m_skills.end()) {
        // Update existing skill
        *it = skill;
    } else {
        // Add new skill
        m_skills.push_back(skill);
    }
}

bool CombatComponent::removeSkill(const std::string& skillId) {
    auto it = std::find_if(m_skills.begin(), m_skills.end(),
        [&skillId](const CombatSkill& skill) {
            return skill.id == skillId;
        });
    
    if (it != m_skills.end()) {
        m_skills.erase(it);
        return true;
    }
    
    return false;
}

const CombatSkill* CombatComponent::getSkill(const std::string& skillId) const {
    auto it = std::find_if(m_skills.begin(), m_skills.end(),
        [&skillId](const CombatSkill& skill) {
            return skill.id == skillId;
        });
    
    return (it != m_skills.end()) ? &(*it) : nullptr;
}

std::vector<const CombatSkill*> CombatComponent::getAvailableSkills() const {
    std::vector<const CombatSkill*> availableSkills;
    
    for (const auto& skill : m_skills) {
        // Check if skill can be used (enough MP, not silenced for magic, etc.)
        if (skill.type == CombatActionType::Magic && !canUseMagic()) {
            continue;
        }
        
        // Add other availability checks here (MP cost, cooldowns, etc.)
        availableSkills.push_back(&skill);
    }
    
    return availableSkills;
}

void CombatComponent::addStatusEffect(const StatusEffect& effect) {
    // Check if effect already exists
    auto it = std::find_if(m_statusEffects.begin(), m_statusEffects.end(),
        [&effect](const StatusEffect& existing) {
            return existing.type == effect.type;
        });
    
    if (it != m_statusEffects.end()) {
        // Update existing effect (refresh duration, update intensity)
        it->duration = std::max(it->duration, effect.duration);
        it->intensity = std::max(it->intensity, effect.intensity);
    } else {
        // Add new effect
        m_statusEffects.push_back(effect);
        triggerStatusEffectCallback(effect, true);
    }
}

bool CombatComponent::removeStatusEffect(StatusEffectType effectType) {
    auto it = std::find_if(m_statusEffects.begin(), m_statusEffects.end(),
        [effectType](const StatusEffect& effect) {
            return effect.type == effectType;
        });
    
    if (it != m_statusEffects.end()) {
        StatusEffect removedEffect = *it;
        m_statusEffects.erase(it);
        triggerStatusEffectCallback(removedEffect, false);
        return true;
    }
    
    return false;
}

void CombatComponent::clearStatusEffects() {
    for (const auto& effect : m_statusEffects) {
        triggerStatusEffectCallback(effect, false);
    }
    m_statusEffects.clear();
}

const StatusEffect* CombatComponent::getStatusEffect(StatusEffectType effectType) const {
    auto it = std::find_if(m_statusEffects.begin(), m_statusEffects.end(),
        [effectType](const StatusEffect& effect) {
            return effect.type == effectType;
        });
    
    return (it != m_statusEffects.end()) ? &(*it) : nullptr;
}

bool CombatComponent::hasStatusEffect(StatusEffectType effectType) const {
    return getStatusEffect(effectType) != nullptr;
}

void CombatComponent::updateStatusEffects(float deltaTime) {
    auto it = m_statusEffects.begin();
    while (it != m_statusEffects.end()) {
        if (it->duration > 0.0f) {
            it->duration -= deltaTime;
            if (it->duration <= 0.0f) {
                StatusEffect expiredEffect = *it;
                it = m_statusEffects.erase(it);
                triggerStatusEffectCallback(expiredEffect, false);
                continue;
            }
        }
        ++it;
    }
}

float CombatComponent::getAttackPowerModifier() const {
    float modifier = 1.0f;
    
    for (const auto& effect : m_statusEffects) {
        switch (effect.type) {
            case StatusEffectType::Strength:
                modifier *= (1.0f + effect.intensity * 0.5f);
                break;
            case StatusEffectType::Weakness:
                modifier *= (1.0f - effect.intensity * 0.3f);
                break;
            default:
                break;
        }
    }
    
    return modifier;
}

float CombatComponent::getDefenseModifier() const {
    float modifier = 1.0f;
    
    for (const auto& effect : m_statusEffects) {
        switch (effect.type) {
            case StatusEffectType::Shield:
                modifier *= (1.0f + effect.intensity * 0.5f);
                break;
            case StatusEffectType::Vulnerable:
                modifier *= (1.0f - effect.intensity * 0.3f);
                break;
            default:
                break;
        }
    }
    
    return modifier;
}

float CombatComponent::getAccuracyModifier() const {
    float modifier = 1.0f;
    
    for (const auto& effect : m_statusEffects) {
        switch (effect.type) {
            case StatusEffectType::Blind:
                modifier *= (1.0f - effect.intensity * 0.5f);
                break;
            default:
                break;
        }
    }
    
    return modifier;
}

float CombatComponent::getSpeedModifier() const {
    float modifier = 1.0f;
    
    for (const auto& effect : m_statusEffects) {
        switch (effect.type) {
            case StatusEffectType::Haste:
                modifier *= (1.0f + effect.intensity * 0.5f);
                break;
            case StatusEffectType::Slow:
                modifier *= (1.0f - effect.intensity * 0.3f);
                break;
            default:
                break;
        }
    }
    
    return modifier;
}

bool CombatComponent::canAct() const {
    // Check for status effects that prevent action
    for (const auto& effect : m_statusEffects) {
        switch (effect.type) {
            case StatusEffectType::Freeze:
            case StatusEffectType::Stun:
            case StatusEffectType::Sleep:
                return false;
            default:
                break;
        }
    }
    
    return true;
}

bool CombatComponent::canUseMagic() const {
    // Check for silence effect
    return !hasStatusEffect(StatusEffectType::Silence);
}

void CombatComponent::triggerActionCallback(const CombatAction& action) {
    if (m_actionCallback) {
        m_actionCallback(action);
    }
}

void CombatComponent::triggerStatusEffectCallback(const StatusEffect& effect, bool applied) {
    if (m_statusEffectCallback) {
        m_statusEffectCallback(effect, applied);
    }
}

} // namespace Components
} // namespace RPGEngine