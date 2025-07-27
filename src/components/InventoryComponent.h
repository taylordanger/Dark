#pragma once

#include "Component.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

namespace RPGEngine {
namespace Components {

/**
 * Item type enumeration
 */
enum class ItemType {
    Consumable,     // Potions, food, etc.
    Equipment,      // Weapons, armor, accessories
    KeyItem,        // Quest items, keys, etc.
    Material,       // Crafting materials
    Misc            // Other items
};

/**
 * Equipment slot enumeration
 */
enum class EquipmentSlot {
    None,           // Not equippable
    Weapon,         // Main weapon
    Shield,         // Shield or off-hand
    Helmet,         // Head armor
    Armor,          // Body armor
    Gloves,         // Hand armor
    Boots,          // Foot armor
    Ring,           // Ring slot
    Necklace,       // Necklace slot
    Accessory       // General accessory
};

/**
 * Item rarity enumeration
 */
enum class ItemRarity {
    Common,         // White
    Uncommon,       // Green
    Rare,           // Blue
    Epic,           // Purple
    Legendary       // Orange
};

/**
 * Item effect structure
 */
struct ItemEffect {
    std::string effectId;       // Effect identifier
    std::string targetStat;     // Target stat (hp, mp, strength, etc.)
    float value;                // Effect value
    float duration;             // Duration in seconds (-1 for permanent)
    bool isPercentage;          // Whether value is percentage
    
    ItemEffect(const std::string& id, const std::string& stat, float val, 
               float dur = -1.0f, bool percentage = false)
        : effectId(id), targetStat(stat), value(val), duration(dur), isPercentage(percentage) {}
};

/**
 * Item definition structure
 */
struct ItemDefinition {
    std::string id;                         // Unique item ID
    std::string name;                       // Display name
    std::string description;                // Item description
    ItemType type;                          // Item type
    EquipmentSlot equipmentSlot;            // Equipment slot (if applicable)
    ItemRarity rarity;                      // Item rarity
    int maxStackSize;                       // Maximum stack size (1 for non-stackable)
    int value;                              // Base value/price
    std::vector<ItemEffect> effects;        // Item effects
    std::string iconPath;                   // Icon texture path
    bool consumable;                        // Whether item is consumed on use
    bool tradeable;                         // Whether item can be traded
    bool droppable;                         // Whether item can be dropped
    
    ItemDefinition() : id(""), name(""), type(ItemType::Misc), equipmentSlot(EquipmentSlot::None),
                      rarity(ItemRarity::Common), maxStackSize(1), value(0),
                      consumable(false), tradeable(true), droppable(true) {}
    
    ItemDefinition(const std::string& itemId, const std::string& itemName, 
                   ItemType itemType = ItemType::Misc, int stackSize = 1)
        : id(itemId), name(itemName), type(itemType), equipmentSlot(EquipmentSlot::None),
          rarity(ItemRarity::Common), maxStackSize(stackSize), value(0),
          consumable(false), tradeable(true), droppable(true) {}
};

/**
 * Item instance structure
 */
struct ItemInstance {
    std::string itemId;                     // Reference to item definition
    int quantity;                           // Current quantity
    int durability;                         // Current durability (-1 for no durability)
    int maxDurability;                      // Maximum durability
    std::unordered_map<std::string, float> customProperties; // Custom properties
    
    ItemInstance() : itemId(""), quantity(0), durability(-1), maxDurability(-1) {}
    
    ItemInstance(const std::string& id, int qty = 1, int dur = -1)
        : itemId(id), quantity(qty), durability(dur), maxDurability(dur) {}
    
    /**
     * Check if item is broken
     * @return true if durability is 0
     */
    bool isBroken() const { return durability == 0; }
    
    /**
     * Check if item has durability
     * @return true if item uses durability system
     */
    bool hasDurability() const { return maxDurability > 0; }
};

/**
 * Inventory slot structure
 */
struct InventorySlot {
    ItemInstance item;
    bool isEmpty;
    
    InventorySlot() : item("", 0), isEmpty(true) {}
    InventorySlot(const ItemInstance& itemInstance) : item(itemInstance), isEmpty(false) {}
    
    /**
     * Clear the slot
     */
    void clear() {
        item = ItemInstance("", 0);
        isEmpty = true;
    }
    
    /**
     * Set item in slot
     * @param itemInstance Item to set
     */
    void setItem(const ItemInstance& itemInstance) {
        item = itemInstance;
        isEmpty = false;
    }
};

/**
 * Equipment set structure
 */
struct EquipmentSet {
    std::unordered_map<EquipmentSlot, ItemInstance> equippedItems;
    
    /**
     * Check if slot is equipped
     * @param slot Equipment slot
     * @return true if slot has item equipped
     */
    bool isSlotEquipped(EquipmentSlot slot) const {
        auto it = equippedItems.find(slot);
        return it != equippedItems.end() && it->second.quantity > 0;
    }
    
    /**
     * Get equipped item in slot
     * @param slot Equipment slot
     * @return Item instance, or empty item if not equipped
     */
    const ItemInstance& getEquippedItem(EquipmentSlot slot) const {
        static ItemInstance emptyItem("", 0);
        auto it = equippedItems.find(slot);
        return (it != equippedItems.end()) ? it->second : emptyItem;
    }
    
    /**
     * Equip item in slot
     * @param slot Equipment slot
     * @param item Item to equip
     */
    void equipItem(EquipmentSlot slot, const ItemInstance& item) {
        equippedItems[slot] = item;
    }
    
    /**
     * Unequip item from slot
     * @param slot Equipment slot
     * @return Unequipped item
     */
    ItemInstance unequipItem(EquipmentSlot slot) {
        auto it = equippedItems.find(slot);
        if (it != equippedItems.end()) {
            ItemInstance item = it->second;
            equippedItems.erase(it);
            return item;
        }
        return ItemInstance("", 0);
    }
};

/**
 * Inventory component
 * Manages item storage, equipment, and item usage
 */
class InventoryComponent : public Component<InventoryComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     * @param capacity Initial inventory capacity
     */
    explicit InventoryComponent(EntityId entityId, int capacity = 30);
    
    /**
     * Destructor
     */
    ~InventoryComponent();
    
    // Item management
    
    /**
     * Add item to inventory
     * @param itemId Item ID
     * @param quantity Quantity to add
     * @return Actual quantity added
     */
    int addItem(const std::string& itemId, int quantity = 1);
    
    /**
     * Add item instance to inventory
     * @param item Item instance to add
     * @return Actual quantity added
     */
    int addItem(const ItemInstance& item);
    
    /**
     * Remove item from inventory
     * @param itemId Item ID
     * @param quantity Quantity to remove
     * @return Actual quantity removed
     */
    int removeItem(const std::string& itemId, int quantity = 1);
    
    /**
     * Remove item from specific slot
     * @param slotIndex Slot index
     * @param quantity Quantity to remove
     * @return Actual quantity removed
     */
    int removeItemFromSlot(int slotIndex, int quantity = 1);
    
    /**
     * Use item
     * @param itemId Item ID
     * @param quantity Quantity to use
     * @return true if item was used successfully
     */
    bool useItem(const std::string& itemId, int quantity = 1);
    
    /**
     * Use item from specific slot
     * @param slotIndex Slot index
     * @param quantity Quantity to use
     * @return true if item was used successfully
     */
    bool useItemFromSlot(int slotIndex, int quantity = 1);
    
    /**
     * Check if inventory has item
     * @param itemId Item ID
     * @param quantity Required quantity
     * @return true if inventory has enough of the item
     */
    bool hasItem(const std::string& itemId, int quantity = 1) const;
    
    /**
     * Get item quantity
     * @param itemId Item ID
     * @return Total quantity of item in inventory
     */
    int getItemQuantity(const std::string& itemId) const;
    
    /**
     * Find item slot
     * @param itemId Item ID
     * @return Slot index, or -1 if not found
     */
    int findItemSlot(const std::string& itemId) const;
    
    /**
     * Find empty slot
     * @return Empty slot index, or -1 if inventory is full
     */
    int findEmptySlot() const;
    
    // Inventory management
    
    /**
     * Get inventory capacity
     * @return Maximum number of slots
     */
    int getCapacity() const { return static_cast<int>(m_slots.size()); }
    
    /**
     * Set inventory capacity
     * @param capacity New capacity
     */
    void setCapacity(int capacity);
    
    /**
     * Get used slots count
     * @return Number of non-empty slots
     */
    int getUsedSlots() const;
    
    /**
     * Get free slots count
     * @return Number of empty slots
     */
    int getFreeSlots() const { return getCapacity() - getUsedSlots(); }
    
    /**
     * Check if inventory is full
     * @return true if no empty slots
     */
    bool isFull() const { return getFreeSlots() == 0; }
    
    /**
     * Check if inventory is empty
     * @return true if all slots are empty
     */
    bool isEmpty() const { return getUsedSlots() == 0; }
    
    /**
     * Get slot
     * @param index Slot index
     * @return Inventory slot reference
     */
    const InventorySlot& getSlot(int index) const;
    
    /**
     * Get all slots
     * @return Vector of inventory slots
     */
    const std::vector<InventorySlot>& getSlots() const { return m_slots; }
    
    /**
     * Clear inventory
     */
    void clear();
    
    /**
     * Sort inventory
     * @param sortByType Whether to sort by item type
     */
    void sortInventory(bool sortByType = true);
    
    // Equipment management
    
    /**
     * Equip item
     * @param itemId Item ID
     * @param slot Equipment slot (auto-detect if None)
     * @return true if item was equipped successfully
     */
    bool equipItem(const std::string& itemId, EquipmentSlot slot = EquipmentSlot::None);
    
    /**
     * Equip item from slot
     * @param slotIndex Inventory slot index
     * @param equipSlot Equipment slot (auto-detect if None)
     * @return true if item was equipped successfully
     */
    bool equipItemFromSlot(int slotIndex, EquipmentSlot equipSlot = EquipmentSlot::None);
    
    /**
     * Unequip item
     * @param slot Equipment slot
     * @return true if item was unequipped successfully
     */
    bool unequipItem(EquipmentSlot slot);
    
    /**
     * Check if equipment slot is equipped
     * @param slot Equipment slot
     * @return true if slot has item equipped
     */
    bool isEquipped(EquipmentSlot slot) const { return m_equipment.isSlotEquipped(slot); }
    
    /**
     * Get equipped item
     * @param slot Equipment slot
     * @return Equipped item instance
     */
    const ItemInstance& getEquippedItem(EquipmentSlot slot) const {
        return m_equipment.getEquippedItem(slot);
    }
    
    /**
     * Get all equipped items
     * @return Equipment set
     */
    const EquipmentSet& getEquipment() const { return m_equipment; }
    
    // Item definitions
    
    /**
     * Register item definition
     * @param definition Item definition
     */
    static void registerItemDefinition(const ItemDefinition& definition);
    
    /**
     * Get item definition
     * @param itemId Item ID
     * @return Item definition, or nullptr if not found
     */
    static const ItemDefinition* getItemDefinition(const std::string& itemId);
    
    /**
     * Check if item definition exists
     * @param itemId Item ID
     * @return true if definition exists
     */
    static bool hasItemDefinition(const std::string& itemId);
    
    // Callbacks
    
    /**
     * Set item added callback
     * @param callback Function called when item is added
     */
    void setItemAddedCallback(std::function<void(const std::string&, int)> callback) {
        m_itemAddedCallback = callback;
    }
    
    /**
     * Set item removed callback
     * @param callback Function called when item is removed
     */
    void setItemRemovedCallback(std::function<void(const std::string&, int)> callback) {
        m_itemRemovedCallback = callback;
    }
    
    /**
     * Set item used callback
     * @param callback Function called when item is used
     */
    void setItemUsedCallback(std::function<void(const std::string&, int)> callback) {
        m_itemUsedCallback = callback;
    }
    
    /**
     * Set item equipped callback
     * @param callback Function called when item is equipped
     */
    void setItemEquippedCallback(std::function<void(const std::string&, EquipmentSlot)> callback) {
        m_itemEquippedCallback = callback;
    }
    
    /**
     * Set item unequipped callback
     * @param callback Function called when item is unequipped
     */
    void setItemUnequippedCallback(std::function<void(const std::string&, EquipmentSlot)> callback) {
        m_itemUnequippedCallback = callback;
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
     * Can items stack together
     * @param item1 First item
     * @param item2 Second item
     * @return true if items can stack
     */
    bool canStack(const ItemInstance& item1, const ItemInstance& item2) const;
    
    /**
     * Get maximum stack size for item
     * @param itemId Item ID
     * @return Maximum stack size
     */
    int getMaxStackSize(const std::string& itemId) const;
    
    /**
     * Apply item effects
     * @param itemId Item ID
     * @param quantity Quantity used
     */
    void applyItemEffects(const std::string& itemId, int quantity);
    
    /**
     * Trigger item added event
     * @param itemId Item ID
     * @param quantity Quantity added
     */
    void triggerItemAdded(const std::string& itemId, int quantity);
    
    /**
     * Trigger item removed event
     * @param itemId Item ID
     * @param quantity Quantity removed
     */
    void triggerItemRemoved(const std::string& itemId, int quantity);
    
    /**
     * Trigger item used event
     * @param itemId Item ID
     * @param quantity Quantity used
     */
    void triggerItemUsed(const std::string& itemId, int quantity);
    
    /**
     * Trigger item equipped event
     * @param itemId Item ID
     * @param slot Equipment slot
     */
    void triggerItemEquipped(const std::string& itemId, EquipmentSlot slot);
    
    /**
     * Trigger item unequipped event
     * @param itemId Item ID
     * @param slot Equipment slot
     */
    void triggerItemUnequipped(const std::string& itemId, EquipmentSlot slot);
    
    // Inventory data
    std::vector<InventorySlot> m_slots;
    EquipmentSet m_equipment;
    
    // Item definitions (static registry)
    static std::unordered_map<std::string, ItemDefinition> s_itemDefinitions;
    
    // Callbacks
    std::function<void(const std::string&, int)> m_itemAddedCallback;
    std::function<void(const std::string&, int)> m_itemRemovedCallback;
    std::function<void(const std::string&, int)> m_itemUsedCallback;
    std::function<void(const std::string&, EquipmentSlot)> m_itemEquippedCallback;
    std::function<void(const std::string&, EquipmentSlot)> m_itemUnequippedCallback;
};

} // namespace Components
} // namespace RPGEngine