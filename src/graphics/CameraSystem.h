#pragma once

#include "Camera.h"
#include "../systems/System.h"
#include "../components/ComponentManager.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace RPGEngine {
namespace Graphics {

/**
 * Camera System
 * Manages cameras and integrates with the entity-component system
 */
class CameraSystem : public System {
public:
    /**
     * Constructor
     * @param componentManager Component manager to use
     */
    explicit CameraSystem(std::shared_ptr<ComponentManager> componentManager);
    
    /**
     * Destructor
     */
    ~CameraSystem();
    
    /**
     * Initialize the camera system
     * @return true if initialization was successful
     */
    bool onInitialize() override;
    
    /**
     * Update the camera system
     * @param deltaTime Time elapsed since last update
     */
    void onUpdate(float deltaTime) override;
    
    /**
     * Shutdown the camera system
     */
    void onShutdown() override;
    
    /**
     * Create a camera
     * @param name Camera name
     * @return Shared pointer to the created camera
     */
    std::shared_ptr<Camera> createCamera(const std::string& name);
    
    /**
     * Get a camera by name
     * @param name Camera name
     * @return Shared pointer to the camera, or nullptr if not found
     */
    std::shared_ptr<Camera> getCamera(const std::string& name) const;
    
    /**
     * Remove a camera
     * @param name Camera name
     * @return true if the camera was removed
     */
    bool removeCamera(const std::string& name);
    
    /**
     * Set the active camera
     * @param name Camera name
     * @return true if the camera was set as active
     */
    bool setActiveCamera(const std::string& name);
    
    /**
     * Get the active camera
     * @return Shared pointer to the active camera, or nullptr if none
     */
    std::shared_ptr<Camera> getActiveCamera() const;
    
    /**
     * Get the active camera name
     * @return Active camera name, or empty string if none
     */
    const std::string& getActiveCameraName() const { return m_activeCameraName; }
    
    /**
     * Check if a camera exists
     * @param name Camera name
     * @return true if the camera exists
     */
    bool hasCamera(const std::string& name) const;
    
    /**
     * Get the number of cameras
     * @return Number of cameras
     */
    size_t getCameraCount() const { return m_cameras.size(); }
    
    /**
     * Create a camera entity
     * @param name Entity name
     * @return Created entity
     */
    Entity createCameraEntity(const std::string& name);
    
    /**
     * Set a camera to follow an entity
     * @param cameraName Camera name
     * @param entity Entity to follow
     * @param offsetX X offset from entity position
     * @param offsetY Y offset from entity position
     * @return true if the camera was set to follow the entity
     */
    bool setCameraToFollowEntity(const std::string& cameraName, Entity entity, float offsetX = 0.0f, float offsetY = 0.0f);
    
    /**
     * Stop a camera from following an entity
     * @param cameraName Camera name
     * @return true if the camera was stopped from following
     */
    bool stopCameraFromFollowing(const std::string& cameraName);
    
private:
    // Component manager
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // Cameras
    std::unordered_map<std::string, std::shared_ptr<Camera>> m_cameras;
    
    // Active camera
    std::string m_activeCameraName;
    
    // Default camera name
    static const std::string DEFAULT_CAMERA_NAME;
};

} // namespace Graphics
} // namespace RPGEngine