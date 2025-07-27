#pragma once

#include "../components/Component.h"
#include "../entities/Entity.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace RPGEngine {
namespace Physics {

/**
 * Trigger event types
 */
enum class TriggerEventType {
    Enter,      // Entity enters the trigger
    Stay,       // Entity stays in the trigger
    Exit        // Entity exits the trigger
};

/**
 * Trigger event structure
 */
struct TriggerEvent {
    Entity triggerEntity;     // The trigger entity
    Entity otherEntity;       // The entity that triggered the event
    TriggerEventType type;    // The event type
    float time;               // The time when the event occurred
    
    TriggerEvent(Entity trigger, Entity other, TriggerEventType eventType, float eventTime)
        : triggerEntity(trigger), otherEntity(other), type(eventType), time(eventTime) {}
};

/**
 * Trigger callback function type
 */
using TriggerCallback = std::function<void(const TriggerEvent&)>;

/**
 * Trigger component
 * Used to create trigger zones for event-based interactions
 */
class TriggerComponent : public RPGEngine::Components::Component<TriggerComponent> {
public:
    /**
     * Constructor
     */
    TriggerComponent(EntityId entityId)
        : RPGEngine::Components::Component<TriggerComponent>(entityId)
        , m_isActive(true)
        , m_oneShot(false)
        , m_triggered(false)
        , m_cooldown(0.0f)
        , m_cooldownTimer(0.0f)
    {}
    
    /**
     * Get the component type
     * @return Component type
     */
    // Inherited from Component<TriggerComponent>
    
    /**
     * Get the component type
     * @return Component type
     */
    // Inherited from Component<TriggerComponent>
    
    /**
     * Check if the trigger is active
     * @return true if the trigger is active
     */
    bool isActive() const { return m_isActive; }
    
    /**
     * Set whether the trigger is active
     * @param active Whether the trigger is active
     */
    void setActive(bool active) { m_isActive = active; }
    
    /**
     * Check if the trigger is one-shot
     * @return true if the trigger is one-shot
     */
    bool isOneShot() const { return m_oneShot; }
    
    /**
     * Set whether the trigger is one-shot
     * @param oneShot Whether the trigger is one-shot
     */
    void setOneShot(bool oneShot) { m_oneShot = oneShot; }
    
    /**
     * Check if the trigger has been triggered
     * @return true if the trigger has been triggered
     */
    bool isTriggered() const { return m_triggered; }
    
    /**
     * Set whether the trigger has been triggered
     * @param triggered Whether the trigger has been triggered
     */
    void setTriggered(bool triggered) { m_triggered = triggered; }
    
    /**
     * Get the trigger cooldown
     * @return Trigger cooldown in seconds
     */
    float getCooldown() const { return m_cooldown; }
    
    /**
     * Set the trigger cooldown
     * @param cooldown Trigger cooldown in seconds
     */
    void setCooldown(float cooldown) { m_cooldown = cooldown; }
    
    /**
     * Get the trigger cooldown timer
     * @return Trigger cooldown timer in seconds
     */
    float getCooldownTimer() const { return m_cooldownTimer; }
    
    /**
     * Set the trigger cooldown timer
     * @param timer Trigger cooldown timer in seconds
     */
    void setCooldownTimer(float timer) { m_cooldownTimer = timer; }
    
    /**
     * Add a callback for a specific event type
     * @param type Event type
     * @param callback Callback function
     */
    void addCallback(TriggerEventType type, TriggerCallback callback) {
        m_callbacks[type].push_back(callback);
    }
    
    /**
     * Get callbacks for a specific event type
     * @param type Event type
     * @return Vector of callbacks
     */
    const std::vector<TriggerCallback>& getCallbacks(TriggerEventType type) const {
        static const std::vector<TriggerCallback> emptyCallbacks;
        auto it = m_callbacks.find(type);
        if (it != m_callbacks.end()) {
            return it->second;
        }
        return emptyCallbacks;
    }
    
    /**
     * Clear all callbacks
     */
    void clearCallbacks() {
        m_callbacks.clear();
    }
    
    /**
     * Set the trigger tag
     * @param tag Trigger tag
     */
    void setTag(const std::string& tag) {
        m_tag = tag;
    }
    
    /**
     * Get the trigger tag
     * @return Trigger tag
     */
    const std::string& getTag() const {
        return m_tag;
    }
    
    /**
     * Add an entity to the filter list
     * @param entity Entity to add
     */
    void addFilterEntity(Entity entity) {
        m_filterEntities.push_back(entity);
    }
    
    /**
     * Remove an entity from the filter list
     * @param entity Entity to remove
     */
    void removeFilterEntity(Entity entity) {
        m_filterEntities.erase(
            std::remove_if(m_filterEntities.begin(), m_filterEntities.end(),
                [entity](const Entity& e) { return e.getID() == entity.getID(); }),
            m_filterEntities.end()
        );
    }
    
    /**
     * Clear the filter list
     */
    void clearFilterEntities() {
        m_filterEntities.clear();
    }
    
    /**
     * Check if an entity is in the filter list
     * @param entity Entity to check
     * @return true if the entity is in the filter list
     */
    bool isEntityInFilter(Entity entity) const {
        // If the filter list is empty, all entities can trigger
        if (m_filterEntities.empty()) {
            return true;
        }
        
        // Otherwise, check if the entity is in the filter list
        for (const auto& e : m_filterEntities) {
            if (e.getID() == entity.getID()) {
                return true;
            }
        }
        
        return false;
    }
    
private:
    bool m_isActive;                                              // Whether the trigger is active
    bool m_oneShot;                                               // Whether the trigger is one-shot
    bool m_triggered;                                              // Whether the trigger has been triggered
    float m_cooldown;                                             // Trigger cooldown in seconds
    float m_cooldownTimer;                                        // Trigger cooldown timer in seconds
    std::string m_tag;                                            // Trigger tag
    std::vector<Entity> m_filterEntities;                         // Entities that can trigger this trigger
    std::unordered_map<TriggerEventType, std::vector<TriggerCallback>> m_callbacks; // Callbacks for each event type
};

} // namespace Physics
} // namespace RPGEngine