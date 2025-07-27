#pragma once

#include "Component.h"

namespace RPGEngine {
namespace Components {

/**
 * Physics Component
 * Stores physics properties for an entity
 */
class PhysicsComponent : public Component<PhysicsComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     * @param velocityX X velocity
     * @param velocityY Y velocity
     * @param mass Mass
     * @param solid Whether the entity is solid (collides with other entities)
     */
    PhysicsComponent(EntityId entityId, float velocityX = 0.0f, float velocityY = 0.0f,
                    float mass = 1.0f, bool solid = true)
        : Component<PhysicsComponent>(entityId)
        , m_velocityX(velocityX)
        , m_velocityY(velocityY)
        , m_mass(mass)
        , m_solid(solid)
        , m_gravityScale(1.0f)
        , m_friction(0.1f)
        , m_restitution(0.5f) // Bounciness
    {
    }
    
    // Velocity
    float getVelocityX() const { return m_velocityX; }
    float getVelocityY() const { return m_velocityY; }
    void setVelocityX(float velocityX) { m_velocityX = velocityX; }
    void setVelocityY(float velocityY) { m_velocityY = velocityY; }
    void setVelocity(float velocityX, float velocityY) { m_velocityX = velocityX; m_velocityY = velocityY; }
    
    // Apply force
    void applyForce(float forceX, float forceY) {
        m_velocityX += forceX / m_mass;
        m_velocityY += forceY / m_mass;
    }
    
    // Apply impulse
    void applyImpulse(float impulseX, float impulseY) {
        m_velocityX += impulseX;
        m_velocityY += impulseY;
    }
    
    // Mass
    float getMass() const { return m_mass; }
    void setMass(float mass) { m_mass = mass > 0.0f ? mass : 0.001f; }
    
    // Solidity
    bool isSolid() const { return m_solid; }
    void setSolid(bool solid) { m_solid = solid; }
    
    // Gravity scale
    float getGravityScale() const { return m_gravityScale; }
    void setGravityScale(float scale) { m_gravityScale = scale; }
    
    // Friction
    float getFriction() const { return m_friction; }
    void setFriction(float friction) { m_friction = friction; }
    
    // Restitution (bounciness)
    float getRestitution() const { return m_restitution; }
    void setRestitution(float restitution) { m_restitution = restitution; }
    
private:
    float m_velocityX;    // X velocity
    float m_velocityY;    // Y velocity
    float m_mass;         // Mass
    bool m_solid;         // Solidity flag
    float m_gravityScale; // Gravity scale
    float m_friction;     // Friction coefficient
    float m_restitution;  // Restitution (bounciness)
};

} // namespace Components
} // namespace RPGEngine