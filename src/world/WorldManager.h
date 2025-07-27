#pragma once

#include "Map.h"
#include "../tilemap/MapLoader.h"
#include "../resources/ResourceManager.h"
#include "../systems/System.h"
#include "../entities/EntityManager.h"
#include "../components/ComponentManager.h"
#include "../graphics/Camera.h"
#include "../core/Event.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

namespace RPGEngine {
namespace World {

/**
 * Map transition event
 * Fired when a map transition occurs
 */
struct MapTransitionEvent : public Event<MapTransitionEvent> {
    uint32_t fromMapId;    // Source map ID
    uint32_t toMapId;      // Destination map ID
    std::string portalName; // Portal name
    
    MapTransitionEvent(uint32_t fromMapId, uint32_t toMapId, const std::string& portalName)
        : fromMapId(fromMapId), toMapId(toMapId), portalName(portalName) {}
};

/**
 * Map loaded event
 * Fired when a map is loaded
 */
struct MapLoadedEvent : public Event<MapLoadedEvent> {
    uint32_t mapId;        // Map ID
    std::string mapName;   // Map name
    
    MapLoadedEvent(uint32_t mapId, const std::string& mapName)
        : mapId(mapId), mapName(mapName) {}
};

/**
 * Map unloaded event
 * Fired when a map is unloaded
 */
struct MapUnloadedEvent : public Event<MapUnloadedEvent> {
    uint32_t mapId;        // Map ID
    std::string mapName;   // Map name
    
    MapUnloadedEvent(uint32_t mapId, const std::string& mapName)
        : mapId(mapId), mapName(mapName) {}
};

/**
 * World manager class
 * Manages maps and map transitions
 */
class WorldManager : public System {
public:
    /**
     * Constructor
     * @param resourceManager Resource manager
     * @param entityManager Entity manager
     * @param componentManager Component manager
     */
    WorldManager(std::shared_ptr<Resources::ResourceManager> resourceManager,
                std::shared_ptr<EntityManager> entityManager,
                std::shared_ptr<ComponentManager> componentManager);
    
    /**
     * Destructor
     */
    ~WorldManager();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Load a map from a file
     * @param filename Map file path
     * @param id Map ID (0 for auto-generated)
     * @return Loaded map, or nullptr if loading failed
     */
    std::shared_ptr<Map> loadMap(const std::string& filename, uint32_t id = 0);
    
    /**
     * Unload a map
     * @param id Map ID
     * @return true if the map was unloaded
     */
    bool unloadMap(uint32_t id);
    
    /**
     * Get a map by ID
     * @param id Map ID
     * @return Map, or nullptr if not found
     */
    std::shared_ptr<Map> getMap(uint32_t id) const;
    
    /**
     * Get a map by name
     * @param name Map name
     * @return Map, or nullptr if not found
     */
    std::shared_ptr<Map> getMapByName(const std::string& name) const;
    
    /**
     * Get all maps
     * @return Map of maps
     */
    const std::unordered_map<uint32_t, std::shared_ptr<Map>>& getMaps() const { return m_maps; }
    
    /**
     * Get the active map
     * @return Active map, or nullptr if no map is active
     */
    std::shared_ptr<Map> getActiveMap() const { return m_activeMap; }
    
    /**
     * Set the active map
     * @param id Map ID
     * @return true if the map was set as active
     */
    bool setActiveMap(uint32_t id);
    
    /**
     * Transition to another map
     * @param toMapId Destination map ID
     * @param portalName Portal name
     * @param fadeTime Fade time in seconds
     * @return true if the transition was initiated
     */
    bool transitionToMap(uint32_t toMapId, const std::string& portalName = "", float fadeTime = 1.0f);
    
    /**
     * Register a map transition callback
     * @param callback Function to call when a map transition occurs
     * @return Callback ID
     */
    int registerTransitionCallback(const std::function<void(const MapTransitionEvent&)>& callback);
    
    /**
     * Unregister a map transition callback
     * @param callbackId Callback ID
     * @return true if the callback was unregistered
     */
    bool unregisterTransitionCallback(int callbackId);
    
    /**
     * Register a map loaded callback
     * @param callback Function to call when a map is loaded
     * @return Callback ID
     */
    int registerMapLoadedCallback(const std::function<void(const MapLoadedEvent&)>& callback);
    
    /**
     * Unregister a map loaded callback
     * @param callbackId Callback ID
     * @return true if the callback was unregistered
     */
    bool unregisterMapLoadedCallback(int callbackId);
    
    /**
     * Register a map unloaded callback
     * @param callback Function to call when a map is unloaded
     * @return Callback ID
     */
    int registerMapUnloadedCallback(const std::function<void(const MapUnloadedEvent&)>& callback);
    
    /**
     * Unregister a map unloaded callback
     * @param callbackId Callback ID
     * @return true if the callback was unregistered
     */
    bool unregisterMapUnloadedCallback(int callbackId);
    
    /**
     * Set the camera
     * @param camera Camera
     */
    void setCamera(std::shared_ptr<Graphics::Camera> camera) { m_camera = camera; }
    
    /**
     * Get the camera
     * @return Camera
     */
    std::shared_ptr<Graphics::Camera> getCamera() const { return m_camera; }
    
    /**
     * Set the map directory
     * @param directory Map directory
     */
    void setMapDirectory(const std::string& directory) { m_mapDirectory = directory; }
    
    /**
     * Get the map directory
     * @return Map directory
     */
    const std::string& getMapDirectory() const { return m_mapDirectory; }
    
    /**
     * Get the map loader
     * @return Map loader
     */
    std::shared_ptr<Tilemap::MapLoader> getMapLoader() const { return m_mapLoader; }
    
private:
    /**
     * Create entities from map objects
     * @param map Map
     */
    void createEntitiesFromObjects(std::shared_ptr<Map> map);
    
    /**
     * Find a spawn point in a map
     * @param map Map
     * @param portalName Portal name
     * @param position Output position
     * @return true if a spawn point was found
     */
    bool findSpawnPoint(std::shared_ptr<Map> map, const std::string& portalName, Physics::Vector2& position);
    
    /**
     * Update map transition
     * @param deltaTime Time since last update
     */
    void updateTransition(float deltaTime);
    
    // Managers
    std::shared_ptr<Resources::ResourceManager> m_resourceManager;
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // Map loader
    std::shared_ptr<Tilemap::MapLoader> m_mapLoader;
    
    // Maps
    std::unordered_map<uint32_t, std::shared_ptr<Map>> m_maps;
    std::shared_ptr<Map> m_activeMap;
    
    // Camera
    std::shared_ptr<Graphics::Camera> m_camera;
    
    // Map directory
    std::string m_mapDirectory;
    
    // Map transition
    bool m_isTransitioning;
    float m_transitionTime;
    float m_transitionDuration;
    uint32_t m_transitionFromMapId;
    uint32_t m_transitionToMapId;
    std::string m_transitionPortalName;
    
    // Callbacks
    std::unordered_map<int, std::function<void(const MapTransitionEvent&)>> m_transitionCallbacks;
    std::unordered_map<int, std::function<void(const MapLoadedEvent&)>> m_mapLoadedCallbacks;
    std::unordered_map<int, std::function<void(const MapUnloadedEvent&)>> m_mapUnloadedCallbacks;
    int m_nextCallbackId;
};

} // namespace World
} // namespace RPGEngine