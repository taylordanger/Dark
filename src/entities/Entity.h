#pragma once

#include <cstdint>
#include <string>
#include "../core/Types.h"

namespace RPGEngine {

/**
 * Entity ID type
 * Uses a 32-bit unsigned integer for entity IDs
 */
using EntityID = EntityId;

/**
 * Entity class
 * Represents a game object in the world
 * An entity is essentially just an ID with associated components
 */
class Entity {
public:
    /**
     * Default constructor
     * Creates an invalid entity
     */
    Entity() : m_id(INVALID_ENTITY_ID), m_active(false), m_name("") {}
    
    /**
     * Constructor with ID
     * @param id Entity ID
     * @param name Optional entity name
     */
    Entity(EntityID id, const std::string& name = "") 
        : m_id(id), m_active(true), m_name(name) {}
    
    /**
     * Get the entity ID
     * @return Entity ID
     */
    EntityID getID() const { return m_id; }
    
    /**
     * Check if the entity is valid
     * @return true if the entity has a valid ID
     */
    bool isValid() const { return m_id != INVALID_ENTITY_ID; }
    
    /**
     * Check if the entity is active
     * @return true if the entity is active
     */
    bool isActive() const { return m_active; }
    
    /**
     * Set the entity active state
     * @param active New active state
     */
    void setActive(bool active) { m_active = active; }
    
    /**
     * Get the entity name
     * @return Entity name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * Set the entity name
     * @param name New entity name
     */
    void setName(const std::string& name) { m_name = name; }
    
    /**
     * Equality operator
     * @param other Entity to compare with
     * @return true if entities have the same ID
     */
    bool operator==(const Entity& other) const { return m_id == other.m_id; }
    
    /**
     * Inequality operator
     * @param other Entity to compare with
     * @return true if entities have different IDs
     */
    bool operator!=(const Entity& other) const { return m_id != other.m_id; }
    
private:
    EntityID m_id;       // Unique entity identifier
    bool m_active;       // Entity active state
    std::string m_name;  // Optional entity name
};

} // namespace RPGEngine