#include "../src/components/InventoryComponent.h"
#include "../src/entities/EntityManager.h"
#include <iostream>
#include <cassert>

using namespace RPGEngine;
using namespace RPGEngine::Components;

void setupTestItems() {
    std::cout << "\n=== Setting up test items ===" << std::endl;
    
    // Health Potion - Consumable, stackable
    ItemDefinition healthPotion("health_potion", "Health Potion", ItemType::Consumable, 10);
    healthPotion.description = "Restores 50 HP";
    healthPotion.value = 25;
    healthPotion.consumable = true;
    healthPotion.effects.push_back(ItemEffect("heal", "hp", 50.0f));
    InventoryComponent::registerItemDefinition(healthPotion);
    
    // Mana Potion - Consumable, stackable
    ItemDefinition manaPotion("mana_potion", "Mana Potion", ItemType::Consumable, 10);
    manaPotion.description = "Restores 30 MP";
    manaPotion.value = 20;
    manaPotion.consumable = true;
    manaPotion.effects.push_back(ItemEffect("restore_mana", "mp", 30.0f));
    InventoryComponent::registerItemDefinition(manaPotion);
    
    // Iron Sword - Equipment, non-stackable
    ItemDefinition ironSword("iron_sword", "Iron Sword", ItemType::Equipment, 1);
    ironSword.description = "A sturdy iron sword";
    ironSword.equipmentSlot = EquipmentSlot::Weapon;
    ironSword.rarity = ItemRarity::Common;
    ironSword.value = 100;
    ironSword.effects.push_back(ItemEffect("weapon_damage", "attack_power", 15.0f, -1.0f));
    InventoryComponent::registerItemDefinition(ironSword);
    
    // Steel Armor - Equipment, non-stackable
    ItemDefinition steelArmor("steel_armor", "Steel Armor", ItemType::Equipment, 1);
    steelArmor.description = "Protective steel armor";
    steelArmor.equipmentSlot = EquipmentSlot::Armor;
    steelArmor.rarity = ItemRarity::Uncommon;
    steelArmor.value = 200;
    steelArmor.effects.push_back(ItemEffect("armor_defense", "defense", 10.0f, -1.0f));
    InventoryComponent::registerItemDefinition(steelArmor);
    
    // Magic Ring - Equipment, non-stackable
    ItemDefinition magicRing("magic_ring", "Ring of Power", ItemType::Equipment, 1);
    magicRing.description = "Increases magical power";
    magicRing.equipmentSlot = EquipmentSlot::Ring;
    magicRing.rarity = ItemRarity::Rare;
    magicRing.value = 500;
    magicRing.effects.push_back(ItemEffect("magic_boost", "magic_power", 8.0f, -1.0f));
    InventoryComponent::registerItemDefinition(magicRing);
    
    // Iron Ore - Material, stackable
    ItemDefinition ironOre("iron_ore", "Iron Ore", ItemType::Material, 50);
    ironOre.description = "Raw iron ore for crafting";
    ironOre.value = 5;
    InventoryComponent::registerItemDefinition(ironOre);
    
    // Quest Key - Key item, non-stackable
    ItemDefinition questKey("dungeon_key", "Dungeon Key", ItemType::KeyItem, 1);
    questKey.description = "Opens the ancient dungeon";
    questKey.value = 0;
    questKey.tradeable = false;
    questKey.droppable = false;
    InventoryComponent::registerItemDefinition(questKey);
    
    std::cout << "Test items registered successfully!" << std::endl;
}

void testBasicInventoryOperations() {
    std::cout << "\n=== Testing Basic Inventory Operations ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory(player.getID(), 10);
    
    // Test initial state
    assert(inventory.getCapacity() == 10);
    assert(inventory.isEmpty());
    assert(inventory.getFreeSlots() == 10);
    assert(inventory.getUsedSlots() == 0);
    
    std::cout << "Initial capacity: " << inventory.getCapacity() << std::endl;
    std::cout << "Initial free slots: " << inventory.getFreeSlots() << std::endl;
    
    // Test adding items
    int added = inventory.addItem("health_potion", 5);
    assert(added == 5);
    assert(inventory.hasItem("health_potion", 5));
    assert(inventory.getItemQuantity("health_potion") == 5);
    assert(inventory.getUsedSlots() == 1);
    
    std::cout << "Added 5 health potions" << std::endl;
    std::cout << "Health potion quantity: " << inventory.getItemQuantity("health_potion") << std::endl;
    std::cout << "Used slots: " << inventory.getUsedSlots() << std::endl;
    
    // Test adding more of the same item (stacking)
    added = inventory.addItem("health_potion", 3);
    assert(added == 3);
    assert(inventory.getItemQuantity("health_potion") == 8);
    assert(inventory.getUsedSlots() == 1); // Still one slot due to stacking
    
    std::cout << "Added 3 more health potions (stacking)" << std::endl;
    std::cout << "Total health potions: " << inventory.getItemQuantity("health_potion") << std::endl;
    
    // Test adding different item
    added = inventory.addItem("mana_potion", 2);
    assert(added == 2);
    assert(inventory.getUsedSlots() == 2);
    
    std::cout << "Added 2 mana potions" << std::endl;
    std::cout << "Used slots: " << inventory.getUsedSlots() << std::endl;
    
    // Test removing items
    int removed = inventory.removeItem("health_potion", 3);
    assert(removed == 3);
    assert(inventory.getItemQuantity("health_potion") == 5);
    
    std::cout << "Removed 3 health potions" << std::endl;
    std::cout << "Remaining health potions: " << inventory.getItemQuantity("health_potion") << std::endl;
    
    // Test finding items
    int slot = inventory.findItemSlot("mana_potion");
    assert(slot != -1);
    
    std::cout << "Mana potion found in slot: " << slot << std::endl;
    
    std::cout << "Basic inventory operations test passed!" << std::endl;
}

void testStackingLimits() {
    std::cout << "\n=== Testing Stacking Limits ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory(player.getID(), 5);
    
    // Test adding items up to stack limit
    int added = inventory.addItem("health_potion", 10); // Max stack size is 10
    assert(added == 10);
    assert(inventory.getUsedSlots() == 1);
    
    std::cout << "Added 10 health potions (full stack)" << std::endl;
    
    // Test adding more than stack limit
    added = inventory.addItem("health_potion", 5);
    assert(added == 5);
    assert(inventory.getUsedSlots() == 2); // Should create new stack
    
    std::cout << "Added 5 more health potions (new stack)" << std::endl;
    std::cout << "Used slots: " << inventory.getUsedSlots() << std::endl;
    
    // Test adding non-stackable items
    added = inventory.addItem("iron_sword", 2);
    assert(added == 2);
    assert(inventory.getUsedSlots() == 4); // Each sword takes one slot
    
    std::cout << "Added 2 iron swords (non-stackable)" << std::endl;
    std::cout << "Used slots: " << inventory.getUsedSlots() << std::endl;
    
    // Test inventory full scenario
    added = inventory.addItem("steel_armor", 2);
    assert(added == 1); // Only one slot left
    assert(inventory.isFull());
    
    std::cout << "Tried to add 2 steel armor, only " << added << " added (inventory full)" << std::endl;
    std::cout << "Inventory is full: " << (inventory.isFull() ? "true" : "false") << std::endl;
    
    std::cout << "Stacking limits test passed!" << std::endl;
}

void testEquipmentSystem() {
    std::cout << "\n=== Testing Equipment System ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory(player.getID(), 10);
    
    // Add equipment items to inventory
    inventory.addItem("iron_sword", 1);
    inventory.addItem("steel_armor", 1);
    inventory.addItem("magic_ring", 1);
    
    std::cout << "Added equipment items to inventory" << std::endl;
    
    // Test equipping items
    bool equipped = inventory.equipItem("iron_sword");
    assert(equipped);
    assert(inventory.isEquipped(EquipmentSlot::Weapon));
    assert(inventory.getItemQuantity("iron_sword") == 0); // Removed from inventory
    
    std::cout << "Equipped iron sword" << std::endl;
    std::cout << "Weapon slot equipped: " << (inventory.isEquipped(EquipmentSlot::Weapon) ? "true" : "false") << std::endl;
    
    equipped = inventory.equipItem("steel_armor");
    assert(equipped);
    assert(inventory.isEquipped(EquipmentSlot::Armor));
    
    std::cout << "Equipped steel armor" << std::endl;
    
    equipped = inventory.equipItem("magic_ring");
    assert(equipped);
    assert(inventory.isEquipped(EquipmentSlot::Ring));
    
    std::cout << "Equipped magic ring" << std::endl;
    
    // Test getting equipped items
    const ItemInstance& weapon = inventory.getEquippedItem(EquipmentSlot::Weapon);
    assert(weapon.itemId == "iron_sword");
    
    std::cout << "Equipped weapon: " << weapon.itemId << std::endl;
    
    // Test unequipping items
    bool unequipped = inventory.unequipItem(EquipmentSlot::Weapon);
    assert(unequipped);
    assert(!inventory.isEquipped(EquipmentSlot::Weapon));
    assert(inventory.hasItem("iron_sword", 1)); // Back in inventory
    
    std::cout << "Unequipped weapon" << std::endl;
    std::cout << "Iron sword back in inventory: " << (inventory.hasItem("iron_sword", 1) ? "true" : "false") << std::endl;
    
    // Test equipping from specific slot
    int swordSlot = inventory.findItemSlot("iron_sword");
    equipped = inventory.equipItemFromSlot(swordSlot);
    assert(equipped);
    assert(inventory.isEquipped(EquipmentSlot::Weapon));
    
    std::cout << "Re-equipped iron sword from slot " << swordSlot << std::endl;
    
    std::cout << "Equipment system test passed!" << std::endl;
}

void testItemUsage() {
    std::cout << "\n=== Testing Item Usage ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory(player.getID(), 10);
    
    // Set up callbacks
    inventory.setItemUsedCallback([](const std::string& itemId, int quantity) {
        std::cout << "Used " << quantity << "x " << itemId << std::endl;
    });
    
    // Add consumable items
    inventory.addItem("health_potion", 5);
    inventory.addItem("mana_potion", 3);
    
    std::cout << "Added consumable items" << std::endl;
    
    // Test using consumable items
    bool used = inventory.useItem("health_potion", 2);
    assert(used);
    assert(inventory.getItemQuantity("health_potion") == 3); // Consumed
    
    std::cout << "Used 2 health potions, remaining: " << inventory.getItemQuantity("health_potion") << std::endl;
    
    // Test using from specific slot
    int potionSlot = inventory.findItemSlot("mana_potion");
    used = inventory.useItemFromSlot(potionSlot, 1);
    assert(used);
    assert(inventory.getItemQuantity("mana_potion") == 2);
    
    std::cout << "Used 1 mana potion from slot, remaining: " << inventory.getItemQuantity("mana_potion") << std::endl;
    
    // Test using non-existent item
    used = inventory.useItem("nonexistent_item", 1);
    assert(!used);
    
    std::cout << "Tried to use non-existent item: " << (used ? "success" : "failed") << std::endl;
    
    // Test using more than available
    used = inventory.useItem("health_potion", 10);
    assert(!used);
    
    std::cout << "Tried to use more than available: " << (used ? "success" : "failed") << std::endl;
    
    std::cout << "Item usage test passed!" << std::endl;
}

void testInventoryManagement() {
    std::cout << "\n=== Testing Inventory Management ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory(player.getID(), 5);
    
    // Add various items
    inventory.addItem("health_potion", 3);
    inventory.addItem("iron_sword", 1);
    inventory.addItem("iron_ore", 10);
    inventory.addItem("mana_potion", 2);
    
    std::cout << "Added various items" << std::endl;
    std::cout << "Used slots before sort: " << inventory.getUsedSlots() << std::endl;
    
    // Test sorting
    inventory.sortInventory(true); // Sort by type
    
    std::cout << "Sorted inventory by type" << std::endl;
    
    // Display inventory contents
    for (int i = 0; i < inventory.getCapacity(); ++i) {
        const auto& slot = inventory.getSlot(i);
        if (!slot.isEmpty) {
            const ItemDefinition* def = InventoryComponent::getItemDefinition(slot.item.itemId);
            std::cout << "Slot " << i << ": " << (def ? def->name : slot.item.itemId) 
                      << " x" << slot.item.quantity << std::endl;
        }
    }
    
    // Test capacity changes
    std::cout << "Changing capacity from 5 to 3..." << std::endl;
    inventory.setCapacity(3);
    assert(inventory.getCapacity() == 3);
    
    std::cout << "New capacity: " << inventory.getCapacity() << std::endl;
    std::cout << "Used slots after resize: " << inventory.getUsedSlots() << std::endl;
    
    // Test expanding capacity
    std::cout << "Expanding capacity to 8..." << std::endl;
    inventory.setCapacity(8);
    assert(inventory.getCapacity() == 8);
    
    std::cout << "New capacity: " << inventory.getCapacity() << std::endl;
    std::cout << "Free slots: " << inventory.getFreeSlots() << std::endl;
    
    // Test clearing inventory
    inventory.clear();
    assert(inventory.isEmpty());
    assert(inventory.getUsedSlots() == 0);
    
    std::cout << "Cleared inventory" << std::endl;
    std::cout << "Is empty: " << (inventory.isEmpty() ? "true" : "false") << std::endl;
    
    std::cout << "Inventory management test passed!" << std::endl;
}

void testCallbacks() {
    std::cout << "\n=== Testing Callbacks ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory(player.getID(), 10);
    
    // Set up callbacks
    inventory.setItemAddedCallback([](const std::string& itemId, int quantity) {
        std::cout << "CALLBACK: Added " << quantity << "x " << itemId << std::endl;
    });
    
    inventory.setItemRemovedCallback([](const std::string& itemId, int quantity) {
        std::cout << "CALLBACK: Removed " << quantity << "x " << itemId << std::endl;
    });
    
    inventory.setItemEquippedCallback([](const std::string& itemId, EquipmentSlot slot) {
        std::cout << "CALLBACK: Equipped " << itemId << " in slot " << static_cast<int>(slot) << std::endl;
    });
    
    inventory.setItemUnequippedCallback([](const std::string& itemId, EquipmentSlot slot) {
        std::cout << "CALLBACK: Unequipped " << itemId << " from slot " << static_cast<int>(slot) << std::endl;
    });
    
    // Trigger callbacks
    inventory.addItem("health_potion", 5);
    inventory.addItem("iron_sword", 1);
    
    inventory.equipItem("iron_sword");
    inventory.unequipItem(EquipmentSlot::Weapon);
    
    inventory.removeItem("health_potion", 2);
    
    std::cout << "Callbacks test passed!" << std::endl;
}

void testSerialization() {
    std::cout << "\n=== Testing Serialization ===" << std::endl;
    
    EntityManager entityManager;
    Entity player = entityManager.createEntity("TestPlayer");
    
    InventoryComponent inventory1(player.getID(), 10);
    
    // Set up inventory
    inventory1.addItem("health_potion", 5);
    inventory1.addItem("iron_sword", 1);
    inventory1.addItem("steel_armor", 1);
    inventory1.equipItem("iron_sword");
    inventory1.equipItem("steel_armor");
    
    std::cout << "Set up inventory with items and equipment" << std::endl;
    
    // Serialize
    std::string serialized = inventory1.serialize();
    std::cout << "Serialized data length: " << serialized.length() << std::endl;
    
    // Create new inventory and deserialize
    InventoryComponent inventory2(player.getID(), 5);
    bool success = inventory2.deserialize(serialized);
    assert(success);
    
    // Verify deserialized data
    assert(inventory2.getCapacity() == 10);
    assert(inventory2.hasItem("health_potion", 5));
    assert(inventory2.isEquipped(EquipmentSlot::Weapon));
    assert(inventory2.isEquipped(EquipmentSlot::Armor));
    
    std::cout << "Deserialized inventory capacity: " << inventory2.getCapacity() << std::endl;
    std::cout << "Health potions: " << inventory2.getItemQuantity("health_potion") << std::endl;
    std::cout << "Weapon equipped: " << (inventory2.isEquipped(EquipmentSlot::Weapon) ? "true" : "false") << std::endl;
    std::cout << "Armor equipped: " << (inventory2.isEquipped(EquipmentSlot::Armor) ? "true" : "false") << std::endl;
    
    const ItemInstance& weapon = inventory2.getEquippedItem(EquipmentSlot::Weapon);
    std::cout << "Equipped weapon: " << weapon.itemId << std::endl;
    
    std::cout << "Serialization test passed!" << std::endl;
}

int main() {
    std::cout << "Starting Inventory System Tests..." << std::endl;
    
    try {
        setupTestItems();
        testBasicInventoryOperations();
        testStackingLimits();
        testEquipmentSystem();
        testItemUsage();
        testInventoryManagement();
        testCallbacks();
        testSerialization();
        
        std::cout << "\n=== All Inventory Tests Passed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}