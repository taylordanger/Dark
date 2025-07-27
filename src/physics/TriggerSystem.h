#pragma once

#include "../systems/System.h"
#include "../components/ComponentManager.h"
#include "CollisionSystem.h"
#include "TriggerComponent.h"
#include "PhysicsComponent.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace RPGEngine {
namespace Physics {

/**
 * Trigger system
 * Manages trigger events for entities with TriggerComponent
 */
class TriggerSystem : public System {
public:
    /**
     * Constructor
     * @param componentManager Component manager
     * @param collisionSystem Collision system
     */
    TriggerSystem(std::shared_ptr<ComponentManager> componentManager, 
                 std::shared_ptr<CollisionSystem> collisionSystem);
    
    /**
     * Destructor
     */
    ~TriggerSystem();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Register a global callback for all trigger events
     * @param callback Callback function
     * @return Callback ID
     */
    int registerGlobalCallback(const std::function<void(const TriggerEvent&)>& callback);
    
    /**
     * Unregister a global callback
     * @param callbackId Callback ID
     * @return true if the callback was unregistered
     */
    bool unregisterGlobalCallback(int callbackId);
    
    /**
     * Get entities currently in a trigger
     * @param triggerEntity Trigger entity
     * @return Vector of entities in the trigger
     */
    std::vector<Entity> getEntitiesInTrigger(Entity triggerEntity) const;
    
    /**
     * Check if an entity is in a trigger
     * @param triggerEntity Trigger entity
     * @param entity Entity to check
     * @return true if the entity is in the trigger
     */
    bool isEntityInTrigger(Entity triggerEntity, Entity entity) const;
    
    /**
     * Get triggers that an entity is currently in
     * @param entity Entity
     * @return Vector of trigger entities
     */
    std::vector<Entity> getTriggersContainingEntity(Entity entity) const;
    
    /**
     * Get all triggers with a specific tag
     * @param tag Trigger tag
     * @return Vector of trigger entities
     */
    std::vector<Entity> getTriggersByTag(const std::string& tag) const;
    
private:
    /**
     * Handle a collision event
     * @param event Collision event
     */
    void handleCollisionEvent(const CollisionEvent& event);
    
    /**
     * Fire a trigger event
     * @param triggerEntity Trigger entity
     * @param otherEntity Other entity
     * @param eventType Event type
     * @param time Event time
     */
    void fireTriggerEvent(Entity triggerEntity, Entity otherEntity, TriggerEventType eventType, float time);
    
    // Component manager
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // Collision system
    std::shared_ptr<CollisionSystem> m_collisionSystem;
    
    // Collision callback ID
    int m_collisionCallbackId;
    
    // Global callbacks
    std::unordered_map<int, std::function<void(const TriggerEvent&)>> m_globalCallbacks;
    int m_nextCallbackId;
    
    // Entities in triggers
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_entitiesInTrigger;
    
    // Current time
    float m_currentTime;
};

} // namespace Physics
} // namespace RPGEngine