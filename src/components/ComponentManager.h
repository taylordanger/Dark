#pragma once

#include "Component.h"
#include "../entities/Entity.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <typeindex>
#include <functional>
#include <iostream>

namespace RPGEngine {

/**
 * Component Manager
 * Manages components for entities in the ECS system
 */
class ComponentManager {
public:
    /**
     * Constructor
     */
    ComponentManager();
    
    /**
     * Destructor
     */
    ~ComponentManager();
    
    /**
     * Initialize the component manager
     * @return true if initialization was successful
     */
    bool initialize();
    
    /**
     * Shutdown the component manager
     */
    void shutdown();
    
    /**
     * Add a component to an entity
     * @param entity Entity to add the component to
     * @param component Component to add
     * @return true if the component was added successfully
     */
    template<typename T>
    bool addComponent(Entity entity, std::shared_ptr<T> component);
    
    /**
     * Create and add a component to an entity
     * @param entity Entity to add the component to
     * @param args Arguments to pass to the component constructor
     * @return Shared pointer to the created component
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> createComponent(Entity entity, Args&&... args);
    
    /**
     * Remove a component from an entity
     * @param entity Entity to remove the component from
     * @return true if the component was removed
     */
    template<typename T>
    bool removeComponent(Entity entity);
    
    /**
     * Check if an entity has a component
     * @param entity Entity to check
     * @return true if the entity has the component
     */
    template<typename T>
    bool hasComponent(Entity entity) const;
    
    /**
     * Get a component from an entity
     * @param entity Entity to get the component from
     * @return Shared pointer to the component, or nullptr if not found
     */
    template<typename T>
    std::shared_ptr<T> getComponent(Entity entity) const;
    
    /**
     * Get all components of a specific type
     * @return Vector of components
     */
    template<typename T>
    std::vector<std::shared_ptr<T>> getAllComponents() const;
    
    /**
     * Get all entities that have a specific component
     * @return Vector of entities
     */
    template<typename T>
    std::vector<Entity> getEntitiesWithComponent() const;
    
    /**
     * Get all entities that have all of the specified components
     * @return Vector of entities
     */
    template<typename T, typename U, typename... Rest>
    std::vector<Entity> getEntitiesWithComponents() const;
    
    /**
     * Remove all components from an entity
     * @param entity Entity to remove components from
     */
    void removeAllComponents(Entity entity);
    
    /**
     * Clear all components
     */
    void clearAllComponents();
    
    /**
     * Get the number of components of a specific type
     * @return Number of components
     */
    template<typename T>
    size_t getComponentCount() const;
    
    /**
     * Iterate over all components of a specific type
     * @param func Function to execute for each component
     */
    template<typename T>
    void forEachComponent(const std::function<void(Entity, std::shared_ptr<T>)>& func) const;
    
private:
    // Helper template for variadic template recursion
    template<typename T>
    bool hasAllComponents(Entity entity) const;
    
    template<typename T, typename U, typename... Rest>
    bool hasAllComponents(Entity entity) const;
    
    // Component storage
    // Maps component type -> (entity ID -> component)
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::shared_ptr<Components::IComponent>>> m_components;
    
    // Initialization state
    bool m_initialized;
};

// Template implementation

template<typename T>
bool ComponentManager::addComponent(Entity entity, std::shared_ptr<T> component) {
    if (!m_initialized || !entity.isValid() || !component) {
        return false;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if entity already has this component type
    if (m_components[type].find(entity.getID()) != m_components[type].end()) {
        std::cerr << "Entity " << entity.getID() << " already has component of type " 
                  << Components::ComponentTypeRegistry::getComponentName<T>() << std::endl;
        return false;
    }
    
    // Add component
    m_components[type][entity.getID()] = component;
    return true;
}

template<typename T, typename... Args>
std::shared_ptr<T> ComponentManager::createComponent(Entity entity, Args&&... args) {
    if (!m_initialized || !entity.isValid()) {
        return nullptr;
    }
    
    // Create component
    std::shared_ptr<T> component = std::make_shared<T>(entity.getID(), std::forward<Args>(args)...);
    
    // Add component to entity
    if (!addComponent<T>(entity, component)) {
        return nullptr;
    }
    
    return component;
}

template<typename T>
bool ComponentManager::removeComponent(Entity entity) {
    if (!m_initialized || !entity.isValid()) {
        return false;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return false;
    }
    
    auto& entityMap = typeIt->second;
    auto entityIt = entityMap.find(entity.getID());
    if (entityIt == entityMap.end()) {
        return false;
    }
    
    // Remove component
    entityMap.erase(entityIt);
    
    // Remove empty component type map
    if (entityMap.empty()) {
        m_components.erase(typeIt);
    }
    
    return true;
}

template<typename T>
bool ComponentManager::hasComponent(Entity entity) const {
    if (!m_initialized || !entity.isValid()) {
        return false;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component type exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return false;
    }
    
    // Check if entity has component
    return typeIt->second.find(entity.getID()) != typeIt->second.end();
}

template<typename T>
std::shared_ptr<T> ComponentManager::getComponent(Entity entity) const {
    if (!m_initialized || !entity.isValid()) {
        return nullptr;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component type exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return nullptr;
    }
    
    // Check if entity has component
    auto entityIt = typeIt->second.find(entity.getID());
    if (entityIt == typeIt->second.end()) {
        return nullptr;
    }
    
    // Return component
    return std::static_pointer_cast<T>(entityIt->second);
}

template<typename T>
std::vector<std::shared_ptr<T>> ComponentManager::getAllComponents() const {
    std::vector<std::shared_ptr<T>> result;
    
    if (!m_initialized) {
        return result;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component type exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return result;
    }
    
    // Get all components of this type
    result.reserve(typeIt->second.size());
    for (const auto& pair : typeIt->second) {
        result.push_back(std::static_pointer_cast<T>(pair.second));
    }
    
    return result;
}

template<typename T>
std::vector<Entity> ComponentManager::getEntitiesWithComponent() const {
    std::vector<Entity> result;
    
    if (!m_initialized) {
        return result;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component type exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return result;
    }
    
    // Get all entities with this component
    result.reserve(typeIt->second.size());
    for (const auto& pair : typeIt->second) {
        result.push_back(Entity(pair.first));
    }
    
    return result;
}

template<typename T, typename U, typename... Rest>
std::vector<Entity> ComponentManager::getEntitiesWithComponents() const {
    // Get entities with first component type
    std::vector<Entity> result = getEntitiesWithComponent<T>();
    
    if (result.empty()) {
        return result;
    }
    
    // Filter entities that don't have all required components
    auto it = result.begin();
    while (it != result.end()) {
        if (!hasComponent<U>(*it) || !hasAllComponents<Rest...>(*it)) {
            it = result.erase(it);
        } else {
            ++it;
        }
    }
    
    return result;
}

template<typename T>
size_t ComponentManager::getComponentCount() const {
    if (!m_initialized) {
        return 0;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component type exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return 0;
    }
    
    return typeIt->second.size();
}

template<typename T>
void ComponentManager::forEachComponent(const std::function<void(Entity, std::shared_ptr<T>)>& func) const {
    if (!m_initialized || !func) {
        return;
    }
    
    std::type_index type = std::type_index(typeid(T));
    
    // Check if component type exists
    auto typeIt = m_components.find(type);
    if (typeIt == m_components.end()) {
        return;
    }
    
    // Execute function for each component
    for (const auto& pair : typeIt->second) {
        func(Entity(pair.first), std::static_pointer_cast<T>(pair.second));
    }
}

template<typename T>
bool ComponentManager::hasAllComponents(Entity entity) const {
    return hasComponent<T>(entity);
}

template<typename T, typename U, typename... Rest>
bool ComponentManager::hasAllComponents(Entity entity) const {
    return hasComponent<T>(entity) && hasAllComponents<U, Rest...>(entity);
}

} // namespace RPGEngine