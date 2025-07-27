#include "Map.h"
#include <algorithm>

namespace RPGEngine {
namespace World {

Map::Map(uint32_t id, const std::string& name, std::shared_ptr<Tilemap::Tilemap> tilemap)
    : m_id(id)
    , m_name(name)
    , m_tilemap(tilemap)
{
}

Map::~Map() {
    // Nothing to do here
}

std::shared_ptr<MapObject> Map::getObject(size_t index) const {
    if (index >= m_objects.size()) {
        return nullptr;
    }
    
    return m_objects[index];
}

std::shared_ptr<MapObject> Map::getObjectById(uint32_t id) const {
    for (const auto& object : m_objects) {
        if (object->getId() == id) {
            return object;
        }
    }
    
    return nullptr;
}

std::shared_ptr<MapObject> Map::getObjectByName(const std::string& name) const {
    for (const auto& object : m_objects) {
        if (object->getName() == name) {
            return object;
        }
    }
    
    return nullptr;
}

std::vector<std::shared_ptr<MapObject>> Map::getObjectsByType(const std::string& type) const {
    std::vector<std::shared_ptr<MapObject>> result;
    
    for (const auto& object : m_objects) {
        if (object->getType() == type) {
            result.push_back(object);
        }
    }
    
    return result;
}

bool Map::addObject(std::shared_ptr<MapObject> object) {
    if (!object) {
        return false;
    }
    
    // Check if object with same ID already exists
    for (const auto& existingObject : m_objects) {
        if (existingObject->getId() == object->getId()) {
            return false;
        }
    }
    
    m_objects.push_back(object);
    return true;
}

bool Map::removeObject(uint32_t id) {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [id](const std::shared_ptr<MapObject>& object) {
            return object->getId() == id;
        });
    
    if (it != m_objects.end()) {
        m_objects.erase(it);
        return true;
    }
    
    return false;
}

Entity Map::getEntity(size_t index) const {
    if (index >= m_entities.size()) {
        return Entity();
    }
    
    return m_entities[index];
}

bool Map::addEntity(Entity entity) {
    if (!entity.isValid()) {
        return false;
    }
    
    // Check if entity already exists
    for (const auto& existingEntity : m_entities) {
        if (existingEntity.getID() == entity.getID()) {
            return false;
        }
    }
    
    m_entities.push_back(entity);
    return true;
}

bool Map::removeEntity(Entity entity) {
    if (!entity.isValid()) {
        return false;
    }
    
    auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [entity](const Entity& existingEntity) {
            return existingEntity.getID() == entity.getID();
        });
    
    if (it != m_entities.end()) {
        m_entities.erase(it);
        return true;
    }
    
    return false;
}

std::string Map::getProperty(const std::string& name, const std::string& defaultValue) const {
    auto it = m_properties.find(name);
    if (it != m_properties.end()) {
        return it->second;
    }
    return defaultValue;
}

void Map::setProperty(const std::string& name, const std::string& value) {
    m_properties[name] = value;
}

bool Map::hasProperty(const std::string& name) const {
    return m_properties.find(name) != m_properties.end();
}

} // namespace World
} // namespace RPGEngine