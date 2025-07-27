#pragma once

#include "Entity.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <queue>

namespace RPGEngine {

/**
 * Entity Manager
 * Responsible for creating, destroying, and managing entities
 */
class EntityManager {
public:
    /**
     * Constructor
     */
    EntityManager();
    
    /**
     * Destructor
     */
    ~EntityManager();
    
    /**
     * Initialize the entity manager
     * @return true if initialization was successful
     */
    bool initialize();
    
    /**
     * Shutdown the entity manager
     */
    void shutdown();
    
    /**
     * Update the entity manager
     * Processes pending entity operations
     */
    void update();
    
    /**
     * Create a new entity
     * @param name Optional entity name
     * @return Created entity
     */
    Entity createEntity(const std::string& name = "");
    
    /**
     * Destroy an entity
     * @param entity Entity to destroy
     * @return true if the entity was destroyed
     */
    bool destroyEntity(Entity entity);
    
    /**
     * Mark an entity for destruction
     * The entity will be destroyed during the next update
     * @param entity Entity to destroy
     */
    void destroyEntityDeferred(Entity entity);
    
    /**
     * Get an entity by ID
     * @param id Entity ID
     * @return Entity with the specified ID, or an invalid entity if not found
     */
    Entity getEntity(EntityID id) const;
    
    /**
     * Get an entity by name
     * @param name Entity name
     * @return First entity with the specified name, or an invalid entity if not found
     */
    Entity getEntityByName(const std::string& name) const;
    
    /**
     * Check if an entity exists
     * @param id Entity ID
     * @return true if the entity exists
     */
    bool entityExists(EntityID id) const;
    
    /**
     * Set an entity's active state
     * @param entity Entity to modify
     * @param active New active state
     * @return true if the entity's state was changed
     */
    bool setEntityActive(Entity entity, bool active);
    
    /**
     * Get all entities
     * @return Vector of all entities
     */
    std::vector<Entity> getAllEntities() const;
    
    /**
     * Get all active entities
     * @return Vector of all active entities
     */
    std::vector<Entity> getActiveEntities() const;
    
    /**
     * Get the number of entities
     * @return Total number of entities
     */
    size_t getEntityCount() const;
    
    /**
     * Get the number of active entities
     * @return Number of active entities
     */
    size_t getActiveEntityCount() const;
    
    /**
     * Clear all entities
     */
    void clearAllEntities();
    
    /**
     * Iterate over all entities and execute a function for each
     * @param func Function to execute for each entity
     */
    void forEachEntity(const std::function<void(Entity)>& func) const;
    
    /**
     * Iterate over all active entities and execute a function for each
     * @param func Function to execute for each active entity
     */
    void forEachActiveEntity(const std::function<void(Entity)>& func) const;
    
private:
    /**
     * Generate a new unique entity ID
     * @return New entity ID
     */
    EntityID generateEntityID();
    
    /**
     * Process deferred entity operations
     */
    void processDeferredOperations();
    
    // Entity storage
    std::unordered_map<EntityID, Entity> m_entities;
    std::unordered_map<std::string, EntityID> m_entityNames;
    
    // Entity ID generation
    EntityID m_nextEntityID;
    std::queue<EntityID> m_reusableIDs;
    
    // Deferred operations
    std::vector<EntityID> m_entitiesToDestroy;
    
    // Statistics
    size_t m_activeEntityCount;
    
    // Initialization state
    bool m_initialized;
};

} // namespace RPGEngine