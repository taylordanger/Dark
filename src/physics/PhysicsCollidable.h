#pragma once

#include "SpatialPartitioning.h"
#include "PhysicsComponent.h"
#include "../entities/Entity.h"
#include <memory>

namespace RPGEngine {
namespace Physics {

/**
 * Physics collidable
 * Implements the ICollidable interface for use with the CollisionSystem
 */
class PhysicsCollidable : public ICollidable {
public:
    /**
     * Constructor
     * @param entity Entity
     * @param physicsComponent Physics component
     */
    PhysicsCollidable(Entity entity, std::shared_ptr<PhysicsComponent> physicsComponent)
        : m_entity(entity)
        , m_physicsComponent(physicsComponent)
    {}
    
    /**
     * Get the collision shape
     * @return Collision shape
     */
    const CollisionShape& getCollisionShape() const override {
        return *m_physicsComponent->getCollisionShape();
    }
    
    /**
     * Get the collidable ID
     * @return Collidable ID
     */
    uint32_t getCollidableID() const override {
        return m_entity.getID();
    }
    
    /**
     * Get the collision layer
     * @return Collision layer
     */
    uint32_t getCollisionLayer() const override {
        return m_physicsComponent->getCollisionLayer();
    }
    
    /**
     * Get the collision mask
     * @return Collision mask
     */
    uint32_t getCollisionMask() const override {
        return m_physicsComponent->getCollisionMask();
    }
    
    /**
     * Get the entity
     * @return Entity
     */
    Entity getEntity() const {
        return m_entity;
    }
    
    /**
     * Get the physics component
     * @return Physics component
     */
    std::shared_ptr<PhysicsComponent> getPhysicsComponent() const {
        return m_physicsComponent;
    }
    
private:
    Entity m_entity;
    std::shared_ptr<PhysicsComponent> m_physicsComponent;
};

} // namespace Physics
} // namespace RPGEngine