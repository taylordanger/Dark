#pragma once

#include "../components/Component.h"
#include "CollisionShape.h"
#include <memory>

namespace RPGEngine {
namespace Physics {

/**
 * Physics component
 * Stores physics-related properties for an entity
 */
class PhysicsComponent : public RPGEngine::Components::Component<PhysicsComponent> {
public:
    /**
     * Constructor
     */
    PhysicsComponent(EntityId entityId)
        : RPGEngine::Components::Component<PhysicsComponent>(entityId)
        , m_position(0.0f, 0.0f)
        , m_velocity(0.0f, 0.0f)
        , m_acceleration(0.0f, 0.0f)
        , m_rotation(0.0f)
        , m_angularVelocity(0.0f)
        , m_mass(1.0f)
        , m_friction(0.5f)
        , m_restitution(0.2f)
        , m_gravityScale(1.0f)
        , m_isStatic(false)
        , m_isTrigger(false)
        , m_collisionLayer(1)
        , m_collisionMask(0xFFFFFFFF)
    {}
    
    /**
     * Get the component type
     * @return Component type
     */
    // Inherited from Component<PhysicsComponent>
    
    /**
     * Get the component type
     * @return Component type
     */
    // Inherited from Component<PhysicsComponent>
    
    /**
     * Get the position
     * @return Position
     */
    const Vector2& getPosition() const { return m_position; }
    
    /**
     * Set the position
     * @param position Position
     */
    void setPosition(const Vector2& position) { m_position = position; }
    
    /**
     * Set the position
     * @param x X position
     * @param y Y position
     */
    void setPosition(float x, float y) { m_position.x = x; m_position.y = y; }
    
    /**
     * Get the velocity
     * @return Velocity
     */
    const Vector2& getVelocity() const { return m_velocity; }
    
    /**
     * Set the velocity
     * @param velocity Velocity
     */
    void setVelocity(const Vector2& velocity) { m_velocity = velocity; }
    
    /**
     * Set the velocity
     * @param x X velocity
     * @param y Y velocity
     */
    void setVelocity(float x, float y) { m_velocity.x = x; m_velocity.y = y; }
    
    /**
     * Get the acceleration
     * @return Acceleration
     */
    const Vector2& getAcceleration() const { return m_acceleration; }
    
    /**
     * Set the acceleration
     * @param acceleration Acceleration
     */
    void setAcceleration(const Vector2& acceleration) { m_acceleration = acceleration; }
    
    /**
     * Set the acceleration
     * @param x X acceleration
     * @param y Y acceleration
     */
    void setAcceleration(float x, float y) { m_acceleration.x = x; m_acceleration.y = y; }
    
    /**
     * Get the rotation
     * @return Rotation in radians
     */
    float getRotation() const { return m_rotation; }
    
    /**
     * Set the rotation
     * @param rotation Rotation in radians
     */
    void setRotation(float rotation) { m_rotation = rotation; }
    
    /**
     * Get the angular velocity
     * @return Angular velocity in radians per second
     */
    float getAngularVelocity() const { return m_angularVelocity; }
    
    /**
     * Set the angular velocity
     * @param angularVelocity Angular velocity in radians per second
     */
    void setAngularVelocity(float angularVelocity) { m_angularVelocity = angularVelocity; }
    
    /**
     * Get the mass
     * @return Mass
     */
    float getMass() const { return m_mass; }
    
    /**
     * Set the mass
     * @param mass Mass
     */
    void setMass(float mass) { m_mass = mass; }
    
    /**
     * Get the friction
     * @return Friction coefficient
     */
    float getFriction() const { return m_friction; }
    
    /**
     * Set the friction
     * @param friction Friction coefficient
     */
    void setFriction(float friction) { m_friction = friction; }
    
    /**
     * Get the restitution
     * @return Restitution coefficient
     */
    float getRestitution() const { return m_restitution; }
    
    /**
     * Set the restitution
     * @param restitution Restitution coefficient
     */
    void setRestitution(float restitution) { m_restitution = restitution; }
    
    /**
     * Get the gravity scale
     * @return Gravity scale
     */
    float getGravityScale() const { return m_gravityScale; }
    
    /**
     * Set the gravity scale
     * @param gravityScale Gravity scale
     */
    void setGravityScale(float gravityScale) { m_gravityScale = gravityScale; }
    
    /**
     * Check if the body is static
     * @return true if the body is static
     */
    bool isStatic() const { return m_isStatic; }
    
    /**
     * Set whether the body is static
     * @param isStatic Whether the body is static
     */
    void setStatic(bool isStatic) { m_isStatic = isStatic; }
    
    /**
     * Check if the body is a trigger
     * @return true if the body is a trigger
     */
    bool isTrigger() const { return m_isTrigger; }
    
    /**
     * Set whether the body is a trigger
     * @param isTrigger Whether the body is a trigger
     */
    void setTrigger(bool isTrigger) { m_isTrigger = isTrigger; }
    
    /**
     * Get the collision shape
     * @return Collision shape
     */
    std::shared_ptr<CollisionShape> getCollisionShape() const { return m_collisionShape; }
    
    /**
     * Set the collision shape
     * @param shape Collision shape
     */
    void setCollisionShape(std::shared_ptr<CollisionShape> shape) { m_collisionShape = shape; }
    
    /**
     * Get the collision layer
     * @return Collision layer
     */
    uint32_t getCollisionLayer() const { return m_collisionLayer; }
    
    /**
     * Set the collision layer
     * @param layer Collision layer
     */
    void setCollisionLayer(uint32_t layer) { m_collisionLayer = layer; }
    
    /**
     * Get the collision mask
     * @return Collision mask
     */
    uint32_t getCollisionMask() const { return m_collisionMask; }
    
    /**
     * Set the collision mask
     * @param mask Collision mask
     */
    void setCollisionMask(uint32_t mask) { m_collisionMask = mask; }
    
    /**
     * Apply a force to the body
     * @param force Force to apply
     */
    void applyForce(const Vector2& force) {
        if (!m_isStatic && m_mass > 0.0f) {
            m_acceleration = m_acceleration + force * (1.0f / m_mass);
        }
    }
    
    /**
     * Apply an impulse to the body
     * @param impulse Impulse to apply
     */
    void applyImpulse(const Vector2& impulse) {
        if (!m_isStatic && m_mass > 0.0f) {
            m_velocity = m_velocity + impulse * (1.0f / m_mass);
        }
    }
    
private:
    Vector2 m_position;                      // Position
    Vector2 m_velocity;                      // Velocity
    Vector2 m_acceleration;                  // Acceleration
    float m_rotation;                        // Rotation in radians
    float m_angularVelocity;                 // Angular velocity in radians per second
    float m_mass;                            // Mass
    float m_friction;                        // Friction coefficient
    float m_restitution;                     // Restitution coefficient
    float m_gravityScale;                    // Gravity scale
    bool m_isStatic;                         // Whether the body is static
    bool m_isTrigger;                        // Whether the body is a trigger
    uint32_t m_collisionLayer;               // Collision layer
    uint32_t m_collisionMask;                // Collision mask
    std::shared_ptr<CollisionShape> m_collisionShape; // Collision shape
};

} // namespace Physics
} // namespace RPGEngine