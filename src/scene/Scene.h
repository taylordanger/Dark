#pragma once

#include "../entities/EntityManager.h"
#include "../components/ComponentManager.h"
#include "../systems/SystemManager.h"
#include "../resources/ResourceManager.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

namespace RPGEngine {
namespace Scene {

/**
 * Scene state enumeration
 */
enum class SceneState {
    Unloaded,       // Scene is not loaded
    Loading,        // Scene is being loaded
    Loaded,         // Scene is loaded but not active
    Active,         // Scene is active and running
    Paused,         // Scene is paused
    Unloading,      // Scene is being unloaded
    Transitioning   // Scene is transitioning
};

/**
 * Scene transition type
 */
enum class SceneTransitionType {
    Immediate,      // Immediate transition
    Fade,           // Fade out/in transition
    Slide,          // Slide transition
    Custom          // Custom transition
};

/**
 * Scene data for persistence
 */
struct SceneData {
    std::string sceneId;
    std::unordered_map<std::string, std::string> properties;
    std::vector<uint8_t> binaryData;
    
    SceneData() = default;
    SceneData(const std::string& id) : sceneId(id) {}
};

/**
 * Base scene class
 * Manages entities, components, and systems for a specific game scene
 */
class Scene {
public:
    /**
     * Constructor
     * @param sceneId Unique scene identifier
     * @param entityManager Entity manager
     * @param componentManager Component manager
     * @param systemManager System manager
     * @param resourceManager Resource manager
     */
    Scene(const std::string& sceneId,
          std::shared_ptr<EntityManager> entityManager,
          std::shared_ptr<ComponentManager> componentManager,
          std::shared_ptr<SystemManager> systemManager,
          std::shared_ptr<Resources::ResourceManager> resourceManager);
    
    /**
     * Virtual destructor
     */
    virtual ~Scene();
    
    /**
     * Load the scene
     * @return true if loading was successful
     */
    virtual bool load();
    
    /**
     * Unload the scene
     */
    virtual void unload();
    
    /**
     * Activate the scene
     */
    virtual void activate();
    
    /**
     * Deactivate the scene
     */
    virtual void deactivate();
    
    /**
     * Pause the scene
     */
    virtual void pause();
    
    /**
     * Resume the scene
     */
    virtual void resume();
    
    /**
     * Update the scene
     * @param deltaTime Time since last update
     */
    virtual void update(float deltaTime);
    
    /**
     * Render the scene
     * @param deltaTime Time since last render
     */
    virtual void render(float deltaTime);
    
    /**
     * Handle input event
     * @param event Input event
     */
    virtual void handleInput(const std::string& event);
    
    /**
     * Get scene ID
     * @return Scene ID
     */
    const std::string& getSceneId() const { return m_sceneId; }
    
    /**
     * Get scene state
     * @return Scene state
     */
    SceneState getState() const { return m_state; }
    
    /**
     * Check if scene is loaded
     * @return true if scene is loaded
     */
    bool isLoaded() const { return m_state != SceneState::Unloaded; }
    
    /**
     * Check if scene is active
     * @return true if scene is active
     */
    bool isActive() const { return m_state == SceneState::Active; }
    
    /**
     * Check if scene is paused
     * @return true if scene is paused
     */
    bool isPaused() const { return m_state == SceneState::Paused; }
    
    /**
     * Save scene state
     * @return Scene data for persistence
     */
    virtual SceneData saveState();
    
    /**
     * Load scene state
     * @param data Scene data to restore
     * @return true if restoration was successful
     */
    virtual bool loadState(const SceneData& data);
    
    /**
     * Set scene property
     * @param key Property key
     * @param value Property value
     */
    void setProperty(const std::string& key, const std::string& value);
    
    /**
     * Get scene property
     * @param key Property key
     * @param defaultValue Default value if key not found
     * @return Property value
     */
    std::string getProperty(const std::string& key, const std::string& defaultValue = "") const;
    
    /**
     * Set transition callback
     * @param callback Callback function for scene transitions
     */
    void setTransitionCallback(std::function<void(const std::string&, SceneTransitionType)> callback) {
        m_transitionCallback = callback;
    }
    
    /**
     * Request scene transition
     * @param targetSceneId Target scene ID
     * @param transitionType Transition type
     */
    void requestTransition(const std::string& targetSceneId, SceneTransitionType transitionType = SceneTransitionType::Fade);
    
    /**
     * Get entity manager
     * @return Entity manager
     */
    std::shared_ptr<EntityManager> getEntityManager() const { return m_entityManager; }
    
    /**
     * Get component manager
     * @return Component manager
     */
    std::shared_ptr<ComponentManager> getComponentManager() const { return m_componentManager; }
    
    /**
     * Get system manager
     * @return System manager
     */
    std::shared_ptr<SystemManager> getSystemManager() const { return m_systemManager; }
    
    /**
     * Get resource manager
     * @return Resource manager
     */
    std::shared_ptr<Resources::ResourceManager> getResourceManager() const { return m_resourceManager; }
    
protected:
    /**
     * Called when scene is being loaded
     * Override this method to implement scene-specific loading
     * @return true if loading was successful
     */
    virtual bool onLoad() { return true; }
    
    /**
     * Called when scene is being unloaded
     * Override this method to implement scene-specific unloading
     */
    virtual void onUnload() {}
    
    /**
     * Called when scene is activated
     * Override this method to implement scene-specific activation
     */
    virtual void onActivate() {}
    
    /**
     * Called when scene is deactivated
     * Override this method to implement scene-specific deactivation
     */
    virtual void onDeactivate() {}
    
    /**
     * Called when scene is paused
     * Override this method to implement scene-specific pause behavior
     */
    virtual void onPause() {}
    
    /**
     * Called when scene is resumed
     * Override this method to implement scene-specific resume behavior
     */
    virtual void onResume() {}
    
    /**
     * Called during scene update
     * Override this method to implement scene-specific update logic
     * @param deltaTime Time since last update
     */
    virtual void onUpdate(float deltaTime) {}
    
    /**
     * Called during scene render
     * Override this method to implement scene-specific rendering
     * @param deltaTime Time since last render
     */
    virtual void onRender(float deltaTime) {}
    
    /**
     * Called when input event is received
     * Override this method to implement scene-specific input handling
     * @param event Input event
     */
    virtual void onHandleInput(const std::string& event) {}
    
    /**
     * Called when saving scene state
     * Override this method to save scene-specific data
     * @param data Scene data to populate
     */
    virtual void onSaveState(SceneData& data) {}
    
    /**
     * Called when loading scene state
     * Override this method to restore scene-specific data
     * @param data Scene data to restore from
     * @return true if restoration was successful
     */
    virtual bool onLoadState(const SceneData& data) { return true; }
    
    /**
     * Set scene state
     * @param state New scene state
     */
    void setState(SceneState state) { m_state = state; }
    
    // Managers
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
    std::shared_ptr<SystemManager> m_systemManager;
    std::shared_ptr<Resources::ResourceManager> m_resourceManager;
    
private:
    std::string m_sceneId;
    SceneState m_state;
    
    // Scene properties
    std::unordered_map<std::string, std::string> m_properties;
    
    // Transition callback
    std::function<void(const std::string&, SceneTransitionType)> m_transitionCallback;
};

} // namespace Scene
} // namespace RPGEngine