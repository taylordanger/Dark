#pragma once

#include "../systems/System.h"
#include "../components/ComponentManager.h"
#include "CollisionSystem.h"
#include "PhysicsComponent.h"
#include "PhysicsCollidable.h"
#include <memory>
#include <unordered_map>

namespace RPGEngine {
namespace Physics {

/**
 * Movement system
 * Handles entity movement and collision resolution
 */
class MovementSystem : public System {
public:
    /**
     * Constructor
     * @param componentManager Component manager
     * @param collisionSystem Collision system
     */
    MovementSystem(std::shared_ptr<ComponentManager> componentManager, 
                  std::shared_ptr<CollisionSystem> collisionSystem);
    
    /**
     * Destructor
     */
    ~MovementSystem();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Set the gravity
     * @param gravity Gravity vector
     */
    void setGravity(const Vector2& gravity) { m_gravity = gravity; }
    
    /**
     * Get the gravity
     * @return Gravity vector
     */
    const Vector2& getGravity() const { return m_gravity; }
    
    /**
     * Set the velocity damping
     * @param damping Velocity damping factor
     */
    void setVelocityDamping(float damping) { m_velocityDamping = damping; }
    
    /**
     * Get the velocity damping
     * @return Velocity damping factor
     */
    float getVelocityDamping() const { return m_velocityDamping; }
    
    /**
     * Set the angular velocity damping
     * @param damping Angular velocity damping factor
     */
    void setAngularVelocityDamping(float damping) { m_angularVelocityDamping = damping; }
    
    /**
     * Get the angular velocity damping
     * @return Angular velocity damping factor
     */
    float getAngularVelocityDamping() const { return m_angularVelocityDamping; }
    
    /**
     * Set the maximum velocity
     * @param maxVelocity Maximum velocity
     */
    void setMaxVelocity(float maxVelocity) { m_maxVelocity = maxVelocity; }
    
    /**
     * Get the maximum velocity
     * @return Maximum velocity
     */
    float getMaxVelocity() const { return m_maxVelocity; }
    
    /**
     * Set the maximum angular velocity
     * @param maxAngularVelocity Maximum angular velocity
     */
    void setMaxAngularVelocity(float maxAngularVelocity) { m_maxAngularVelocity = maxAngularVelocity; }
    
    /**
     * Get the maximum angular velocity
     * @return Maximum angular velocity
     */
    float getMaxAngularVelocity() const { return m_maxAngularVelocity; }
    
    /**
     * Set the collision response enabled state
     * @param enabled Whether collision response is enabled
     */
    void setCollisionResponseEnabled(bool enabled) { m_collisionResponseEnabled = enabled; }
    
    /**
     * Check if collision response is enabled
     * @return true if collision response is enabled
     */
    bool isCollisionResponseEnabled() const { return m_collisionResponseEnabled; }
    
    /**
     * Set the number of iterations for collision resolution
     * @param iterations Number of iterations
     */
    void setCollisionIterations(int iterations) { m_collisionIterations = iterations; }
    
    /**
     * Get the number of iterations for collision resolution
     * @return Number of iterations
     */
    int getCollisionIterations() const { return m_collisionIterations; }
    
private:
    /**
     * Update physics for an entity
     * @param entity Entity
     * @param physicsComponent Physics component
     * @param deltaTime Time step
     */
    void updatePhysics(Entity entity, std::shared_ptr<PhysicsComponent> physicsComponent, float deltaTime);
    
    /**
     * Resolve collisions for an entity
     * @param entity Entity
     * @param physicsComponent Physics component
     * @param collidable Physics collidable
     */
    void resolveCollisions(Entity entity, std::shared_ptr<PhysicsComponent> physicsComponent, 
                          std::shared_ptr<PhysicsCollidable> collidable);
    
    /**
     * Resolve a collision between two entities
     * @param entity1 First entity
     * @param physics1 First physics component
     * @param entity2 Second entity
     * @param physics2 Second physics component
     * @param collisionResult Collision result
     */
    void resolveCollision(Entity entity1, std::shared_ptr<PhysicsComponent> physics1,
                         Entity entity2, std::shared_ptr<PhysicsComponent> physics2,
                         const CollisionResult& collisionResult);
    
    /**
     * Update the collision shape for a physics component
     * @param physicsComponent Physics component
     */
    void updateCollisionShape(std::shared_ptr<PhysicsComponent> physicsComponent);
    
    // Component manager
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // Collision system
    std::shared_ptr<CollisionSystem> m_collisionSystem;
    
    // Physics collidables
    std::unordered_map<EntityID, std::shared_ptr<PhysicsCollidable>> m_collidables;
    
    // Physics settings
    Vector2 m_gravity;                  // Gravity vector
    float m_velocityDamping;            // Velocity damping factor
    float m_angularVelocityDamping;     // Angular velocity damping factor
    float m_maxVelocity;                // Maximum velocity
    float m_maxAngularVelocity;         // Maximum angular velocity
    bool m_collisionResponseEnabled;    // Whether collision response is enabled
    int m_collisionIterations;          // Number of iterations for collision resolution
};

} // namespace Physics
} // namespace RPGEngine