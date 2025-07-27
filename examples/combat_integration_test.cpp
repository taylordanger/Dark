#include <iostream>
#include <memory>
#include "../src/systems/CombatSystem.h"
#include "../src/components/CombatComponent.h"
#include "../src/components/StatsComponent.h"
#include "../src/components/InventoryComponent.h"
#include "../src/components/ComponentManager.h"
#include "../src/entities/EntityManager.h"
#include "../src/ui/CombatUI.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/input/InputManager.h"

using namespace RPGEngine;

void setupTestItems() {
    // Register test items
    Components::ItemDefinition healthPotion("health_potion", "Health Potion", Components::ItemType::Consumable, 10);
    healthPotion.description = "Restores 50 HP";
    healthPotion.consumable = true;
    healthPotion.effects.emplace_back("heal", "hp", 50.0f);
    Components::InventoryComponent::registerItemDefinition(healthPotion);
    
    Components::ItemDefinition manaPotion("mana_potion", "Mana Potion", Components::ItemType::Consumable, 10);
    manaPotion.description = "Restores 30 MP";
    manaPotion.consumable = true;
    manaPotion.effects.emplace_back("restore_mp", "mp", 30.0f);
    Components::InventoryComponent::registerItemDefinition(manaPotion);
    
    Components::ItemDefinition ironSword("iron_sword", "Iron Sword", Components::ItemType::Equipment, 1);
    ironSword.description = "A sturdy iron sword";
    ironSword.equipmentSlot = Components::EquipmentSlot::Weapon;
    ironSword.effects.emplace_back("weapon_damage", "attack_power", 15.0f);
    ironSword.effects.emplace_back("weapon_accuracy", "accuracy", 5.0f);
    Components::InventoryComponent::registerItemDefinition(ironSword);
    
    Components::ItemDefinition leatherArmor("leather_armor", "Leather Armor", Components::ItemType::Equipment, 1);
    leatherArmor.description = "Basic leather armor";
    leatherArmor.equipmentSlot = Components::EquipmentSlot::Armor;
    leatherArmor.effects.emplace_back("armor_defense", "defense", 8.0f);
    leatherArmor.effects.emplace_back("armor_hp", "hp", 20.0f);
    Components::InventoryComponent::registerItemDefinition(leatherArmor);
}

void setupTestSkills(Components::CombatComponent* combatComp) {
    // Add test combat skills
    Components::CombatSkill fireball("fireball", "Fireball", Components::CombatActionType::Magic);
    fireball.description = "Launches a fireball at the enemy";
    fireball.mpCost = 15.0f;
    fireball.damage = 35.0f;
    fireball.accuracy = 0.9f;
    fireball.criticalChance = 0.1f;
    fireball.statusEffects.emplace_back(Components::StatusEffectType::Burn, "Burn", 3.0f, 1.0f, false);
    combatComp->addSkill(fireball);
    
    Components::CombatSkill heal("heal", "Heal", Components::CombatActionType::Magic);
    heal.description = "Restores HP to target";
    heal.mpCost = 12.0f;
    heal.damage = -40.0f; // Negative damage = healing
    heal.accuracy = 1.0f;
    heal.targetsSelf = true;
    combatComp->addSkill(heal);
    
    Components::CombatSkill powerStrike("power_strike", "Power Strike", Components::CombatActionType::Skill);
    powerStrike.description = "A powerful physical attack";
    powerStrike.mpCost = 8.0f;
    powerStrike.damage = 25.0f;
    powerStrike.accuracy = 0.85f;
    powerStrike.criticalChance = 0.25f;
    combatComp->addSkill(powerStrike);
    
    Components::CombatSkill poison("poison", "Poison", Components::CombatActionType::Magic);
    poison.description = "Inflicts poison on the target";
    poison.mpCost = 10.0f;
    poison.damage = 5.0f;
    poison.accuracy = 0.95f;
    poison.statusEffects.emplace_back(Components::StatusEffectType::Poison, "Poison", 5.0f, 2.0f, false);
    combatComp->addSkill(poison);
}

int main() {
    std::cout << "=== Combat Integration Test ===" << std::endl;
    
    // Setup test items
    setupTestItems();
    
    // Create managers
    auto entityManager = std::make_shared<EntityManager>();
    auto componentManager = std::make_shared<ComponentManager>();
    
    // Create combat system
    auto combatSystem = std::make_shared<Systems::CombatSystem>();
    combatSystem->setEntityManager(entityManager);
    combatSystem->setComponentManager(componentManager);
    
    // Create player entity
    Entity player = entityManager->createEntity();
    std::cout << "Created player entity: " << player.getID() << std::endl;
    
    // Add stats component to player
    auto playerStats = std::make_shared<Components::StatsComponent>(player.getID());
    playerStats->setBaseMaxHP(120.0f);
    playerStats->setBaseMaxMP(80.0f);
    playerStats->setBaseAttribute(Components::AttributeType::Strength, 15);
    playerStats->setBaseAttribute(Components::AttributeType::Dexterity, 12);
    playerStats->setBaseAttribute(Components::AttributeType::Intelligence, 14);
    playerStats->setBaseAttribute(Components::AttributeType::Vitality, 13);
    componentManager->addComponent(player, playerStats);
    
    // Add combat component to player
    auto playerCombat = std::make_shared<Components::CombatComponent>(player.getID());
    setupTestSkills(playerCombat.get());
    componentManager->addComponent(player, playerCombat);
    
    // Add inventory component to player
    auto playerInventory = std::make_shared<Components::InventoryComponent>(player.getID(), 20);
    
    // Add test items to inventory
    playerInventory->addItem("health_potion", 5);
    playerInventory->addItem("mana_potion", 3);
    playerInventory->addItem("iron_sword", 1);
    playerInventory->addItem("leather_armor", 1);
    
    // Equip items
    playerInventory->equipItem("iron_sword", Components::EquipmentSlot::Weapon);
    playerInventory->equipItem("leather_armor", Components::EquipmentSlot::Armor);
    
    componentManager->addComponent(player, playerInventory);
    
    // Create enemy entity
    Entity enemy = entityManager->createEntity();
    std::cout << "Created enemy entity: " << enemy.getID() << std::endl;
    
    // Add stats component to enemy
    auto enemyStats = std::make_shared<Components::StatsComponent>(enemy.getID());
    enemyStats->setBaseMaxHP(80.0f);
    enemyStats->setBaseMaxMP(40.0f);
    enemyStats->setBaseAttribute(Components::AttributeType::Strength, 12);
    enemyStats->setBaseAttribute(Components::AttributeType::Dexterity, 10);
    enemyStats->setBaseAttribute(Components::AttributeType::Intelligence, 8);
    enemyStats->setBaseAttribute(Components::AttributeType::Vitality, 11);
    componentManager->addComponent(enemy, enemyStats);
    
    // Add combat component to enemy
    auto enemyCombat = std::make_shared<Components::CombatComponent>(enemy.getID());
    // Add basic attack skill for enemy
    Components::CombatSkill claw("claw", "Claw Attack", Components::CombatActionType::Attack);
    claw.damage = 18.0f;
    claw.accuracy = 0.85f;
    claw.criticalChance = 0.05f;
    enemyCombat->addSkill(claw);
    componentManager->addComponent(enemy, enemyCombat);
    
    // Apply equipment modifiers to player
    combatSystem->applyEquipmentModifiers(player.getID());
    
    std::cout << "\n=== Initial Stats ===" << std::endl;
    std::cout << "Player HP: " << playerStats->getCurrentHP() << "/" << playerStats->getMaxHP() << std::endl;
    std::cout << "Player MP: " << playerStats->getCurrentMP() << "/" << playerStats->getMaxMP() << std::endl;
    std::cout << "Player Attack Power: " << playerStats->getAttackPower() << std::endl;
    std::cout << "Player Defense: " << playerStats->getDefense() << std::endl;
    std::cout << "Player equipped weapon: " << (playerInventory->isEquipped(Components::EquipmentSlot::Weapon) ? "Yes" : "No") << std::endl;
    std::cout << "Player equipped armor: " << (playerInventory->isEquipped(Components::EquipmentSlot::Armor) ? "Yes" : "No") << std::endl;
    
    std::cout << "\nEnemy HP: " << enemyStats->getCurrentHP() << "/" << enemyStats->getMaxHP() << std::endl;
    std::cout << "Enemy MP: " << enemyStats->getCurrentMP() << "/" << enemyStats->getMaxMP() << std::endl;
    std::cout << "Enemy Attack Power: " << enemyStats->getAttackPower() << std::endl;
    std::cout << "Enemy Defense: " << enemyStats->getDefense() << std::endl;
    
    // Start combat
    std::cout << "\n=== Starting Combat ===" << std::endl;
    std::vector<EntityId> playerEntities = {player.getID()};
    std::vector<EntityId> enemyEntities = {enemy.getID()};
    
    bool combatStarted = combatSystem->startCombat("test_encounter", playerEntities, enemyEntities);
    std::cout << "Combat started: " << (combatStarted ? "Yes" : "No") << std::endl;
    
    if (combatStarted) {
        // Test different combat actions
        std::cout << "\n=== Testing Combat Actions ===" << std::endl;
        
        // Test fireball attack
        Components::CombatAction fireballAction(player.getID(), enemy.getID(), 
                                               Components::CombatActionType::Magic, "fireball");
        std::cout << "\nExecuting fireball attack..." << std::endl;
        bool fireballSuccess = combatSystem->executeAction(fireballAction);
        std::cout << "Fireball success: " << (fireballSuccess ? "Yes" : "No") << std::endl;
        std::cout << "Enemy HP after fireball: " << enemyStats->getCurrentHP() << "/" << enemyStats->getMaxHP() << std::endl;
        std::cout << "Player MP after fireball: " << playerStats->getCurrentMP() << "/" << playerStats->getMaxMP() << std::endl;
        
        // Check for burn status effect
        if (enemyCombat->hasStatusEffect(Components::StatusEffectType::Burn)) {
            std::cout << "Enemy is burning!" << std::endl;
        }
        
        // Test healing potion
        Components::CombatAction potionAction(player.getID(), player.getID(), 
                                            Components::CombatActionType::Item, "health_potion");
        std::cout << "\nUsing health potion..." << std::endl;
        
        // Damage player first to test healing
        playerStats->modifyHP(-30.0f);
        std::cout << "Player HP before potion: " << playerStats->getCurrentHP() << "/" << playerStats->getMaxHP() << std::endl;
        
        bool potionSuccess = combatSystem->executeAction(potionAction);
        std::cout << "Potion success: " << (potionSuccess ? "Yes" : "No") << std::endl;
        std::cout << "Player HP after potion: " << playerStats->getCurrentHP() << "/" << playerStats->getMaxHP() << std::endl;
        std::cout << "Health potions remaining: " << playerInventory->getItemQuantity("health_potion") << std::endl;
        
        // Test defend action
        Components::CombatAction defendAction(player.getID(), player.getID(), 
                                            Components::CombatActionType::Defend);
        std::cout << "\nDefending..." << std::endl;
        bool defendSuccess = combatSystem->executeAction(defendAction);
        std::cout << "Defend success: " << (defendSuccess ? "Yes" : "No") << std::endl;
        
        // Check for defense buff
        if (playerCombat->hasStatusEffect(Components::StatusEffectType::Shield)) {
            std::cout << "Player has defense boost!" << std::endl;
        }
        
        // Test status effect application over time
        std::cout << "\n=== Testing Status Effects Over Time ===" << std::endl;
        for (int turn = 1; turn <= 3; ++turn) {
            std::cout << "\nTurn " << turn << ":" << std::endl;
            
            // Apply status effects
            combatSystem->applyStatusEffects(enemy.getID());
            
            std::cout << "Enemy HP: " << enemyStats->getCurrentHP() << "/" << enemyStats->getMaxHP() << std::endl;
            
            // Update status effect durations (simulate 1 second per turn)
            enemyCombat->updateStatusEffects(1.0f);
            
            // Check remaining status effects
            const auto& effects = enemyCombat->getStatusEffects();
            std::cout << "Active status effects: " << effects.size() << std::endl;
            for (const auto& effect : effects) {
                std::cout << "  - " << effect.name << " (duration: " << effect.duration << ")" << std::endl;
            }
        }
        
        // Test equipment stat integration
        std::cout << "\n=== Testing Equipment Integration ===" << std::endl;
        std::cout << "Player attack power with equipment: " << playerStats->getAttackPower() << std::endl;
        std::cout << "Player defense with equipment: " << playerStats->getDefense() << std::endl;
        
        // Unequip weapon and test
        playerInventory->unequipItem(Components::EquipmentSlot::Weapon);
        combatSystem->removeEquipmentModifiers(player.getID());
        combatSystem->applyEquipmentModifiers(player.getID());
        
        std::cout << "Player attack power without weapon: " << playerStats->getAttackPower() << std::endl;
        
        // Re-equip weapon
        playerInventory->equipItem("iron_sword", Components::EquipmentSlot::Weapon);
        combatSystem->applyEquipmentModifiers(player.getID());
        
        std::cout << "Player attack power with weapon re-equipped: " << playerStats->getAttackPower() << std::endl;
        
        // End combat
        combatSystem->endCombat(true);
        std::cout << "\nCombat ended." << std::endl;
    }
    
    std::cout << "\n=== Combat Integration Test Complete ===" << std::endl;
    return 0;
}