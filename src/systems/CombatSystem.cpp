#include "CombatSystem.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
#include <algorithm>
#include <cmath>

namespace RPGEngine {
namespace Systems {

CombatSystem::CombatSystem()
    : System("CombatSystem")
    , m_randomGenerator(m_randomDevice())
    , m_turnTimeLimit(0.0f)
    , m_autoEndTurn(false)
    , m_actionDelay(1.0f)
{
    // Set system priority (combat should run after physics but before rendering)
    setPriority(300);
    
    // Add dependencies
    addDependency(SystemType::ECS);
}

CombatSystem::~CombatSystem() = default;

bool CombatSystem::onInitialize() {
    if (!m_entityManager || !m_componentManager) {
        return false;
    }
    
    return true;
}

void CombatSystem::onUpdate(float deltaTime) {
    if (!isCombatActive()) {
        return;
    }
    
    updateCombatState(deltaTime);
    
    // Update status effects for all combat participants
    if (m_currentEncounter) {
        for (auto& participant : m_currentEncounter->participants) {
            if (participant.isAlive) {
                applyStatusEffects(participant.entity);
                
                auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
                if (combatComp) {
                    combatComp->updateStatusEffects(deltaTime);
                }
            }
        }
    }
}

void CombatSystem::onShutdown() {
    if (isCombatActive()) {
        endCombat(false);
    }
}

bool CombatSystem::startCombat(const std::string& encounterId, 
                               const std::vector<EntityId>& playerEntities,
                               const std::vector<EntityId>& enemyEntities) {
    if (isCombatActive()) {
        return false; // Combat already active
    }
    
    // Create new encounter
    m_currentEncounter = std::make_unique<CombatEncounter>(encounterId);
    
    // Add player participants
    for (EntityId entity : playerEntities) {
        auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(entity));
        if (statsComp && statsComp->isAlive()) {
            std::string name = "Player"; // TODO: Get actual name from component
            m_currentEncounter->participants.emplace_back(entity, name, true);
        }
    }
    
    // Add enemy participants
    for (EntityId entity : enemyEntities) {
        auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(entity));
        if (statsComp && statsComp->isAlive()) {
            std::string name = "Enemy"; // TODO: Get actual name from component
            m_currentEncounter->participants.emplace_back(entity, name, false);
        }
    }
    
    if (m_currentEncounter->participants.empty()) {
        m_currentEncounter.reset();
        return false;
    }
    
    // Initialize encounter
    initializeEncounter(*m_currentEncounter);
    
    // Trigger combat start callback
    if (m_combatStartCallback) {
        m_combatStartCallback(encounterId);
    }
    
    return true;
}

void CombatSystem::endCombat(bool victory) {
    if (!isCombatActive()) {
        return;
    }
    
    // Clear combat state from all participants
    for (auto& participant : m_currentEncounter->participants) {
        auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
        if (combatComp) {
            combatComp->setInCombat(false);
            combatComp->setMyTurn(false);
            combatComp->setHasActed(false);
            combatComp->clearStatusEffects();
        }
    }
    
    // Trigger combat end callback
    if (m_combatEndCallback) {
        bool escaped = (m_currentEncounter->state == CombatState::Escaped);
        m_combatEndCallback(victory, escaped);
    }
    
    // Clear encounter
    m_currentEncounter.reset();
}

const CombatParticipant* CombatSystem::getCurrentTurnParticipant() const {
    if (!m_currentEncounter || m_currentEncounter->participants.empty()) {
        return nullptr;
    }
    
    if (m_currentEncounter->currentTurnIndex >= 0 && 
        m_currentEncounter->currentTurnIndex < static_cast<int>(m_currentEncounter->participants.size())) {
        return &m_currentEncounter->participants[m_currentEncounter->currentTurnIndex];
    }
    
    return nullptr;
}

void CombatSystem::nextTurn() {
    if (!m_currentEncounter) {
        return;
    }
    
    // Find next participant who can act
    int startIndex = m_currentEncounter->currentTurnIndex;
    do {
        m_currentEncounter->currentTurnIndex = 
            (m_currentEncounter->currentTurnIndex + 1) % m_currentEncounter->participants.size();
        
        auto& participant = m_currentEncounter->participants[m_currentEncounter->currentTurnIndex];
        
        // Check if participant is alive and can act
        if (participant.isAlive && !participant.hasActed) {
            auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
            if (combatComp && combatComp->canAct()) {
                // Set turn state
                combatComp->setMyTurn(true);
                m_currentEncounter->state = participant.isPlayer ? CombatState::PlayerTurn : CombatState::EnemyTurn;
                m_currentEncounter->turnTimer = 0.0f;
                
                // Trigger turn start callback
                if (m_turnStartCallback) {
                    m_turnStartCallback(participant.entity, m_currentEncounter->turnNumber);
                }
                
                // Process AI turn if it's an enemy
                if (!participant.isPlayer) {
                    processAITurn(participant.entity);
                }
                
                return;
            }
        }
    } while (m_currentEncounter->currentTurnIndex != startIndex);
    
    // If we've gone through all participants, start new turn
    endTurn();
}

void CombatSystem::endTurn() {
    if (!m_currentEncounter) {
        return;
    }
    
    // Clear turn state from current participant
    const auto* currentParticipant = getCurrentTurnParticipant();
    if (currentParticipant) {
        auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(currentParticipant->entity));
        if (combatComp) {
            combatComp->setMyTurn(false);
        }
    }
    
    // Check if all participants have acted
    bool allActed = true;
    for (auto& participant : m_currentEncounter->participants) {
        if (participant.isAlive && !participant.hasActed) {
            auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
            if (combatComp && combatComp->canAct()) {
                allActed = false;
                break;
            }
        }
    }
    
    if (allActed) {
        // Start new turn
        m_currentEncounter->turnNumber++;
        for (auto& participant : m_currentEncounter->participants) {
            participant.hasActed = false;
            auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
            if (combatComp) {
                combatComp->setHasActed(false);
            }
        }
        
        // Recalculate turn order (in case speed changed)
        calculateTurnOrder(m_currentEncounter->participants);
        m_currentEncounter->currentTurnIndex = -1;
    }
    
    // Move to next turn
    nextTurn();
}

std::vector<const CombatParticipant*> CombatSystem::getTurnOrder() const {
    std::vector<const CombatParticipant*> turnOrder;
    
    if (m_currentEncounter) {
        for (const auto& participant : m_currentEncounter->participants) {
            if (participant.isAlive) {
                turnOrder.push_back(&participant);
            }
        }
        
        // Sort by turn order
        std::sort(turnOrder.begin(), turnOrder.end(),
            [](const CombatParticipant* a, const CombatParticipant* b) {
                return a->turnOrder > b->turnOrder;
            });
    }
    
    return turnOrder;
}

void CombatSystem::queueAction(const Components::CombatAction& action) {
    if (m_currentEncounter) {
        m_currentEncounter->actionQueue.push(action);
    }
}

void CombatSystem::processActionQueue() {
    if (!m_currentEncounter) {
        return;
    }
    
    while (!m_currentEncounter->actionQueue.empty()) {
        auto action = m_currentEncounter->actionQueue.front();
        m_currentEncounter->actionQueue.pop();
        
        executeAction(action);
    }
}

bool CombatSystem::executeAction(const Components::CombatAction& action) {
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.actor));
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    auto actorInventory = m_componentManager->getComponent<Components::InventoryComponent>(Entity(action.actor));
    
    if (!actorStats || !actorCombat || !actorCombat->canAct()) {
        return false;
    }
    
    // Create mutable copy of action for processing
    Components::CombatAction processedAction = action;
    
    // Handle different action types
    switch (action.type) {
        case Components::CombatActionType::Attack:
            return executeAttackAction(processedAction);
        case Components::CombatActionType::Magic:
        case Components::CombatActionType::Skill:
            return executeSkillAction(processedAction);
        case Components::CombatActionType::Item:
            return executeItemAction(processedAction);
        case Components::CombatActionType::Defend:
            return executeDefendAction(processedAction);
        case Components::CombatActionType::Escape:
            return executeEscapeAction(processedAction);
        default:
            return false;
    }
}

std::vector<const Components::CombatSkill*> CombatSystem::getAvailableActions(EntityId entity) const {
    auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(entity));
    if (!combatComp) {
        return {};
    }
    
    return combatComp->getAvailableSkills();
}

std::vector<EntityId> CombatSystem::getValidTargets(EntityId actor, const Components::CombatSkill& skill) const {
    std::vector<EntityId> targets;
    
    if (!m_currentEncounter) {
        return targets;
    }
    
    auto* actorParticipant = getParticipant(actor);
    if (!actorParticipant) {
        return targets;
    }
    
    for (const auto& participant : m_currentEncounter->participants) {
        if (!participant.isAlive) {
            continue;
        }
        
        if (skill.targetsSelf && participant.entity == actor) {
            targets.push_back(participant.entity);
        } else if (!skill.targetsSelf && participant.entity != actor) {
            // Check if target is valid based on skill type and participant alignment
            bool validTarget = false;
            
            if (skill.type == Components::CombatActionType::Attack || 
                skill.type == Components::CombatActionType::Magic) {
                // Offensive skills target enemies
                validTarget = (participant.isPlayer != actorParticipant->isPlayer);
            } else {
                // Defensive/support skills target allies
                validTarget = (participant.isPlayer == actorParticipant->isPlayer);
            }
            
            if (validTarget) {
                targets.push_back(participant.entity);
            }
        }
    }
    
    return targets;
}

float CombatSystem::calculateDamage(const Components::CombatAction& action) const {
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.actor));
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    auto targetStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.target));
    auto targetCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.target));
    
    if (!actorStats || !targetStats) {
        return 0.0f;
    }
    
    float baseDamage = 0.0f;
    
    // Get skill information
    const auto* skill = actorCombat ? actorCombat->getSkill(action.skillId) : nullptr;
    
    if (skill) {
        baseDamage = skill->damage;
        
        // Add attribute-based damage
        if (skill->type == Components::CombatActionType::Attack) {
            baseDamage += actorStats->getAttackPower();
        } else if (skill->type == Components::CombatActionType::Magic) {
            baseDamage += actorStats->getMagicPower();
        }
    } else {
        // Basic attack
        baseDamage = actorStats->getAttackPower();
    }
    
    // Apply attacker modifiers
    if (actorCombat) {
        baseDamage *= actorCombat->getAttackPowerModifier();
    }
    
    // Apply defender modifiers
    float defense = targetStats->getDefense();
    if (targetCombat) {
        defense *= targetCombat->getDefenseModifier();
    }
    
    // Calculate final damage (simple formula)
    float finalDamage = std::max(1.0f, baseDamage - defense * 0.5f);
    
    // Add some randomness
    finalDamage *= const_cast<CombatSystem*>(this)->randomFloat(0.8f, 1.2f);
    
    return finalDamage;
}

float CombatSystem::calculateHitChance(EntityId actor, EntityId target, const Components::CombatSkill& skill) const {
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(actor));
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(actor));
    auto targetStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(target));
    auto targetCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(target));
    
    if (!actorStats || !targetStats) {
        return 0.0f;
    }
    
    float baseAccuracy = skill.accuracy;
    float actorAccuracy = actorStats->getAccuracy();
    float targetEvasion = targetStats->getEvasion();
    
    // Apply modifiers
    if (actorCombat) {
        actorAccuracy *= actorCombat->getAccuracyModifier();
    }
    
    // Calculate hit chance
    float hitChance = baseAccuracy * (actorAccuracy / (actorAccuracy + targetEvasion));
    
    return std::clamp(hitChance, 0.05f, 0.95f); // Min 5%, max 95%
}

float CombatSystem::calculateCriticalChance(EntityId actor, const Components::CombatSkill& skill) const {
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(actor));
    if (!actorStats) {
        return 0.0f;
    }
    
    float baseCritical = skill.criticalChance;
    float actorCritical = actorStats->getCriticalChance();
    
    return std::clamp(baseCritical + actorCritical, 0.0f, 0.5f); // Max 50% critical chance
}

float CombatSystem::applyDamage(EntityId target, float damage, const std::string& damageType) {
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(target));
    if (!statsComp) {
        return 0.0f;
    }
    
    float actualDamage = std::min(damage, statsComp->getCurrentHP());
    statsComp->modifyHP(-actualDamage);
    
    // Update participant status
    auto* participant = getParticipant(target);
    if (participant) {
        updateParticipantStatus(*participant);
    }
    
    return actualDamage;
}

float CombatSystem::applyHealing(EntityId target, float healing) {
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(target));
    if (!statsComp) {
        return 0.0f;
    }
    
    float actualHealing = std::min(healing, statsComp->getMaxHP() - statsComp->getCurrentHP());
    statsComp->modifyHP(actualHealing);
    
    // Update participant status
    auto* participant = getParticipant(target);
    if (participant) {
        updateParticipantStatus(*participant);
    }
    
    return actualHealing;
}

bool CombatSystem::executeAttackAction(Components::CombatAction& action) {
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.actor));
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    
    if (!actorStats || !actorCombat) {
        return false;
    }
    
    // Basic attack uses weapon stats if equipped
    auto actorInventory = m_componentManager->getComponent<Components::InventoryComponent>(Entity(action.actor));
    float weaponDamage = 0.0f;
    float weaponAccuracy = 0.95f;
    float weaponCritical = 0.05f;
    
    if (actorInventory) {
        const auto& weapon = actorInventory->getEquippedItem(Components::EquipmentSlot::Weapon);
        if (weapon.quantity > 0) {
            const auto* weaponDef = Components::InventoryComponent::getItemDefinition(weapon.itemId);
            if (weaponDef) {
                // Apply weapon effects to damage calculation
                for (const auto& effect : weaponDef->effects) {
                    if (effect.targetStat == "attack_power") {
                        weaponDamage += effect.value;
                    } else if (effect.targetStat == "accuracy") {
                        weaponAccuracy += effect.value / 100.0f;
                    } else if (effect.targetStat == "critical_chance") {
                        weaponCritical += effect.value / 100.0f;
                    }
                }
            }
        }
    }
    
    // Calculate hit chance
    float hitChance = weaponAccuracy * (actorStats->getAccuracy() / 
        (actorStats->getAccuracy() + m_componentManager->getComponent<Components::StatsComponent>(Entity(action.target))->getEvasion()));
    action.hit = (randomFloat(0.0f, 1.0f) <= hitChance);
    
    if (action.hit) {
        // Calculate damage
        action.damage = actorStats->getAttackPower() + weaponDamage;
        action.damage *= actorCombat->getAttackPowerModifier();
        
        // Check for critical hit
        float critChance = weaponCritical + actorStats->getCriticalChance();
        action.critical = (randomFloat(0.0f, 1.0f) <= critChance);
        
        if (action.critical) {
            action.damage *= 2.0f;
        }
        
        // Apply damage
        applyDamage(action.target, action.damage);
    }
    
    // Mark actor as having acted
    actorCombat->setHasActed(true);
    auto* participant = getParticipant(action.actor);
    if (participant) {
        participant->hasActed = true;
    }
    
    // Trigger callbacks
    if (m_actionExecutedCallback) {
        m_actionExecutedCallback(action);
    }
    actorCombat->triggerActionCallback(action);
    
    return true;
}

bool CombatSystem::executeSkillAction(Components::CombatAction& action) {
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.actor));
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    
    if (!actorStats || !actorCombat) {
        return false;
    }
    
    // Get skill information
    const auto* skill = actorCombat->getSkill(action.skillId);
    if (!skill) {
        return false;
    }
    
    // Check MP cost
    if (skill->mpCost > actorStats->getCurrentMP()) {
        return false;
    }
    
    // Check if can use magic (for magic skills)
    if (skill->type == Components::CombatActionType::Magic && !actorCombat->canUseMagic()) {
        return false;
    }
    
    // Calculate hit chance
    float hitChance = calculateHitChance(action.actor, action.target, *skill);
    action.hit = (randomFloat(0.0f, 1.0f) <= hitChance);
    
    if (action.hit) {
        // Calculate damage/healing
        action.damage = calculateDamage(action);
        
        // Check for critical hit
        float critChance = calculateCriticalChance(action.actor, *skill);
        action.critical = (randomFloat(0.0f, 1.0f) <= critChance);
        
        if (action.critical) {
            action.damage *= 2.0f;
        }
        
        // Apply damage or healing
        if (action.damage > 0) {
            applyDamage(action.target, action.damage);
        } else if (action.damage < 0) {
            applyHealing(action.target, -action.damage);
        }
        
        // Apply status effects
        auto targetCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.target));
        if (targetCombat) {
            for (const auto& effect : skill->statusEffects) {
                targetCombat->addStatusEffect(effect);
                action.appliedEffects.push_back(effect);
            }
        }
    }
    
    // Consume MP
    if (skill->mpCost > 0) {
        actorStats->modifyMP(-skill->mpCost);
    }
    
    // Mark actor as having acted
    actorCombat->setHasActed(true);
    auto* participant = getParticipant(action.actor);
    if (participant) {
        participant->hasActed = true;
    }
    
    // Trigger callbacks
    if (m_actionExecutedCallback) {
        m_actionExecutedCallback(action);
    }
    actorCombat->triggerActionCallback(action);
    
    return true;
}

bool CombatSystem::executeItemAction(Components::CombatAction& action) {
    auto actorInventory = m_componentManager->getComponent<Components::InventoryComponent>(Entity(action.actor));
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    
    if (!actorInventory || !actorCombat) {
        return false;
    }
    
    // Check if actor has the item
    if (!actorInventory->hasItem(action.skillId, 1)) {
        return false;
    }
    
    // Get item definition
    const auto* itemDef = Components::InventoryComponent::getItemDefinition(action.skillId);
    if (!itemDef || itemDef->type != Components::ItemType::Consumable) {
        return false;
    }
    
    // Items always hit
    action.hit = true;
    action.critical = false;
    action.damage = 0.0f;
    
    // Apply item effects
    auto targetStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.target));
    auto targetCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.target));
    
    if (targetStats) {
        for (const auto& effect : itemDef->effects) {
            if (effect.targetStat == "hp") {
                if (effect.value > 0) {
                    float healing = effect.isPercentage ? 
                        (targetStats->getMaxHP() * effect.value / 100.0f) : effect.value;
                    applyHealing(action.target, healing);
                    action.damage = -healing; // Negative damage indicates healing
                } else {
                    float damage = effect.isPercentage ? 
                        (targetStats->getMaxHP() * -effect.value / 100.0f) : -effect.value;
                    applyDamage(action.target, damage);
                    action.damage = damage;
                }
            } else if (effect.targetStat == "mp") {
                if (effect.value > 0) {
                    float mpRestore = effect.isPercentage ? 
                        (targetStats->getMaxMP() * effect.value / 100.0f) : effect.value;
                    targetStats->modifyMP(mpRestore);
                }
            } else {
                // Apply stat modifiers
                Components::StatModifier modifier(
                    "item_" + action.skillId,
                    "item_use",
                    effect.isPercentage ? Components::ModifierType::Percentage : Components::ModifierType::Flat,
                    effect.value,
                    effect.duration
                );
                targetStats->addModifier(effect.targetStat, modifier);
            }
        }
    }
    
    // Apply status effects from item
    if (targetCombat) {
        // Create status effects based on item effects
        for (const auto& effect : itemDef->effects) {
            if (effect.effectId == "poison") {
                Components::StatusEffect statusEffect(
                    Components::StatusEffectType::Poison,
                    "Poison",
                    effect.duration,
                    effect.value,
                    false
                );
                targetCombat->addStatusEffect(statusEffect);
                action.appliedEffects.push_back(statusEffect);
            } else if (effect.effectId == "heal_over_time") {
                // Custom healing over time effect could be implemented
            }
        }
    }
    
    // Consume item
    actorInventory->removeItem(action.skillId, 1);
    
    // Mark actor as having acted
    actorCombat->setHasActed(true);
    auto* participant = getParticipant(action.actor);
    if (participant) {
        participant->hasActed = true;
    }
    
    // Trigger callbacks
    if (m_actionExecutedCallback) {
        m_actionExecutedCallback(action);
    }
    actorCombat->triggerActionCallback(action);
    
    return true;
}

bool CombatSystem::executeDefendAction(Components::CombatAction& action) {
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.actor));
    
    if (!actorCombat || !actorStats) {
        return false;
    }
    
    // Defending increases defense and reduces damage taken
    Components::StatusEffect defenseBoost(
        Components::StatusEffectType::Shield,
        "Defending",
        1.0f, // Lasts until next turn
        1.5f, // 50% defense boost
        true
    );
    
    actorCombat->addStatusEffect(defenseBoost);
    action.appliedEffects.push_back(defenseBoost);
    
    // Defending also restores a small amount of MP
    float mpRestore = actorStats->getMaxMP() * 0.1f;
    actorStats->modifyMP(mpRestore);
    
    action.hit = true;
    action.critical = false;
    action.damage = -mpRestore; // Show MP restoration as negative damage
    
    // Mark actor as having acted
    actorCombat->setHasActed(true);
    auto* participant = getParticipant(action.actor);
    if (participant) {
        participant->hasActed = true;
    }
    
    // Trigger callbacks
    if (m_actionExecutedCallback) {
        m_actionExecutedCallback(action);
    }
    actorCombat->triggerActionCallback(action);
    
    return true;
}

bool CombatSystem::executeEscapeAction(Components::CombatAction& action) {
    auto actorCombat = m_componentManager->getComponent<Components::CombatComponent>(Entity(action.actor));
    auto actorStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(action.actor));
    
    if (!actorCombat || !actorStats) {
        return false;
    }
    
    // Calculate escape chance based on speed/dexterity
    float escapeChance = 0.5f + (actorStats->getMovementSpeed() / 200.0f);
    escapeChance = std::clamp(escapeChance, 0.1f, 0.9f);
    
    action.hit = (randomFloat(0.0f, 1.0f) <= escapeChance);
    action.critical = false;
    action.damage = 0.0f;
    
    if (action.hit) {
        // Successful escape - end combat
        if (m_currentEncounter) {
            m_currentEncounter->state = Systems::CombatState::Escaped;
        }
        endCombat(false); // Escape counts as not winning
    }
    
    // Mark actor as having acted
    actorCombat->setHasActed(true);
    auto* participant = getParticipant(action.actor);
    if (participant) {
        participant->hasActed = true;
    }
    
    // Trigger callbacks
    if (m_actionExecutedCallback) {
        m_actionExecutedCallback(action);
    }
    actorCombat->triggerActionCallback(action);
    
    return true;
}

void CombatSystem::processAITurn(EntityId entity) {
    auto action = getAIAction(entity);
    queueAction(action);
    processActionQueue();
}

Components::CombatAction CombatSystem::getAIAction(EntityId entity) const {
    auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(entity));
    if (!combatComp) {
        return Components::CombatAction(entity, entity, Components::CombatActionType::Defend);
    }
    
    // Get available skills
    auto availableSkills = combatComp->getAvailableSkills();
    if (availableSkills.empty()) {
        return Components::CombatAction(entity, entity, Components::CombatActionType::Defend);
    }
    
    // Simple AI: choose random skill and target
    const auto* skill = availableSkills[const_cast<CombatSystem*>(this)->randomInt(0, availableSkills.size() - 1)];
    auto validTargets = getValidTargets(entity, *skill);
    
    if (validTargets.empty()) {
        return Components::CombatAction(entity, entity, Components::CombatActionType::Defend);
    }
    
    // Choose target with highest priority
    EntityId bestTarget = validTargets[0];
    float bestPriority = evaluateTargetPriority(entity, bestTarget);
    
    for (EntityId target : validTargets) {
        float priority = evaluateTargetPriority(entity, target);
        if (priority > bestPriority) {
            bestPriority = priority;
            bestTarget = target;
        }
    }
    
    return Components::CombatAction(entity, bestTarget, skill->type, skill->id);
}

float CombatSystem::evaluateTargetPriority(EntityId aiEntity, EntityId target) const {
    auto targetStats = m_componentManager->getComponent<Components::StatsComponent>(Entity(target));
    if (!targetStats) {
        return 0.0f;
    }
    
    // Simple priority: target with lowest HP percentage
    float hpPercentage = targetStats->getHPPercentage();
    return 1.0f - hpPercentage; // Lower HP = higher priority
}

void CombatSystem::initializeEncounter(CombatEncounter& encounter) {
    // Set combat state for all participants
    for (auto& participant : encounter.participants) {
        auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
        if (combatComp) {
            combatComp->setInCombat(true);
            combatComp->setMyTurn(false);
            combatComp->setHasActed(false);
        }
        
        updateParticipantStatus(participant);
    }
    
    // Calculate turn order
    calculateTurnOrder(encounter.participants);
    
    // Set initial state
    encounter.state = CombatState::Initializing;
    encounter.currentTurnIndex = -1;
    encounter.turnNumber = 1;
    
    // Start first turn
    nextTurn();
}

void CombatSystem::calculateTurnOrder(std::vector<CombatParticipant>& participants) {
    // Calculate turn order based on speed/dexterity
    for (auto& participant : participants) {
        auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(participant.entity));
        auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(participant.entity));
        
        if (statsComp) {
            float speed = statsComp->getMovementSpeed();
            if (combatComp) {
                speed *= combatComp->getSpeedModifier();
            }
            
            // Add some randomness
            speed *= randomFloat(0.9f, 1.1f);
            
            participant.turnOrder = static_cast<int>(speed * 100);
            
            if (combatComp) {
                combatComp->setTurnOrder(participant.turnOrder);
            }
        }
    }
    
    // Sort participants by turn order (highest first)
    std::sort(participants.begin(), participants.end(),
        [](const CombatParticipant& a, const CombatParticipant& b) {
            return a.turnOrder > b.turnOrder;
        });
}

void CombatSystem::updateCombatState(float deltaTime) {
    if (!m_currentEncounter) {
        return;
    }
    
    m_currentEncounter->turnTimer += deltaTime;
    
    // Check for turn time limit
    if (m_turnTimeLimit > 0.0f && m_currentEncounter->turnTimer >= m_turnTimeLimit) {
        if (m_autoEndTurn) {
            endTurn();
        }
    }
    
    // Process action queue
    processActionQueue();
    
    // Check for combat end conditions
    checkCombatEndConditions();
}

void CombatSystem::checkCombatEndConditions() {
    if (!m_currentEncounter) {
        return;
    }
    
    bool playersAlive = false;
    bool enemiesAlive = false;
    
    for (const auto& participant : m_currentEncounter->participants) {
        if (participant.isAlive) {
            if (participant.isPlayer) {
                playersAlive = true;
            } else {
                enemiesAlive = true;
            }
        }
    }
    
    if (!playersAlive) {
        m_currentEncounter->state = CombatState::Defeat;
        endCombat(false);
    } else if (!enemiesAlive) {
        m_currentEncounter->state = CombatState::Victory;
        endCombat(true);
    }
}

CombatParticipant* CombatSystem::getParticipant(EntityId entity) {
    if (!m_currentEncounter) {
        return nullptr;
    }
    
    auto it = std::find_if(m_currentEncounter->participants.begin(), 
                          m_currentEncounter->participants.end(),
                          [entity](const CombatParticipant& p) {
                              return p.entity == entity;
                          });
    
    return (it != m_currentEncounter->participants.end()) ? &(*it) : nullptr;
}

const CombatParticipant* CombatSystem::getParticipant(EntityId entity) const {
    if (!m_currentEncounter) {
        return nullptr;
    }
    
    auto it = std::find_if(m_currentEncounter->participants.begin(), 
                          m_currentEncounter->participants.end(),
                          [entity](const CombatParticipant& p) {
                              return p.entity == entity;
                          });
    
    return (it != m_currentEncounter->participants.end()) ? &(*it) : nullptr;
}

void CombatSystem::updateParticipantStatus(CombatParticipant& participant) {
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(participant.entity));
    if (statsComp) {
        participant.isAlive = statsComp->isAlive();
    }
}

void CombatSystem::applyStatusEffects(EntityId entity) {
    auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(entity));
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(entity));
    
    if (!combatComp || !statsComp) {
        return;
    }
    
    // Apply damage/healing from status effects
    for (const auto& effect : combatComp->getStatusEffects()) {
        switch (effect.type) {
            case Components::StatusEffectType::Poison:
                // Apply poison damage over time
                applyDamage(entity, effect.intensity * 8.0f);
                break;
            case Components::StatusEffectType::Burn:
                // Apply burn damage over time
                applyDamage(entity, effect.intensity * 10.0f);
                break;
            default:
                break;
        }
    }
}

void CombatSystem::applyEquipmentModifiers(EntityId entity) {
    auto inventoryComp = m_componentManager->getComponent<Components::InventoryComponent>(Entity(entity));
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(entity));
    
    if (!inventoryComp || !statsComp) {
        return;
    }
    
    // Remove existing equipment modifiers first
    removeEquipmentModifiers(entity);
    
    // Apply modifiers from all equipped items
    const auto& equipment = inventoryComp->getEquipment();
    for (const auto& pair : equipment.equippedItems) {
        const auto& item = pair.second;
        if (item.quantity > 0) {
            const auto* itemDef = Components::InventoryComponent::getItemDefinition(item.itemId);
            if (itemDef) {
                // Apply each effect as a stat modifier
                for (const auto& effect : itemDef->effects) {
                    Components::StatModifier modifier(
                        "equipment_" + item.itemId + "_" + effect.effectId,
                        "equipment",
                        effect.isPercentage ? Components::ModifierType::Percentage : Components::ModifierType::Flat,
                        effect.value,
                        -1.0f, // Permanent while equipped
                        false  // Not stackable
                    );
                    
                    statsComp->addModifier(effect.targetStat, modifier);
                }
            }
        }
    }
}

void CombatSystem::removeEquipmentModifiers(EntityId entity) {
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(entity));
    
    if (!statsComp) {
        return;
    }
    
    // Remove all modifiers from equipment source
    statsComp->removeModifiersFromSource("equipment");
}

float CombatSystem::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_randomGenerator);
}

int CombatSystem::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_randomGenerator);
}

} // namespace Systems
} // namespace RPGEngine