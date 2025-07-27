#pragma once

#include "Scene.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <stack>
#include <functional>

namespace RPGEngine {
namespace Scene {

/**
 * Scene transition data
 */
struct SceneTransition {
    std::string fromSceneId;
    std::string toSceneId;
    SceneTransitionType type;
    float duration;
    float progress;
    bool isComplete;
    std::function<void()> onComplete;
    
    SceneTransition(const std::string& from, const std::string& to, 
                   SceneTransitionType transitionType, float dur = 1.0f)
        : fromSceneId(from), toSceneId(to), type(transitionType), 
          duration(dur), progress(0.0f), isComplete(false) {}
};

/**
 * Scene manager
 * Manages scene lifecycle, transitions, and state persistence
 */
class SceneManager {
public:
    /**
     * Constructor
     * @param entityManager Entity manager
     * @param componentManager Component manager
     * @param systemManager System manager
     * @param resourceManager Resource manager
     */
    SceneManager(std::shared_ptr<EntityManager> entityManager,
                std::shared_ptr<ComponentManager> componentManager,
                std::shared_ptr<SystemManager> systemManager,
                std::shared_ptr<Resources::ResourceManager> resourceManager);
    
    /**
     * Destructor
     */
    ~SceneManager();
    
    /**
     * Update the scene manager
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
    
    /**
     * Render the current scene
     * @param deltaTime Time since last render
     */
    void render(float deltaTime);
    
    /**
     * Handle input event
     * @param event Input event
     */
    void handleInput(const std::string& event);
    
    /**
     * Register a scene
     * @param scene Scene to register
     * @return true if registration was successful
     */
    bool registerScene(std::shared_ptr<Scene> scene);
    
    /**
     * Unregister a scene
     * @param sceneId Scene ID to unregister
     * @return true if unregistration was successful
     */
    bool unregisterScene(const std::string& sceneId);
    
    /**
     * Load a scene
     * @param sceneId Scene ID to load
     * @return true if loading was successful
     */
    bool loadScene(const std::string& sceneId);
    
    /**
     * Unload a scene
     * @param sceneId Scene ID to unload
     * @return true if unloading was successful
     */
    bool unloadScene(const std::string& sceneId);
    
    /**
     * Switch to a scene
     * @param sceneId Scene ID to switch to
     * @param transitionType Transition type
     * @param transitionDuration Transition duration in seconds
     * @param onComplete Callback when transition completes
     * @return true if transition was initiated
     */
    bool switchToScene(const std::string& sceneId, 
                      SceneTransitionType transitionType = SceneTransitionType::Fade,
                      float transitionDuration = 1.0f,
                      std::function<void()> onComplete = nullptr);
    
    /**
     * Push a scene onto the scene stack (for overlays/menus)
     * @param sceneId Scene ID to push
     * @param pauseCurrentScene Whether to pause the current scene
     * @return true if push was successful
     */
    bool pushScene(const std::string& sceneId, bool pauseCurrentScene = true);
    
    /**
     * Pop the top scene from the scene stack
     * @return true if pop was successful
     */
    bool popScene();
    
    /**
     * Get current active scene
     * @return Current scene, or nullptr if no scene is active
     */
    std::shared_ptr<Scene> getCurrentScene() const;
    
    /**
     * Get scene by ID
     * @param sceneId Scene ID
     * @return Scene, or nullptr if not found
     */
    std::shared_ptr<Scene> getScene(const std::string& sceneId) const;
    
    /**
     * Check if scene exists
     * @param sceneId Scene ID
     * @return true if scene exists
     */
    bool hasScene(const std::string& sceneId) const;
    
    /**
     * Check if scene is loaded
     * @param sceneId Scene ID
     * @return true if scene is loaded
     */
    bool isSceneLoaded(const std::string& sceneId) const;
    
    /**
     * Check if scene is active
     * @param sceneId Scene ID
     * @return true if scene is active
     */
    bool isSceneActive(const std::string& sceneId) const;
    
    /**
     * Check if transition is in progress
     * @return true if transition is in progress
     */
    bool isTransitioning() const { return m_currentTransition != nullptr; }
    
    /**
     * Get current transition progress (0.0 to 1.0)
     * @return Transition progress
     */
    float getTransitionProgress() const;
    
    /**
     * Save scene state
     * @param sceneId Scene ID
     * @return Scene data, or empty data if scene not found
     */
    SceneData saveSceneState(const std::string& sceneId);
    
    /**
     * Load scene state
     * @param data Scene data to load
     * @return true if loading was successful
     */
    bool loadSceneState(const SceneData& data);
    
    /**
     * Save all scene states
     * @return Map of scene ID to scene data
     */
    std::unordered_map<std::string, SceneData> saveAllSceneStates();
    
    /**
     * Load all scene states
     * @param sceneStates Map of scene ID to scene data
     * @return true if all scenes were loaded successfully
     */
    bool loadAllSceneStates(const std::unordered_map<std::string, SceneData>& sceneStates);
    
    /**
     * Set transition effect callback
     * @param callback Callback function for custom transition effects
     */
    void setTransitionEffectCallback(std::function<void(const SceneTransition&)> callback) {
        m_transitionEffectCallback = callback;
    }
    
    /**
     * Get scene stack size
     * @return Number of scenes in the stack
     */
    size_t getSceneStackSize() const { return m_sceneStack.size(); }
    
    /**
     * Clear all scenes
     */
    void clearAllScenes();
    
private:
    /**
     * Handle scene transition request
     * @param targetSceneId Target scene ID
     * @param transitionType Transition type
     */
    void handleTransitionRequest(const std::string& targetSceneId, SceneTransitionType transitionType);
    
    /**
     * Update current transition
     * @param deltaTime Time since last update
     */
    void updateTransition(float deltaTime);
    
    /**
     * Complete current transition
     */
    void completeTransition();
    
    // Managers
    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
    std::shared_ptr<SystemManager> m_systemManager;
    std::shared_ptr<Resources::ResourceManager> m_resourceManager;
    
    // Scene management
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::stack<std::string> m_sceneStack;
    std::string m_currentSceneId;
    
    // Transition management
    std::unique_ptr<SceneTransition> m_currentTransition;
    std::function<void(const SceneTransition&)> m_transitionEffectCallback;
};

} // namespace Scene
} // namespace RPGEngine