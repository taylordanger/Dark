#include "EntityManager.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {

EntityManager::EntityManager()
    : m_nextEntityID(1)  // Start from 1, as 0 is INVALID_ENTITY_ID
    , m_activeEntityCount(0)
    , m_initialized(false)
{
}

EntityManager::~EntityManager() {
    if (m_initialized) {
        shutdown();
    }
}

bool EntityManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Clear any existing data
    m_entities.clear();
    m_entityNames.clear();
    m_reusableIDs = std::queue<EntityID>();
    m_entitiesToDestroy.clear();
    
    m_nextEntityID = 1;
    m_activeEntityCount = 0;
    m_initialized = true;
    
    std::cout << "EntityManager initialized" << std::endl;
    return true;
}

void EntityManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Process any pending operations
    processDeferredOperations();
    
    // Clear all entities
    clearAllEntities();
    
    m_initialized = false;
    std::cout << "EntityManager shutdown" << std::endl;
}

void EntityManager::update() {
    if (!m_initialized) {
        return;
    }
    
    // Process deferred operations
    processDeferredOperations();
}

Entity EntityManager::createEntity(const std::string& name) {
    if (!m_initialized) {
        std::cerr << "EntityManager not initialized" << std::endl;
        return Entity();
    }
    
    // Generate a new entity ID
    EntityID id = generateEntityID();
    
    // Create the entity
    Entity entity(id, name);
    
    // Store the entity
    m_entities[id] = entity;
    
    // Store the entity name if provided
    if (!name.empty()) {
        m_entityNames[name] = id;
    }
    
    // Update statistics
    m_activeEntityCount++;
    
    return entity;
}

bool EntityManager::destroyEntity(Entity entity) {
    if (!m_initialized || !entity.isValid() || !entityExists(entity.getID())) {
        return false;
    }
    
    EntityID id = entity.getID();
    
    // Remove from name map if it has a name
    const std::string& name = entity.getName();
    if (!name.empty()) {
        m_entityNames.erase(name);
    }
    
    // Remove from entity map
    m_entities.erase(id);
    
    // Add ID to reusable pool
    m_reusableIDs.push(id);
    
    // Update statistics
    if (entity.isActive()) {
        m_activeEntityCount--;
    }
    
    return true;
}

void EntityManager::destroyEntityDeferred(Entity entity) {
    if (!m_initialized || !entity.isValid() || !entityExists(entity.getID())) {
        return;
    }
    
    // Add to deferred destruction list
    m_entitiesToDestroy.push_back(entity.getID());
}

Entity EntityManager::getEntity(EntityID id) const {
    if (!m_initialized || id == INVALID_ENTITY_ID) {
        return Entity();
    }
    
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second;
    }
    
    return Entity();
}

Entity EntityManager::getEntityByName(const std::string& name) const {
    if (!m_initialized || name.empty()) {
        return Entity();
    }
    
    auto it = m_entityNames.find(name);
    if (it != m_entityNames.end()) {
        return getEntity(it->second);
    }
    
    return Entity();
}

bool EntityManager::entityExists(EntityID id) const {
    if (!m_initialized || id == INVALID_ENTITY_ID) {
        return false;
    }
    
    return m_entities.find(id) != m_entities.end();
}

bool EntityManager::setEntityActive(Entity entity, bool active) {
    if (!m_initialized || !entity.isValid() || !entityExists(entity.getID())) {
        return false;
    }
    
    EntityID id = entity.getID();
    bool wasActive = m_entities[id].isActive();
    
    // Update active state
    m_entities[id].setActive(active);
    
    // Update statistics
    if (wasActive && !active) {
        m_activeEntityCount--;
    } else if (!wasActive && active) {
        m_activeEntityCount++;
    }
    
    return true;
}

std::vector<Entity> EntityManager::getAllEntities() const {
    std::vector<Entity> result;
    
    if (!m_initialized) {
        return result;
    }
    
    result.reserve(m_entities.size());
    
    for (const auto& pair : m_entities) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::vector<Entity> EntityManager::getActiveEntities() const {
    std::vector<Entity> result;
    
    if (!m_initialized) {
        return result;
    }
    
    result.reserve(m_activeEntityCount);
    
    for (const auto& pair : m_entities) {
        if (pair.second.isActive()) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

size_t EntityManager::getEntityCount() const {
    return m_initialized ? m_entities.size() : 0;
}

size_t EntityManager::getActiveEntityCount() const {
    return m_initialized ? m_activeEntityCount : 0;
}

void EntityManager::clearAllEntities() {
    if (!m_initialized) {
        return;
    }
    
    m_entities.clear();
    m_entityNames.clear();
    m_reusableIDs = std::queue<EntityID>();
    m_entitiesToDestroy.clear();
    
    m_nextEntityID = 1;
    m_activeEntityCount = 0;
}

void EntityManager::forEachEntity(const std::function<void(Entity)>& func) const {
    if (!m_initialized || !func) {
        return;
    }
    
    for (const auto& pair : m_entities) {
        func(pair.second);
    }
}

void EntityManager::forEachActiveEntity(const std::function<void(Entity)>& func) const {
    if (!m_initialized || !func) {
        return;
    }
    
    for (const auto& pair : m_entities) {
        if (pair.second.isActive()) {
            func(pair.second);
        }
    }
}

EntityID EntityManager::generateEntityID() {
    EntityID id;
    
    // Reuse IDs if available
    if (!m_reusableIDs.empty()) {
        id = m_reusableIDs.front();
        m_reusableIDs.pop();
    } else {
        // Generate a new ID
        id = m_nextEntityID++;
    }
    
    return id;
}

void EntityManager::processDeferredOperations() {
    // Process entity destruction
    for (EntityID id : m_entitiesToDestroy) {
        destroyEntity(getEntity(id));
    }
    
    m_entitiesToDestroy.clear();
}

} // namespace RPGEngine