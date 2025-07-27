#include "TriggerSystem.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Physics {

TriggerSystem::TriggerSystem(std::shared_ptr<ComponentManager> componentManager, 
                           std::shared_ptr<CollisionSystem> collisionSystem)
    : System("TriggerSystem")
    , m_componentManager(componentManager)
    , m_collisionSystem(collisionSystem)
    , m_collisionCallbackId(-1)
    , m_nextCallbackId(1)
    , m_currentTime(0.0f)
{
}

TriggerSystem::~TriggerSystem() {
    if (isInitialized()) {
        shutdown();
    }
}

bool TriggerSystem::onInitialize() {
    if (!m_componentManager) {
        std::cerr << "Component manager not provided to TriggerSystem" << std::endl;
        return false;
    }
    
    if (!m_collisionSystem) {
        std::cerr << "Collision system not provided to TriggerSystem" << std::endl;
        return false;
    }
    
    // Register collision callback
    m_collisionCallbackId = m_collisionSystem->registerCollisionCallback(
        [this](const CollisionEvent& event) {
            handleCollisionEvent(event);
        }
    );
    
    std::cout << "TriggerSystem initialized" << std::endl;
    return true;
}

void TriggerSystem::onUpdate(float deltaTime) {
    // Update current time
    m_currentTime += deltaTime;
    
    // Get all entities with trigger components
    auto triggerEntities = m_componentManager->getEntitiesWithComponent<TriggerComponent>();
    
    // Update trigger cooldowns
    for (auto entity : triggerEntities) {
        auto triggerComponent = m_componentManager->getComponent<TriggerComponent>(entity);
        if (triggerComponent) {
            // Update cooldown timer
            float cooldownTimer = triggerComponent->getCooldownTimer();
            if (cooldownTimer > 0.0f) {
                cooldownTimer -= deltaTime;
                if (cooldownTimer <= 0.0f) {
                    cooldownTimer = 0.0f;
                    triggerComponent->setTriggered(false);
                }
                triggerComponent->setCooldownTimer(cooldownTimer);
            }
            
            // Fire stay events for entities in the trigger
            if (triggerComponent->isActive() && !triggerComponent->isTriggered()) {
                auto it = m_entitiesInTrigger.find(entity.getID());
                if (it != m_entitiesInTrigger.end()) {
                    for (auto otherEntityId : it->second) {
                        Entity otherEntity(otherEntityId);
                        fireTriggerEvent(entity, otherEntity, TriggerEventType::Stay, m_currentTime);
                    }
                }
            }
        }
    }
}

void TriggerSystem::onShutdown() {
    // Unregister collision callback
    if (m_collisionCallbackId != -1) {
        m_collisionSystem->unregisterCollisionCallback(m_collisionCallbackId);
        m_collisionCallbackId = -1;
    }
    
    // Clear data
    m_entitiesInTrigger.clear();
    m_globalCallbacks.clear();
    
    std::cout << "TriggerSystem shutdown" << std::endl;
}

int TriggerSystem::registerGlobalCallback(const std::function<void(const TriggerEvent&)>& callback) {
    if (!callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_globalCallbacks[callbackId] = callback;
    return callbackId;
}

bool TriggerSystem::unregisterGlobalCallback(int callbackId) {
    auto it = m_globalCallbacks.find(callbackId);
    if (it != m_globalCallbacks.end()) {
        m_globalCallbacks.erase(it);
        return true;
    }
    return false;
}

std::vector<Entity> TriggerSystem::getEntitiesInTrigger(Entity triggerEntity) const {
    std::vector<Entity> entities;
    
    auto it = m_entitiesInTrigger.find(triggerEntity.getID());
    if (it != m_entitiesInTrigger.end()) {
        for (auto entityId : it->second) {
            entities.push_back(Entity(entityId));
        }
    }
    
    return entities;
}

bool TriggerSystem::isEntityInTrigger(Entity triggerEntity, Entity entity) const {
    auto it = m_entitiesInTrigger.find(triggerEntity.getID());
    if (it != m_entitiesInTrigger.end()) {
        return it->second.find(entity.getID()) != it->second.end();
    }
    return false;
}

std::vector<Entity> TriggerSystem::getTriggersContainingEntity(Entity entity) const {
    std::vector<Entity> triggers;
    
    for (const auto& pair : m_entitiesInTrigger) {
        if (pair.second.find(entity.getID()) != pair.second.end()) {
            triggers.push_back(Entity(pair.first));
        }
    }
    
    return triggers;
}

std::vector<Entity> TriggerSystem::getTriggersByTag(const std::string& tag) const {
    std::vector<Entity> triggers;
    
    auto triggerEntities = m_componentManager->getEntitiesWithComponent<TriggerComponent>();
    for (auto entity : triggerEntities) {
        auto triggerComponent = m_componentManager->getComponent<TriggerComponent>(entity);
        if (triggerComponent && triggerComponent->getTag() == tag) {
            triggers.push_back(entity);
        }
    }
    
    return triggers;
}

void TriggerSystem::handleCollisionEvent(const CollisionEvent& event) {
    // Get the entities from the collidables
    auto collidable1 = std::dynamic_pointer_cast<PhysicsCollidable>(event.collidable1);
    auto collidable2 = std::dynamic_pointer_cast<PhysicsCollidable>(event.collidable2);
    
    if (!collidable1 || !collidable2) {
        return;
    }
    
    Entity entity1 = collidable1->getEntity();
    Entity entity2 = collidable2->getEntity();
    
    // Check if either entity has a trigger component
    auto triggerComponent1 = m_componentManager->getComponent<TriggerComponent>(entity1);
    auto triggerComponent2 = m_componentManager->getComponent<TriggerComponent>(entity2);
    
    // Check if either entity is a trigger
    auto physicsComponent1 = collidable1->getPhysicsComponent();
    auto physicsComponent2 = collidable2->getPhysicsComponent();
    
    bool isTrigger1 = physicsComponent1->isTrigger() && triggerComponent1;
    bool isTrigger2 = physicsComponent2->isTrigger() && triggerComponent2;
    
    // Handle entity1 as trigger
    if (isTrigger1) {
        // Check if the trigger is active
        if (!triggerComponent1->isActive()) {
            return;
        }
        
        // Check if the entity is in the filter list
        if (!triggerComponent1->isEntityInFilter(entity2)) {
            return;
        }
        
        // Check if the entity is already in the trigger
        bool alreadyInTrigger = isEntityInTrigger(entity1, entity2);
        
        if (!alreadyInTrigger) {
            // Add the entity to the trigger
            m_entitiesInTrigger[entity1.getID()].insert(entity2.getID());
            
            // Check if the trigger is one-shot and already triggered
            if (triggerComponent1->isOneShot() && triggerComponent1->isTriggered()) {
                return;
            }
            
            // Check if the trigger is on cooldown
            if (triggerComponent1->getCooldownTimer() > 0.0f) {
                return;
            }
            
            // Fire enter event
            fireTriggerEvent(entity1, entity2, TriggerEventType::Enter, m_currentTime);
            
            // Mark the trigger as triggered
            triggerComponent1->setTriggered(true);
            
            // Start cooldown timer
            if (triggerComponent1->getCooldown() > 0.0f) {
                triggerComponent1->setCooldownTimer(triggerComponent1->getCooldown());
            }
        }
    }
    
    // Handle entity2 as trigger
    if (isTrigger2) {
        // Check if the trigger is active
        if (!triggerComponent2->isActive()) {
            return;
        }
        
        // Check if the entity is in the filter list
        if (!triggerComponent2->isEntityInFilter(entity1)) {
            return;
        }
        
        // Check if the entity is already in the trigger
        bool alreadyInTrigger = isEntityInTrigger(entity2, entity1);
        
        if (!alreadyInTrigger) {
            // Add the entity to the trigger
            m_entitiesInTrigger[entity2.getID()].insert(entity1.getID());
            
            // Check if the trigger is one-shot and already triggered
            if (triggerComponent2->isOneShot() && triggerComponent2->isTriggered()) {
                return;
            }
            
            // Check if the trigger is on cooldown
            if (triggerComponent2->getCooldownTimer() > 0.0f) {
                return;
            }
            
            // Fire enter event
            fireTriggerEvent(entity2, entity1, TriggerEventType::Enter, m_currentTime);
            
            // Mark the trigger as triggered
            triggerComponent2->setTriggered(true);
            
            // Start cooldown timer
            if (triggerComponent2->getCooldown() > 0.0f) {
                triggerComponent2->setCooldownTimer(triggerComponent2->getCooldown());
            }
        }
    }
}

void TriggerSystem::fireTriggerEvent(Entity triggerEntity, Entity otherEntity, TriggerEventType eventType, float time) {
    // Create trigger event
    TriggerEvent event(triggerEntity, otherEntity, eventType, time);
    
    // Fire trigger component callbacks
    auto triggerComponent = m_componentManager->getComponent<TriggerComponent>(triggerEntity);
    if (triggerComponent) {
        const auto& callbacks = triggerComponent->getCallbacks(eventType);
        for (const auto& callback : callbacks) {
            callback(event);
        }
    }
    
    // Fire global callbacks
    for (const auto& pair : m_globalCallbacks) {
        pair.second(event);
    }
    
    // Handle exit events
    if (eventType == TriggerEventType::Exit) {
        // Remove the entity from the trigger
        auto it = m_entitiesInTrigger.find(triggerEntity.getID());
        if (it != m_entitiesInTrigger.end()) {
            it->second.erase(otherEntity.getID());
        }
    }
}

} // namespace Physics
} // namespace RPGEngine