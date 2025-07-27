#include "InventoryComponent.h"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace RPGEngine {
namespace Components {

// Static member initialization
std::unordered_map<std::string, ItemDefinition> InventoryComponent::s_itemDefinitions;

InventoryComponent::InventoryComponent(EntityId entityId, int capacity)
    : Component<InventoryComponent>(entityId)
    , m_slots(capacity)
{
    // Initialize all slots as empty
    for (auto& slot : m_slots) {
        slot.clear();
    }
}

InventoryComponent::~InventoryComponent() {
    // Cleanup handled automatically
}

int InventoryComponent::addItem(const std::string& itemId, int quantity) {
    if (quantity <= 0 || !hasItemDefinition(itemId)) {
        return 0;
    }
    
    const ItemDefinition* definition = getItemDefinition(itemId);
    if (!definition) {
        return 0;
    }
    
    int remainingQuantity = quantity;
    int maxStackSize = definition->maxStackSize;
    
    // First, try to add to existing stacks
    if (maxStackSize > 1) {
        for (auto& slot : m_slots) {
            if (!slot.isEmpty && slot.item.itemId == itemId) {
                int canAdd = std::min(remainingQuantity, maxStackSize - slot.item.quantity);
                if (canAdd > 0) {
                    slot.item.quantity += canAdd;
                    remainingQuantity -= canAdd;
                    
                    if (remainingQuantity == 0) {
                        break;
                    }
                }
            }
        }
    }
    
    // Then, create new stacks in empty slots
    while (remainingQuantity > 0) {
        int emptySlot = findEmptySlot();
        if (emptySlot == -1) {
            break; // Inventory full
        }
        
        int stackSize = std::min(remainingQuantity, maxStackSize);
        ItemInstance newItem(itemId, stackSize);
        
        // Set durability for equipment
        if (definition->type == ItemType::Equipment && definition->maxStackSize == 1) {
            // Equipment items typically have durability
            newItem.maxDurability = 100; // Default durability
            newItem.durability = 100;
        }
        
        m_slots[emptySlot].setItem(newItem);
        remainingQuantity -= stackSize;
    }
    
    int actuallyAdded = quantity - remainingQuantity;
    if (actuallyAdded > 0) {
        triggerItemAdded(itemId, actuallyAdded);
    }
    
    return actuallyAdded;
}

int InventoryComponent::addItem(const ItemInstance& item) {
    if (item.quantity <= 0 || !hasItemDefinition(item.itemId)) {
        return 0;
    }
    
    const ItemDefinition* definition = getItemDefinition(item.itemId);
    if (!definition) {
        return 0;
    }
    
    int remainingQuantity = item.quantity;
    int maxStackSize = definition->maxStackSize;
    
    // First, try to add to existing stacks (only if items can stack)
    if (maxStackSize > 1) {
        for (auto& slot : m_slots) {
            if (!slot.isEmpty && canStack(slot.item, item)) {
                int canAdd = std::min(remainingQuantity, maxStackSize - slot.item.quantity);
                if (canAdd > 0) {
                    slot.item.quantity += canAdd;
                    remainingQuantity -= canAdd;
                    
                    if (remainingQuantity == 0) {
                        break;
                    }
                }
            }
        }
    }
    
    // Then, create new stacks in empty slots
    while (remainingQuantity > 0) {
        int emptySlot = findEmptySlot();
        if (emptySlot == -1) {
            break; // Inventory full
        }
        
        int stackSize = std::min(remainingQuantity, maxStackSize);
        ItemInstance newItem = item;
        newItem.quantity = stackSize;
        
        m_slots[emptySlot].setItem(newItem);
        remainingQuantity -= stackSize;
    }
    
    int actuallyAdded = item.quantity - remainingQuantity;
    if (actuallyAdded > 0) {
        triggerItemAdded(item.itemId, actuallyAdded);
    }
    
    return actuallyAdded;
}

int InventoryComponent::removeItem(const std::string& itemId, int quantity) {
    if (quantity <= 0) {
        return 0;
    }
    
    int remainingToRemove = quantity;
    
    // Remove from slots, starting from the end to avoid index issues
    for (int i = static_cast<int>(m_slots.size()) - 1; i >= 0 && remainingToRemove > 0; --i) {
        auto& slot = m_slots[i];
        if (!slot.isEmpty && slot.item.itemId == itemId) {
            int canRemove = std::min(remainingToRemove, slot.item.quantity);
            slot.item.quantity -= canRemove;
            remainingToRemove -= canRemove;
            
            if (slot.item.quantity <= 0) {
                slot.clear();
            }
        }
    }
    
    int actuallyRemoved = quantity - remainingToRemove;
    if (actuallyRemoved > 0) {
        triggerItemRemoved(itemId, actuallyRemoved);
    }
    
    return actuallyRemoved;
}

int InventoryComponent::removeItemFromSlot(int slotIndex, int quantity) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(m_slots.size()) || quantity <= 0) {
        return 0;
    }
    
    auto& slot = m_slots[slotIndex];
    if (slot.isEmpty) {
        return 0;
    }
    
    int canRemove = std::min(quantity, slot.item.quantity);
    std::string itemId = slot.item.itemId;
    
    slot.item.quantity -= canRemove;
    if (slot.item.quantity <= 0) {
        slot.clear();
    }
    
    if (canRemove > 0) {
        triggerItemRemoved(itemId, canRemove);
    }
    
    return canRemove;
}

bool InventoryComponent::useItem(const std::string& itemId, int quantity) {
    if (!hasItem(itemId, quantity)) {
        return false;
    }
    
    const ItemDefinition* definition = getItemDefinition(itemId);
    if (!definition) {
        return false;
    }
    
    // Apply item effects
    applyItemEffects(itemId, quantity);
    
    // Remove item if consumable
    if (definition->consumable) {
        removeItem(itemId, quantity);
    }
    
    triggerItemUsed(itemId, quantity);
    return true;
}

bool InventoryComponent::useItemFromSlot(int slotIndex, int quantity) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(m_slots.size())) {
        return false;
    }
    
    const auto& slot = m_slots[slotIndex];
    if (slot.isEmpty || slot.item.quantity < quantity) {
        return false;
    }
    
    return useItem(slot.item.itemId, quantity);
}

bool InventoryComponent::hasItem(const std::string& itemId, int quantity) const {
    return getItemQuantity(itemId) >= quantity;
}

int InventoryComponent::getItemQuantity(const std::string& itemId) const {
    int totalQuantity = 0;
    
    for (const auto& slot : m_slots) {
        if (!slot.isEmpty && slot.item.itemId == itemId) {
            totalQuantity += slot.item.quantity;
        }
    }
    
    return totalQuantity;
}

int InventoryComponent::findItemSlot(const std::string& itemId) const {
    for (int i = 0; i < static_cast<int>(m_slots.size()); ++i) {
        if (!m_slots[i].isEmpty && m_slots[i].item.itemId == itemId) {
            return i;
        }
    }
    return -1;
}

int InventoryComponent::findEmptySlot() const {
    for (int i = 0; i < static_cast<int>(m_slots.size()); ++i) {
        if (m_slots[i].isEmpty) {
            return i;
        }
    }
    return -1;
}

void InventoryComponent::setCapacity(int capacity) {
    if (capacity < 0) {
        capacity = 0;
    }
    
    int oldCapacity = static_cast<int>(m_slots.size());
    
    if (capacity > oldCapacity) {
        // Expand inventory
        m_slots.resize(capacity);
        for (int i = oldCapacity; i < capacity; ++i) {
            m_slots[i].clear();
        }
    } else if (capacity < oldCapacity) {
        // Shrink inventory - move items from removed slots to available slots
        std::vector<ItemInstance> itemsToRelocate;
        
        // Collect items from slots that will be removed
        for (int i = capacity; i < oldCapacity; ++i) {
            if (!m_slots[i].isEmpty) {
                itemsToRelocate.push_back(m_slots[i].item);
            }
        }
        
        // Resize inventory
        m_slots.resize(capacity);
        
        // Try to relocate items
        for (const auto& item : itemsToRelocate) {
            addItem(item);
        }
    }
}

int InventoryComponent::getUsedSlots() const {
    int usedSlots = 0;
    for (const auto& slot : m_slots) {
        if (!slot.isEmpty) {
            usedSlots++;
        }
    }
    return usedSlots;
}

const InventorySlot& InventoryComponent::getSlot(int index) const {
    static InventorySlot emptySlot;
    
    if (index >= 0 && index < static_cast<int>(m_slots.size())) {
        return m_slots[index];
    }
    
    return emptySlot;
}

void InventoryComponent::clear() {
    for (auto& slot : m_slots) {
        slot.clear();
    }
    m_equipment = EquipmentSet();
}

void InventoryComponent::sortInventory(bool sortByType) {
    // Collect all items
    std::vector<ItemInstance> items;
    for (const auto& slot : m_slots) {
        if (!slot.isEmpty) {
            items.push_back(slot.item);
        }
    }
    
    // Clear all slots
    for (auto& slot : m_slots) {
        slot.clear();
    }
    
    // Sort items
    if (sortByType) {
        std::sort(items.begin(), items.end(), [](const ItemInstance& a, const ItemInstance& b) {
            const ItemDefinition* defA = getItemDefinition(a.itemId);
            const ItemDefinition* defB = getItemDefinition(b.itemId);
            
            if (defA && defB) {
                if (defA->type != defB->type) {
                    return static_cast<int>(defA->type) < static_cast<int>(defB->type);
                }
                if (defA->rarity != defB->rarity) {
                    return static_cast<int>(defA->rarity) > static_cast<int>(defB->rarity);
                }
                return defA->name < defB->name;
            }
            
            return a.itemId < b.itemId;
        });
    } else {
        std::sort(items.begin(), items.end(), [](const ItemInstance& a, const ItemInstance& b) {
            const ItemDefinition* defA = getItemDefinition(a.itemId);
            const ItemDefinition* defB = getItemDefinition(b.itemId);
            
            if (defA && defB) {
                return defA->name < defB->name;
            }
            
            return a.itemId < b.itemId;
        });
    }
    
    // Re-add items to inventory
    for (const auto& item : items) {
        addItem(item);
    }
}

bool InventoryComponent::equipItem(const std::string& itemId, EquipmentSlot slot) {
    const ItemDefinition* definition = getItemDefinition(itemId);
    if (!definition || definition->type != ItemType::Equipment) {
        return false;
    }
    
    // Auto-detect slot if not specified
    if (slot == EquipmentSlot::None) {
        slot = definition->equipmentSlot;
    }
    
    if (slot == EquipmentSlot::None) {
        return false; // Item is not equippable
    }
    
    // Find item in inventory
    int itemSlot = findItemSlot(itemId);
    if (itemSlot == -1) {
        return false;
    }
    
    // Unequip current item if any
    if (isEquipped(slot)) {
        if (!unequipItem(slot)) {
            return false; // Failed to unequip current item
        }
    }
    
    // Move item from inventory to equipment
    ItemInstance item = m_slots[itemSlot].item;
    item.quantity = 1; // Equipment items are always quantity 1
    
    removeItemFromSlot(itemSlot, 1);
    m_equipment.equipItem(slot, item);
    
    triggerItemEquipped(itemId, slot);
    return true;
}

bool InventoryComponent::equipItemFromSlot(int slotIndex, EquipmentSlot equipSlot) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(m_slots.size())) {
        return false;
    }
    
    const auto& slot = m_slots[slotIndex];
    if (slot.isEmpty) {
        return false;
    }
    
    return equipItem(slot.item.itemId, equipSlot);
}

bool InventoryComponent::unequipItem(EquipmentSlot slot) {
    if (!isEquipped(slot)) {
        return false;
    }
    
    ItemInstance item = m_equipment.unequipItem(slot);
    
    // Try to add item back to inventory
    int added = addItem(item);
    if (added == 0) {
        // Inventory full, re-equip item
        m_equipment.equipItem(slot, item);
        return false;
    }
    
    triggerItemUnequipped(item.itemId, slot);
    return true;
}

void InventoryComponent::registerItemDefinition(const ItemDefinition& definition) {
    s_itemDefinitions[definition.id] = definition;
    std::cout << "Registered item definition: " << definition.name << " (" << definition.id << ")" << std::endl;
}

const ItemDefinition* InventoryComponent::getItemDefinition(const std::string& itemId) {
    auto it = s_itemDefinitions.find(itemId);
    return (it != s_itemDefinitions.end()) ? &it->second : nullptr;
}

bool InventoryComponent::hasItemDefinition(const std::string& itemId) {
    return s_itemDefinitions.find(itemId) != s_itemDefinitions.end();
}

std::string InventoryComponent::serialize() const {
    std::ostringstream oss;
    
    // Serialize inventory slots
    oss << m_slots.size() << ",";
    for (const auto& slot : m_slots) {
        if (slot.isEmpty) {
            oss << "empty;";
        } else {
            oss << slot.item.itemId << "|" << slot.item.quantity << "|" 
                << slot.item.durability << "|" << slot.item.maxDurability << ";";
        }
    }
    oss << ",";
    
    // Serialize equipment
    oss << m_equipment.equippedItems.size() << ",";
    for (const auto& pair : m_equipment.equippedItems) {
        oss << static_cast<int>(pair.first) << "|" << pair.second.itemId << "|" 
            << pair.second.quantity << "|" << pair.second.durability << "|" 
            << pair.second.maxDurability << ";";
    }
    
    return oss.str();
}

bool InventoryComponent::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    
    try {
        // Deserialize inventory slots
        std::getline(iss, token, ',');
        int slotCount = std::stoi(token);
        
        m_slots.clear();
        m_slots.resize(slotCount);
        
        std::getline(iss, token, ',');
        std::istringstream slotStream(token);
        std::string slotToken;
        
        int slotIndex = 0;
        while (std::getline(slotStream, slotToken, ';') && slotIndex < slotCount) {
            if (slotToken == "empty") {
                m_slots[slotIndex].clear();
            } else {
                std::istringstream slotData(slotToken);
                std::string part;
                std::vector<std::string> parts;
                
                while (std::getline(slotData, part, '|')) {
                    parts.push_back(part);
                }
                
                if (parts.size() >= 4) {
                    ItemInstance item(parts[0], std::stoi(parts[1]), std::stoi(parts[2]));
                    item.maxDurability = std::stoi(parts[3]);
                    m_slots[slotIndex].setItem(item);
                }
            }
            slotIndex++;
        }
        
        // Deserialize equipment
        std::getline(iss, token, ',');
        int equipmentCount = std::stoi(token);
        
        m_equipment.equippedItems.clear();
        
        std::string equipmentData;
        std::getline(iss, equipmentData);
        std::istringstream equipStream(equipmentData);
        std::string equipToken;
        
        while (std::getline(equipStream, equipToken, ';') && equipmentCount > 0) {
            if (!equipToken.empty()) {
                std::istringstream equipData(equipToken);
                std::string part;
                std::vector<std::string> parts;
                
                while (std::getline(equipData, part, '|')) {
                    parts.push_back(part);
                }
                
                if (parts.size() >= 5) {
                    EquipmentSlot slot = static_cast<EquipmentSlot>(std::stoi(parts[0]));
                    ItemInstance item(parts[1], std::stoi(parts[2]), std::stoi(parts[3]));
                    item.maxDurability = std::stoi(parts[4]);
                    m_equipment.equipItem(slot, item);
                }
                equipmentCount--;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize InventoryComponent: " << e.what() << std::endl;
        return false;
    }
}

bool InventoryComponent::canStack(const ItemInstance& item1, const ItemInstance& item2) const {
    if (item1.itemId != item2.itemId) {
        return false;
    }
    
    // Items with different durability cannot stack
    if (item1.durability != item2.durability || item1.maxDurability != item2.maxDurability) {
        return false;
    }
    
    // Check custom properties
    if (item1.customProperties != item2.customProperties) {
        return false;
    }
    
    return true;
}

int InventoryComponent::getMaxStackSize(const std::string& itemId) const {
    const ItemDefinition* definition = getItemDefinition(itemId);
    return definition ? definition->maxStackSize : 1;
}

void InventoryComponent::applyItemEffects(const std::string& itemId, int quantity) {
    const ItemDefinition* definition = getItemDefinition(itemId);
    if (!definition) {
        return;
    }
    
    // Apply each effect
    for (const auto& effect : definition->effects) {
        float totalValue = effect.value * quantity;
        
        std::cout << "Applying item effect: " << effect.effectId 
                  << " to " << effect.targetStat 
                  << " (value: " << totalValue << ")" << std::endl;
        
        // TODO: Apply effect to entity's stats component
        // This would require access to the entity's StatsComponent
        // For now, just log the effect
    }
}

void InventoryComponent::triggerItemAdded(const std::string& itemId, int quantity) {
    if (m_itemAddedCallback) {
        m_itemAddedCallback(itemId, quantity);
    }
}

void InventoryComponent::triggerItemRemoved(const std::string& itemId, int quantity) {
    if (m_itemRemovedCallback) {
        m_itemRemovedCallback(itemId, quantity);
    }
}

void InventoryComponent::triggerItemUsed(const std::string& itemId, int quantity) {
    if (m_itemUsedCallback) {
        m_itemUsedCallback(itemId, quantity);
    }
}

void InventoryComponent::triggerItemEquipped(const std::string& itemId, EquipmentSlot slot) {
    if (m_itemEquippedCallback) {
        m_itemEquippedCallback(itemId, slot);
    }
}

void InventoryComponent::triggerItemUnequipped(const std::string& itemId, EquipmentSlot slot) {
    if (m_itemUnequippedCallback) {
        m_itemUnequippedCallback(itemId, slot);
    }
}

} // namespace Components
} // namespace RPGEngine