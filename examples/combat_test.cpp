#include <iostream>
#include <memory>
#include <vector>
#include "../src/systems/CombatSystem.h"
#include "../src/components/CombatComponent.h"
#include "../src/components/StatsComponent.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Systems;
using namespace RPGEngine::Components;

int main() {
    std::cout << "=== Combat System Test ===" << std::endl;
    
    // Create managers
    auto entityManager = std::make_shared<EntityManager>();
    auto componentManager = std::make_shared<ComponentManager>();
    
    // Initialize managers
    if (!entityManager->initialize()) {
        std::cout << "Failed to initialize entity manager!" << std::endl;
        return 1;
    }
    
    // Create combat system
    auto combatSystem = std::make_unique<CombatSystem>();
    combatSystem->setEntityManager(entityManager);
    combatSystem->setComponentManager(componentManager);
    
    // Initialize system
    if (!combatSystem->initialize()) {
        std::cout << "Failed to initialize combat system!" << std::endl;
        return 1;
    }
    
    std::cout << "Combat system initialized successfully." << std::endl;
    
    // Create test entities
    Entity player1Entity = entityManager->createEntity();
    Entity player2Entity = entityManager->createEntity();
    Entity enemy1Entity = entityManager->createEntity();
    Entity enemy2Entity = entityManager->createEntity();
    
    EntityId player1 = player1Entity.getID();
    EntityId player2 = player2Entity.getID();
    EntityId enemy1 = enemy1Entity.getID();
    EntityId enemy2 = enemy2Entity.getID();
    
    std::cout << "Created entities: Player1=" << player1 << ", Player2=" << player2 
              << ", Enemy1=" << enemy1 << ", Enemy2=" << enemy2 << std::endl;
    
    // Initialize component manager
    if (!componentManager->initialize()) {
        std::cout << "Failed to initialize component manager!" << std::endl;
        return 1;
    }
    
    // Add stats components
    auto player1Stats = componentManager->createComponent<StatsComponent>(player1Entity);
    auto player2Stats = componentManager->createComponent<StatsComponent>(player2Entity);
    auto enemy1Stats = componentManager->createComponent<StatsComponent>(enemy1Entity);
    auto enemy2Stats = componentManager->createComponent<StatsComponent>(enemy2Entity);
    
    // Configure player stats
    player1Stats->setBaseMaxHP(100.0f);
    player1Stats->setCurrentHP(100.0f);
    player1Stats->setBaseMaxMP(50.0f);
    player1Stats->setCurrentMP(50.0f);
    player1Stats->setBaseAttribute(AttributeType::Strength, 15);
    player1Stats->setBaseAttribute(AttributeType::Dexterity, 12);
    player1Stats->setBaseAttribute(AttributeType::Intelligence, 10);
    player1Stats->setBaseAttribute(AttributeType::Vitality, 14);
    
    player2Stats->setBaseMaxHP(80.0f);
    player2Stats->setCurrentHP(80.0f);
    player2Stats->setBaseMaxMP(70.0f);
    player2Stats->setCurrentMP(70.0f);
    player2Stats->setBaseAttribute(AttributeType::Strength, 10);
    player2Stats->setBaseAttribute(AttributeType::Dexterity, 16);
    player2Stats->setBaseAttribute(AttributeType::Intelligence, 18);
    player2Stats->setBaseAttribute(AttributeType::Vitality, 12);
    
    // Configure enemy stats
    enemy1Stats->setBaseMaxHP(60.0f);
    enemy1Stats->setCurrentHP(60.0f);
    enemy1Stats->setBaseMaxMP(30.0f);
    enemy1Stats->setCurrentMP(30.0f);
    enemy1Stats->setBaseAttribute(AttributeType::Strength, 12);
    enemy1Stats->setBaseAttribute(AttributeType::Dexterity, 10);
    enemy1Stats->setBaseAttribute(AttributeType::Intelligence, 8);
    enemy1Stats->setBaseAttribute(AttributeType::Vitality, 11);
    
    enemy2Stats->setBaseMaxHP(40.0f);
    enemy2Stats->setCurrentHP(40.0f);
    enemy2Stats->setBaseMaxMP(60.0f);
    enemy2Stats->setCurrentMP(60.0f);
    enemy2Stats->setBaseAttribute(AttributeType::Strength, 8);
    enemy2Stats->setBaseAttribute(AttributeType::Dexterity, 14);
    enemy2Stats->setBaseAttribute(AttributeType::Intelligence, 16);
    enemy2Stats->setBaseAttribute(AttributeType::Vitality, 9);
    
    std::cout << "Configured entity stats." << std::endl;
    
    // Add combat components
    auto player1Combat = componentManager->createComponent<CombatComponent>(player1Entity);
    auto player2Combat = componentManager->createComponent<CombatComponent>(player2Entity);
    auto enemy1Combat = componentManager->createComponent<CombatComponent>(enemy1Entity);
    auto enemy2Combat = componentManager->createComponent<CombatComponent>(enemy2Entity);
    
    // Add skills to player1 (warrior)
    CombatSkill swordSlash("sword_slash", "Sword Slash", CombatActionType::Attack);
    swordSlash.damage = 20.0f;
    swordSlash.accuracy = 0.9f;
    swordSlash.criticalChance = 0.1f;
    player1Combat->addSkill(swordSlash);
    
    CombatSkill powerStrike("power_strike", "Power Strike", CombatActionType::Skill);
    powerStrike.damage = 35.0f;
    powerStrike.mpCost = 10.0f;
    powerStrike.accuracy = 0.8f;
    powerStrike.criticalChance = 0.15f;
    player1Combat->addSkill(powerStrike);
    
    // Add skills to player2 (mage)
    CombatSkill fireball("fireball", "Fireball", CombatActionType::Magic);
    fireball.damage = 25.0f;
    fireball.mpCost = 15.0f;
    fireball.accuracy = 0.85f;
    fireball.criticalChance = 0.05f;
    StatusEffect burnEffect(StatusEffectType::Burn, "Burn", 3.0f, 1.0f, false);
    fireball.statusEffects.push_back(burnEffect);
    player2Combat->addSkill(fireball);
    
    CombatSkill heal("heal", "Heal", CombatActionType::Magic);
    heal.damage = -30.0f; // Negative damage = healing
    heal.mpCost = 20.0f;
    heal.accuracy = 1.0f;
    heal.targetsSelf = true;
    player2Combat->addSkill(heal);
    
    // Add skills to enemies
    CombatSkill clawAttack("claw_attack", "Claw Attack", CombatActionType::Attack);
    clawAttack.damage = 15.0f;
    clawAttack.accuracy = 0.85f;
    clawAttack.criticalChance = 0.08f;
    enemy1Combat->addSkill(clawAttack);
    
    CombatSkill darkBolt("dark_bolt", "Dark Bolt", CombatActionType::Magic);
    darkBolt.damage = 20.0f;
    darkBolt.mpCost = 12.0f;
    darkBolt.accuracy = 0.8f;
    enemy2Combat->addSkill(darkBolt);
    
    std::cout << "Added combat skills to entities." << std::endl;
    
    // Set up combat callbacks
    combatSystem->setCombatStartCallback([](const std::string& encounterId) {
        std::cout << "Combat started: " << encounterId << std::endl;
    });
    
    combatSystem->setCombatEndCallback([](bool victory, bool escaped) {
        std::cout << "Combat ended - Victory: " << (victory ? "Yes" : "No") 
                  << ", Escaped: " << (escaped ? "Yes" : "No") << std::endl;
    });
    
    combatSystem->setTurnStartCallback([](EntityId entity, int turnNumber) {
        std::cout << "Turn " << turnNumber << " started for entity " << entity << std::endl;
    });
    
    combatSystem->setActionExecutedCallback([](const CombatAction& action) {
        std::cout << "Action executed: Actor=" << action.actor << ", Target=" << action.target
                  << ", Damage=" << action.damage << ", Hit=" << (action.hit ? "Yes" : "No")
                  << ", Critical=" << (action.critical ? "Yes" : "No") << std::endl;
    });
    
    // Start combat
    std::vector<EntityId> players = {player1, player2};
    std::vector<EntityId> enemies = {enemy1, enemy2};
    
    if (!combatSystem->startCombat("test_encounter", players, enemies)) {
        std::cout << "Failed to start combat!" << std::endl;
        return 1;
    }
    
    std::cout << "Combat started successfully." << std::endl;
    
    // Test combat state
    std::cout << "Combat active: " << (combatSystem->isCombatActive() ? "Yes" : "No") << std::endl;
    std::cout << "Combat state: " << static_cast<int>(combatSystem->getCombatState()) << std::endl;
    
    // Get turn order
    auto turnOrder = combatSystem->getTurnOrder();
    std::cout << "Turn order:" << std::endl;
    for (size_t i = 0; i < turnOrder.size(); ++i) {
        std::cout << "  " << (i + 1) << ". Entity " << turnOrder[i]->entity 
                  << " (Player: " << (turnOrder[i]->isPlayer ? "Yes" : "No") 
                  << ", Turn Order: " << turnOrder[i]->turnOrder << ")" << std::endl;
    }
    
    // Simulate a few turns
    std::cout << "\n=== Simulating Combat Turns ===" << std::endl;
    
    int maxTurns = 10;
    int turnCount = 0;
    
    while (combatSystem->isCombatActive() && turnCount < maxTurns) {
        turnCount++;
        
        const auto* currentParticipant = combatSystem->getCurrentTurnParticipant();
        if (!currentParticipant) {
            std::cout << "No current participant, ending simulation." << std::endl;
            break;
        }
        
        std::cout << "\nTurn " << turnCount << " - Entity " << currentParticipant->entity 
                  << " (" << (currentParticipant->isPlayer ? "Player" : "Enemy") << ")" << std::endl;
        
        // Get available actions
        auto availableActions = combatSystem->getAvailableActions(currentParticipant->entity);
        std::cout << "Available actions: " << availableActions.size() << std::endl;
        
        if (!availableActions.empty()) {
            // Use first available action
            const auto* skill = availableActions[0];
            auto validTargets = combatSystem->getValidTargets(currentParticipant->entity, *skill);
            
            std::cout << "Using skill: " << skill->name << " (Valid targets: " << validTargets.size() << ")" << std::endl;
            
            if (!validTargets.empty()) {
                // Target first valid target
                EntityId target = validTargets[0];
                CombatAction action(currentParticipant->entity, target, skill->type, skill->id);
                
                combatSystem->queueAction(action);
                combatSystem->processActionQueue();
            }
        }
        
        // Update system
        combatSystem->update(1.0f);
        
        // Print current HP status
        std::cout << "HP Status:" << std::endl;
        std::cout << "  Player1: " << player1Stats->getCurrentHP() << "/" << player1Stats->getMaxHP() << std::endl;
        std::cout << "  Player2: " << player2Stats->getCurrentHP() << "/" << player2Stats->getMaxHP() << std::endl;
        std::cout << "  Enemy1: " << enemy1Stats->getCurrentHP() << "/" << enemy1Stats->getMaxHP() << std::endl;
        std::cout << "  Enemy2: " << enemy2Stats->getCurrentHP() << "/" << enemy2Stats->getMaxHP() << std::endl;
        
        // Small delay to make output readable
        if (combatSystem->isCombatActive()) {
            combatSystem->endTurn();
        }
    }
    
    if (turnCount >= maxTurns) {
        std::cout << "\nReached maximum turns, ending combat." << std::endl;
        combatSystem->endCombat(false);
    }
    
    std::cout << "\n=== Combat Test Complete ===" << std::endl;
    
    return 0;
}