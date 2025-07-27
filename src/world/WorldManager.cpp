#include "WorldManager.h"
#include "../physics/PhysicsComponent.h"
#include "../physics/TriggerComponent.h"
#include <iostream>
#include <filesystem>

namespace RPGEngine {
namespace World {

WorldManager::WorldManager(std::shared_ptr<Resources::ResourceManager> resourceManager,
                         std::shared_ptr<EntityManager> entityManager,
                         std::shared_ptr<ComponentManager> componentManager)
    : System("WorldManager")
    , m_resourceManager(resourceManager)
    , m_entityManager(entityManager)
    , m_componentManager(componentManager)
    , m_mapDirectory("assets/maps/")
    , m_isTransitioning(false)
    , m_transitionTime(0.0f)
    , m_transitionDuration(1.0f)
    , m_transitionFromMapId(0)
    , m_transitionToMapId(0)
    , m_nextCallbackId(1)
{
    // Create map loader
    m_mapLoader = std::make_shared<Tilemap::MapLoader>(resourceManager);
}

WorldManager::~WorldManager() {
    if (isInitialized()) {
        shutdown();
    }
}

bool WorldManager::onInitialize() {
    if (!m_resourceManager) {
        std::cerr << "Resource manager not provided to WorldManager" << std::endl;
        return false;
    }
    
    if (!m_entityManager) {
        std::cerr << "Entity manager not provided to WorldManager" << std::endl;
        return false;
    }
    
    if (!m_componentManager) {
        std::cerr << "Component manager not provided to WorldManager" << std::endl;
        return false;
    }
    
    std::cout << "WorldManager initialized" << std::endl;
    return true;
}

void WorldManager::onUpdate(float deltaTime) {
    // Update map transition
    if (m_isTransitioning) {
        updateTransition(deltaTime);
    }
}

void WorldManager::onShutdown() {
    // Unload all maps
    m_maps.clear();
    m_activeMap = nullptr;
    
    // Clear callbacks
    m_transitionCallbacks.clear();
    m_mapLoadedCallbacks.clear();
    m_mapUnloadedCallbacks.clear();
    
    std::cout << "WorldManager shutdown" << std::endl;
}

std::shared_ptr<Map> WorldManager::loadMap(const std::string& filename, uint32_t id) {
    // Generate map ID if not provided
    if (id == 0) {
        id = 1;
        while (m_maps.find(id) != m_maps.end()) {
            id++;
        }
    } else if (m_maps.find(id) != m_maps.end()) {
        std::cerr << "Map with ID " << id << " already exists" << std::endl;
        return nullptr;
    }
    
    // Load tilemap
    std::string mapPath = m_mapDirectory + filename;
    auto tilemap = m_mapLoader->loadMap(mapPath);
    if (!tilemap) {
        std::cerr << "Failed to load map: " << mapPath << std::endl;
        return nullptr;
    }
    
    // Create map
    std::string mapName = std::filesystem::path(filename).stem().string();
    auto map = std::make_shared<Map>(id, mapName, tilemap);
    
    // Add map to maps
    m_maps[id] = map;
    
    // Create entities from map objects
    createEntitiesFromObjects(map);
    
    // Fire map loaded event
    MapLoadedEvent event(id, mapName);
    for (const auto& pair : m_mapLoadedCallbacks) {
        pair.second(event);
    }
    
    std::cout << "Loaded map: " << mapName << " (ID: " << id << ")" << std::endl;
    
    return map;
}

bool WorldManager::unloadMap(uint32_t id) {
    auto it = m_maps.find(id);
    if (it == m_maps.end()) {
        return false;
    }
    
    // Check if this is the active map
    if (m_activeMap && m_activeMap->getId() == id) {
        m_activeMap = nullptr;
    }
    
    // Get map name
    std::string mapName = it->second->getName();
    
    // Remove map
    m_maps.erase(it);
    
    // Fire map unloaded event
    MapUnloadedEvent event(id, mapName);
    for (const auto& pair : m_mapUnloadedCallbacks) {
        pair.second(event);
    }
    
    std::cout << "Unloaded map: " << mapName << " (ID: " << id << ")" << std::endl;
    
    return true;
}

std::shared_ptr<Map> WorldManager::getMap(uint32_t id) const {
    auto it = m_maps.find(id);
    if (it != m_maps.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::shared_ptr<Map> WorldManager::getMapByName(const std::string& name) const {
    for (const auto& pair : m_maps) {
        if (pair.second->getName() == name) {
            return pair.second;
        }
    }
    
    return nullptr;
}

bool WorldManager::setActiveMap(uint32_t id) {
    auto map = getMap(id);
    if (!map) {
        return false;
    }
    
    m_activeMap = map;
    
    // Update camera position
    if (m_camera && m_activeMap->getTilemap()) {
        const auto& mapProps = m_activeMap->getTilemap()->getProperties();
        m_camera->setPosition(mapProps.width * mapProps.tileWidth / 2.0f, mapProps.height * mapProps.tileHeight / 2.0f);
    }
    
    std::cout << "Active map set to: " << m_activeMap->getName() << " (ID: " << id << ")" << std::endl;
    
    return true;
}

bool WorldManager::transitionToMap(uint32_t toMapId, const std::string& portalName, float fadeTime) {
    // Check if already transitioning
    if (m_isTransitioning) {
        return false;
    }
    
    // Check if active map exists
    if (!m_activeMap) {
        // No active map, just set the new map as active
        return setActiveMap(toMapId);
    }
    
    // Check if destination map exists
    auto toMap = getMap(toMapId);
    if (!toMap) {
        return false;
    }
    
    // Start transition
    m_isTransitioning = true;
    m_transitionTime = 0.0f;
    m_transitionDuration = fadeTime;
    m_transitionFromMapId = m_activeMap->getId();
    m_transitionToMapId = toMapId;
    m_transitionPortalName = portalName;
    
    std::cout << "Starting map transition from " << m_activeMap->getName() << " to " << toMap->getName() << std::endl;
    
    return true;
}

int WorldManager::registerTransitionCallback(const std::function<void(const MapTransitionEvent&)>& callback) {
    if (!callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_transitionCallbacks[callbackId] = callback;
    return callbackId;
}

bool WorldManager::unregisterTransitionCallback(int callbackId) {
    auto it = m_transitionCallbacks.find(callbackId);
    if (it != m_transitionCallbacks.end()) {
        m_transitionCallbacks.erase(it);
        return true;
    }
    
    return false;
}

int WorldManager::registerMapLoadedCallback(const std::function<void(const MapLoadedEvent&)>& callback) {
    if (!callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_mapLoadedCallbacks[callbackId] = callback;
    return callbackId;
}

bool WorldManager::unregisterMapLoadedCallback(int callbackId) {
    auto it = m_mapLoadedCallbacks.find(callbackId);
    if (it != m_mapLoadedCallbacks.end()) {
        m_mapLoadedCallbacks.erase(it);
        return true;
    }
    
    return false;
}

int WorldManager::registerMapUnloadedCallback(const std::function<void(const MapUnloadedEvent&)>& callback) {
    if (!callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_mapUnloadedCallbacks[callbackId] = callback;
    return callbackId;
}

bool WorldManager::unregisterMapUnloadedCallback(int callbackId) {
    auto it = m_mapUnloadedCallbacks.find(callbackId);
    if (it != m_mapUnloadedCallbacks.end()) {
        m_mapUnloadedCallbacks.erase(it);
        return true;
    }
    
    return false;
}

void WorldManager::createEntitiesFromObjects(std::shared_ptr<Map> map) {
    // TODO: Create entities from map objects
    // This would typically involve:
    // 1. Parsing object layers from the tilemap
    // 2. Creating MapObject instances for each object
    // 3. Creating entities based on object types
    // 4. Adding components to entities based on object properties
    
    // For now, we'll just create a simple placeholder implementation
    
    // Get tilemap
    auto tilemap = map->getTilemap();
    if (!tilemap) {
        return;
    }
    
    // Create some example objects
    uint32_t objectId = 1;
    
    // Create a spawn point
    auto spawnPoint = std::make_shared<MapObject>(objectId++, "spawn", "spawn_point", 100.0f, 100.0f, 32.0f, 32.0f);
    map->addObject(spawnPoint);
    
    // Create a portal
    auto portal = std::make_shared<MapObject>(objectId++, "portal", "portal", 200.0f, 200.0f, 32.0f, 32.0f);
    portal->setProperty("target_map", "2");
    portal->setProperty("target_portal", "spawn");
    map->addObject(portal);
    
    // Create a trigger
    auto trigger = std::make_shared<MapObject>(objectId++, "trigger", "trigger", 300.0f, 300.0f, 64.0f, 64.0f);
    trigger->setProperty("event", "chest_open");
    map->addObject(trigger);
    
    // Create entities for objects
    for (const auto& object : map->getObjects()) {
        // Create entity
        Entity entity = m_entityManager->createEntity();
        
        // Add physics component
        auto physicsComponent = std::make_shared<Physics::PhysicsComponent>();
        physicsComponent->setPosition(object->getX() + object->getWidth() / 2.0f, object->getY() + object->getHeight() / 2.0f);
        
        // Create collision shape based on object type
        if (object->getType() == "portal" || object->getType() == "trigger") {
            // Create rectangle shape
            auto shape = std::make_shared<Physics::RectangleShape>(object->getWidth(), object->getHeight());
            shape->setPosition(object->getX() + object->getWidth() / 2.0f, object->getY() + object->getHeight() / 2.0f);
            shape->setRotation(object->getRotation());
            
            physicsComponent->setCollisionShape(shape);
            physicsComponent->setTrigger(true);
            
            // Add trigger component for triggers
            if (object->getType() == "trigger") {
                auto triggerComponent = std::make_shared<Physics::TriggerComponent>();
                triggerComponent->setTag(object->getName());
                
                // Add callback for trigger events
                triggerComponent->addCallback(Physics::TriggerEventType::Enter, [object](const Physics::TriggerEvent& event) {
                    std::cout << "Trigger entered: " << object->getName() << std::endl;
                    
                    // Handle trigger event based on properties
                    if (object->hasProperty("event")) {
                        std::cout << "Trigger event: " << object->getProperty("event") << std::endl;
                    }
                });
                
                m_componentManager->addComponent(entity, triggerComponent);
            }
        } else {
            // Create point shape for spawn points
            auto shape = std::make_shared<Physics::PointShape>();
            shape->setPosition(object->getX(), object->getY());
            
            physicsComponent->setCollisionShape(shape);
        }
        
        // Add component to entity
        m_componentManager->addComponent(entity, physicsComponent);
        
        // Add entity to map
        map->addEntity(entity);
    }
}

bool WorldManager::findSpawnPoint(std::shared_ptr<Map> map, const std::string& portalName, Physics::Vector2& position) {
    // Find spawn point by name
    auto spawnPoint = map->getObjectByName(portalName);
    if (spawnPoint) {
        position.x = spawnPoint->getX() + spawnPoint->getWidth() / 2.0f;
        position.y = spawnPoint->getY() + spawnPoint->getHeight() / 2.0f;
        return true;
    }
    
    // Find spawn point by type
    auto spawnPoints = map->getObjectsByType("spawn_point");
    if (!spawnPoints.empty()) {
        auto spawn = spawnPoints[0];
        position.x = spawn->getX() + spawn->getWidth() / 2.0f;
        position.y = spawn->getY() + spawn->getHeight() / 2.0f;
        return true;
    }
    
    // Use map center as fallback
    auto tilemap = map->getTilemap();
    if (tilemap) {
        const auto& mapProps = tilemap->getProperties();
        position.x = mapProps.width * mapProps.tileWidth / 2.0f;
        position.y = mapProps.height * mapProps.tileHeight / 2.0f;
        return true;
    }
    
    return false;
}

void WorldManager::updateTransition(float deltaTime) {
    // Update transition time
    m_transitionTime += deltaTime;
    
    // Check if transition is complete
    if (m_transitionTime >= m_transitionDuration) {
        // Complete transition
        m_isTransitioning = false;
        
        // Set new active map
        setActiveMap(m_transitionToMapId);
        
        // Find spawn point
        Physics::Vector2 spawnPosition;
        if (findSpawnPoint(m_activeMap, m_transitionPortalName, spawnPosition)) {
            // Update camera position
            if (m_camera) {
                m_camera->setPosition(spawnPosition.x, spawnPosition.y);
            }
        }
        
        // Fire transition event
        MapTransitionEvent event(m_transitionFromMapId, m_transitionToMapId, m_transitionPortalName);
        for (const auto& pair : m_transitionCallbacks) {
            pair.second(event);
        }
        
        std::cout << "Map transition complete" << std::endl;
    }
}

} // namespace World
} // namespace RPGEngine