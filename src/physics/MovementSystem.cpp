#include "MovementSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace RPGEngine {
namespace Physics {

MovementSystem::MovementSystem(std::shared_ptr<ComponentManager> componentManager, 
                             std::shared_ptr<CollisionSystem> collisionSystem)
    : System("MovementSystem")
    , m_componentManager(componentManager)
    , m_collisionSystem(collisionSystem)
    , m_gravity(0.0f, 9.8f)
    , m_velocityDamping(0.98f)
    , m_angularVelocityDamping(0.98f)
    , m_maxVelocity(1000.0f)
    , m_maxAngularVelocity(10.0f)
    , m_collisionResponseEnabled(true)
    , m_collisionIterations(3)
{
}

MovementSystem::~MovementSystem() {
    if (isInitialized()) {
        shutdown();
    }
}

bool MovementSystem::onInitialize() {
    if (!m_componentManager) {
        std::cerr << "Component manager not provided to MovementSystem" << std::endl;
        return false;
    }
    
    if (!m_collisionSystem) {
        std::cerr << "Collision system not provided to MovementSystem" << std::endl;
        return false;
    }
    
    std::cout << "MovementSystem initialized" << std::endl;
    return true;
}

void MovementSystem::onUpdate(float deltaTime) {
    // Get all entities with physics components
    auto entities = m_componentManager->getEntitiesWithComponent<PhysicsComponent>();
    
    // First pass: update physics for all entities
    for (auto entity : entities) {
        auto physicsComponent = m_componentManager->getComponent<PhysicsComponent>(entity);
        if (physicsComponent) {
            updatePhysics(entity, physicsComponent, deltaTime);
        }
    }
    
    // Second pass: resolve collisions
    if (m_collisionResponseEnabled) {
        // Perform multiple iterations for better collision resolution
        for (int i = 0; i < m_collisionIterations; ++i) {
            // Update collidables
            for (auto entity : entities) {
                auto physicsComponent = m_componentManager->getComponent<PhysicsComponent>(entity);
                if (physicsComponent && physicsComponent->getCollisionShape()) {
                    // Get or create collidable
                    auto it = m_collidables.find(entity.getID());
                    if (it == m_collidables.end()) {
                        auto collidable = std::make_shared<PhysicsCollidable>(entity, physicsComponent);
                        m_collidables[entity.getID()] = collidable;
                        m_collisionSystem->registerCollidable(collidable);
                    } else {
                        // Update collidable
                        updateCollisionShape(physicsComponent);
                        m_collisionSystem->updateCollidable(it->second);
                    }
                    
                    // Resolve collisions
                    resolveCollisions(entity, physicsComponent, m_collidables[entity.getID()]);
                }
            }
        }
    }
    
    // Update collision shapes for rendering
    for (auto entity : entities) {
        auto physicsComponent = m_componentManager->getComponent<PhysicsComponent>(entity);
        if (physicsComponent && physicsComponent->getCollisionShape()) {
            updateCollisionShape(physicsComponent);
        }
    }
}

void MovementSystem::onShutdown() {
    // Unregister all collidables
    for (auto& pair : m_collidables) {
        m_collisionSystem->unregisterCollidable(pair.second);
    }
    
    m_collidables.clear();
    
    std::cout << "MovementSystem shutdown" << std::endl;
}

void MovementSystem::updatePhysics(Entity entity, std::shared_ptr<PhysicsComponent> physicsComponent, float deltaTime) {
    // Skip static bodies
    if (physicsComponent->isStatic()) {
        return;
    }
    
    // Apply gravity
    if (physicsComponent->getGravityScale() != 0.0f) {
        physicsComponent->applyForce(m_gravity * physicsComponent->getGravityScale());
    }
    
    // Update velocity based on acceleration
    Vector2 velocity = physicsComponent->getVelocity();
    velocity = velocity + physicsComponent->getAcceleration() * deltaTime;
    
    // Apply damping
    velocity = velocity * std::pow(m_velocityDamping, deltaTime);
    
    // Limit velocity
    float speedSquared = velocity.lengthSquared();
    if (speedSquared > m_maxVelocity * m_maxVelocity) {
        velocity = velocity * (m_maxVelocity / std::sqrt(speedSquared));
    }
    
    // Update position based on velocity
    Vector2 position = physicsComponent->getPosition();
    position = position + velocity * deltaTime;
    
    // Update angular velocity
    float angularVelocity = physicsComponent->getAngularVelocity();
    angularVelocity = angularVelocity * std::pow(m_angularVelocityDamping, deltaTime);
    
    // Limit angular velocity
    if (std::abs(angularVelocity) > m_maxAngularVelocity) {
        angularVelocity = std::copysign(m_maxAngularVelocity, angularVelocity);
    }
    
    // Update rotation based on angular velocity
    float rotation = physicsComponent->getRotation();
    rotation = rotation + angularVelocity * deltaTime;
    
    // Update physics component
    physicsComponent->setPosition(position);
    physicsComponent->setVelocity(velocity);
    physicsComponent->setAcceleration(Vector2(0.0f, 0.0f)); // Reset acceleration
    physicsComponent->setRotation(rotation);
    physicsComponent->setAngularVelocity(angularVelocity);
    
    // Update collision shape
    updateCollisionShape(physicsComponent);
}

void MovementSystem::resolveCollisions(Entity entity, std::shared_ptr<PhysicsComponent> physicsComponent, 
                                     std::shared_ptr<PhysicsCollidable> collidable) {
    // Skip static bodies and triggers
    if (physicsComponent->isStatic() || physicsComponent->isTrigger()) {
        return;
    }
    
    // Get potential collisions
    auto potentialCollisions = m_collisionSystem->getPotentialCollisions(collidable);
    
    for (auto& otherCollidable : potentialCollisions) {
        // Skip self-collision
        if (otherCollidable->getCollidableID() == collidable->getCollidableID()) {
            continue;
        }
        
        // Check if the other collidable is a PhysicsCollidable
        auto otherPhysicsCollidable = std::dynamic_pointer_cast<PhysicsCollidable>(otherCollidable);
        if (!otherPhysicsCollidable) {
            continue;
        }
        
        // Get the other entity and physics component
        Entity otherEntity = otherPhysicsCollidable->getEntity();
        auto otherPhysicsComponent = otherPhysicsCollidable->getPhysicsComponent();
        
        // Skip if the other entity is a trigger
        if (otherPhysicsComponent->isTrigger()) {
            continue;
        }
        
        // Check for collision
        CollisionResult result;
        if (m_collisionSystem->checkCollision(collidable->getCollisionShape(), otherCollidable->getCollisionShape(), &result)) {
            // Resolve collision
            resolveCollision(entity, physicsComponent, otherEntity, otherPhysicsComponent, result);
        }
    }
}

void MovementSystem::resolveCollision(Entity entity1, std::shared_ptr<PhysicsComponent> physics1,
                                    Entity entity2, std::shared_ptr<PhysicsComponent> physics2,
                                    const CollisionResult& collisionResult) {
    // Calculate masses
    float mass1 = physics1->getMass();
    float mass2 = physics2->getMass();
    
    // Handle static bodies
    bool isStatic1 = physics1->isStatic();
    bool isStatic2 = physics2->isStatic();
    
    if (isStatic1 && isStatic2) {
        return; // Both static, no resolution needed
    }
    
    // Calculate mass ratios
    float totalMass = mass1 + mass2;
    float massRatio1 = isStatic1 ? 0.0f : (isStatic2 ? 1.0f : mass2 / totalMass);
    float massRatio2 = isStatic2 ? 0.0f : (isStatic1 ? 1.0f : mass1 / totalMass);
    
    // Resolve penetration
    Vector2 separation = collisionResult.normal * collisionResult.penetration;
    
    if (!isStatic1) {
        Vector2 pos1 = physics1->getPosition();
        pos1 = pos1 - separation * massRatio1;
        physics1->setPosition(pos1);
    }
    
    if (!isStatic2) {
        Vector2 pos2 = physics2->getPosition();
        pos2 = pos2 + separation * massRatio2;
        physics2->setPosition(pos2);
    }
    
    // Update collision shapes
    updateCollisionShape(physics1);
    updateCollisionShape(physics2);
    
    // Calculate relative velocity
    Vector2 relativeVelocity = physics2->getVelocity() - physics1->getVelocity();
    
    // Calculate velocity along the normal
    float velocityAlongNormal = relativeVelocity.dot(collisionResult.normal);
    
    // Only resolve if the objects are moving toward each other
    if (velocityAlongNormal > 0.0f) {
        return;
    }
    
    // Calculate restitution (bounce)
    float restitution = std::min(physics1->getRestitution(), physics2->getRestitution());
    
    // Calculate impulse scalar
    float j = -(1.0f + restitution) * velocityAlongNormal;
    j /= (isStatic1 ? 0.0f : 1.0f / mass1) + (isStatic2 ? 0.0f : 1.0f / mass2);
    
    // Apply impulse
    Vector2 impulse = collisionResult.normal * j;
    
    if (!isStatic1) {
        Vector2 vel1 = physics1->getVelocity();
        vel1 = vel1 - impulse * (1.0f / mass1);
        physics1->setVelocity(vel1);
    }
    
    if (!isStatic2) {
        Vector2 vel2 = physics2->getVelocity();
        vel2 = vel2 + impulse * (1.0f / mass2);
        physics2->setVelocity(vel2);
    }
    
    // Apply friction
    float friction = std::sqrt(physics1->getFriction() * physics2->getFriction());
    
    if (friction > 0.0f) {
        // Calculate tangent vector
        Vector2 tangent = relativeVelocity - collisionResult.normal * velocityAlongNormal;
        float tangentLength = tangent.length();
        
        if (tangentLength > 0.0001f) {
            tangent = tangent / tangentLength;
            
            // Calculate friction impulse scalar
            float jt = -relativeVelocity.dot(tangent);
            jt /= (isStatic1 ? 0.0f : 1.0f / mass1) + (isStatic2 ? 0.0f : 1.0f / mass2);
            
            // Clamp friction impulse
            float maxJt = friction * j;
            jt = std::max(-maxJt, std::min(jt, maxJt));
            
            // Apply friction impulse
            Vector2 frictionImpulse = tangent * jt;
            
            if (!isStatic1) {
                Vector2 vel1 = physics1->getVelocity();
                vel1 = vel1 - frictionImpulse * (1.0f / mass1);
                physics1->setVelocity(vel1);
            }
            
            if (!isStatic2) {
                Vector2 vel2 = physics2->getVelocity();
                vel2 = vel2 + frictionImpulse * (1.0f / mass2);
                physics2->setVelocity(vel2);
            }
        }
    }
}

void MovementSystem::updateCollisionShape(std::shared_ptr<PhysicsComponent> physicsComponent) {
    auto shape = physicsComponent->getCollisionShape();
    if (shape) {
        shape->setPosition(physicsComponent->getPosition());
        shape->setRotation(physicsComponent->getRotation());
    }
}

} // namespace Physics
} // namespace RPGEngine