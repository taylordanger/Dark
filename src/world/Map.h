#pragma once

#include "MapObject.h"
#include "../tilemap/Tilemap.h"
#include "../entities/Entity.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace RPGEngine {
namespace World {

/**
 * Map class
 * Represents a game map
 */
class Map {
public:
    /**
     * Constructor
     * @param id Map ID
     * @param name Map name
     * @param tilemap Tilemap
     */
    Map(uint32_t id, const std::string& name, std::shared_ptr<Tilemap::Tilemap> tilemap);
    
    /**
     * Destructor
     */
    ~Map();
    
    /**
     * Get the map ID
     * @return Map ID
     */
    uint32_t getId() const { return m_id; }
    
    /**
     * Get the map name
     * @return Map name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * Set the map name
     * @param name Map name
     */
    void setName(const std::string& name) { m_name = name; }
    
    /**
     * Get the tilemap
     * @return Tilemap
     */
    std::shared_ptr<Tilemap::Tilemap> getTilemap() const { return m_tilemap; }
    
    /**
     * Set the tilemap
     * @param tilemap Tilemap
     */
    void setTilemap(std::shared_ptr<Tilemap::Tilemap> tilemap) { m_tilemap = tilemap; }
    
    /**
     * Get the number of objects
     * @return Number of objects
     */
    size_t getObjectCount() const { return m_objects.size(); }
    
    /**
     * Get an object by index
     * @param index Object index
     * @return Object, or nullptr if index is out of bounds
     */
    std::shared_ptr<MapObject> getObject(size_t index) const;
    
    /**
     * Get an object by ID
     * @param id Object ID
     * @return Object, or nullptr if not found
     */
    std::shared_ptr<MapObject> getObjectById(uint32_t id) const;
    
    /**
     * Get an object by name
     * @param name Object name
     * @return Object, or nullptr if not found
     */
    std::shared_ptr<MapObject> getObjectByName(const std::string& name) const;
    
    /**
     * Get objects by type
     * @param type Object type
     * @return Vector of objects
     */
    std::vector<std::shared_ptr<MapObject>> getObjectsByType(const std::string& type) const;
    
    /**
     * Add an object
     * @param object Object to add
     * @return true if the object was added
     */
    bool addObject(std::shared_ptr<MapObject> object);
    
    /**
     * Remove an object by ID
     * @param id Object ID
     * @return true if the object was removed
     */
    bool removeObject(uint32_t id);
    
    /**
     * Get all objects
     * @return Vector of objects
     */
    const std::vector<std::shared_ptr<MapObject>>& getObjects() const { return m_objects; }
    
    /**
     * Get the number of entities
     * @return Number of entities
     */
    size_t getEntityCount() const { return m_entities.size(); }
    
    /**
     * Get an entity by index
     * @param index Entity index
     * @return Entity, or an invalid entity if index is out of bounds
     */
    Entity getEntity(size_t index) const;
    
    /**
     * Add an entity
     * @param entity Entity to add
     * @return true if the entity was added
     */
    bool addEntity(Entity entity);
    
    /**
     * Remove an entity
     * @param entity Entity to remove
     * @return true if the entity was removed
     */
    bool removeEntity(Entity entity);
    
    /**
     * Get all entities
     * @return Vector of entities
     */
    const std::vector<Entity>& getEntities() const { return m_entities; }
    
    /**
     * Get a property
     * @param name Property name
     * @param defaultValue Default value if property doesn't exist
     * @return Property value
     */
    std::string getProperty(const std::string& name, const std::string& defaultValue = "") const;
    
    /**
     * Set a property
     * @param name Property name
     * @param value Property value
     */
    void setProperty(const std::string& name, const std::string& value);
    
    /**
     * Check if a property exists
     * @param name Property name
     * @return true if the property exists
     */
    bool hasProperty(const std::string& name) const;
    
    /**
     * Get all properties
     * @return Map of properties
     */
    const std::unordered_map<std::string, std::string>& getProperties() const { return m_properties; }
    
private:
    uint32_t m_id;                                              // Map ID
    std::string m_name;                                         // Map name
    std::shared_ptr<Tilemap::Tilemap> m_tilemap;                // Tilemap
    std::vector<std::shared_ptr<MapObject>> m_objects;          // Map objects
    std::vector<Entity> m_entities;                             // Map entities
    std::unordered_map<std::string, std::string> m_properties;  // Map properties
};

} // namespace World
} // namespace RPGEngine