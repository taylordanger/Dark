#include "Scene.h"
#include <iostream>

namespace RPGEngine {
namespace Scene {

Scene::Scene(const std::string& sceneId,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ComponentManager> componentManager,
            std::shared_ptr<SystemManager> systemManager,
            std::shared_ptr<Resources::ResourceManager> resourceManager)
    : m_sceneId(sceneId)
    , m_state(SceneState::Unloaded)
    , m_entityManager(entityManager)
    , m_componentManager(componentManager)
    , m_systemManager(systemManager)
    , m_resourceManager(resourceManager)
{
}

Scene::~Scene() {
    if (isLoaded()) {
        unload();
    }
}

bool Scene::load() {
    if (m_state != SceneState::Unloaded) {
        std::cerr << "Scene " << m_sceneId << " is already loaded or loading" << std::endl;
        return false;
    }
    
    setState(SceneState::Loading);
    
    std::cout << "Loading scene: " << m_sceneId << std::endl;
    
    // Call derived class loading logic
    if (!onLoad()) {
        std::cerr << "Failed to load scene: " << m_sceneId << std::endl;
        setState(SceneState::Unloaded);
        return false;
    }
    
    setState(SceneState::Loaded);
    std::cout << "Scene loaded successfully: " << m_sceneId << std::endl;
    
    return true;
}

void Scene::unload() {
    if (m_state == SceneState::Unloaded || m_state == SceneState::Unloading) {
        return;
    }
    
    setState(SceneState::Unloading);
    
    std::cout << "Unloading scene: " << m_sceneId << std::endl;
    
    // Deactivate if active
    if (m_state == SceneState::Active || m_state == SceneState::Paused) {
        deactivate();
    }
    
    // Call derived class unloading logic
    onUnload();
    
    // Clear all entities in this scene
    if (m_entityManager) {
        m_entityManager->clearAllEntities();
    }
    
    // Clear properties
    m_properties.clear();
    
    setState(SceneState::Unloaded);
    std::cout << "Scene unloaded: " << m_sceneId << std::endl;
}

void Scene::activate() {
    if (m_state != SceneState::Loaded) {
        std::cerr << "Cannot activate scene " << m_sceneId << " - not loaded" << std::endl;
        return;
    }
    
    setState(SceneState::Active);
    
    std::cout << "Activating scene: " << m_sceneId << std::endl;
    
    // Call derived class activation logic
    onActivate();
    
    std::cout << "Scene activated: " << m_sceneId << std::endl;
}

void Scene::deactivate() {
    if (m_state != SceneState::Active && m_state != SceneState::Paused) {
        return;
    }
    
    std::cout << "Deactivating scene: " << m_sceneId << std::endl;
    
    // Call derived class deactivation logic
    onDeactivate();
    
    setState(SceneState::Loaded);
    std::cout << "Scene deactivated: " << m_sceneId << std::endl;
}

void Scene::pause() {
    if (m_state != SceneState::Active) {
        return;
    }
    
    setState(SceneState::Paused);
    
    std::cout << "Pausing scene: " << m_sceneId << std::endl;
    
    // Call derived class pause logic
    onPause();
    
    std::cout << "Scene paused: " << m_sceneId << std::endl;
}

void Scene::resume() {
    if (m_state != SceneState::Paused) {
        return;
    }
    
    setState(SceneState::Active);
    
    std::cout << "Resuming scene: " << m_sceneId << std::endl;
    
    // Call derived class resume logic
    onResume();
    
    std::cout << "Scene resumed: " << m_sceneId << std::endl;
}

void Scene::update(float deltaTime) {
    if (m_state != SceneState::Active) {
        return;
    }
    
    // Update entity manager
    if (m_entityManager) {
        m_entityManager->update();
    }
    
    // Update systems
    if (m_systemManager) {
        m_systemManager->update(deltaTime);
    }
    
    // Call derived class update logic
    onUpdate(deltaTime);
}

void Scene::render(float deltaTime) {
    if (m_state != SceneState::Active) {
        return;
    }
    
    // Call derived class render logic
    onRender(deltaTime);
}

void Scene::handleInput(const std::string& event) {
    if (m_state != SceneState::Active) {
        return;
    }
    
    // Call derived class input handling
    onHandleInput(event);
}

SceneData Scene::saveState() {
    SceneData data(m_sceneId);
    
    // Save properties
    data.properties = m_properties;
    
    // Call derived class save logic
    onSaveState(data);
    
    std::cout << "Scene state saved: " << m_sceneId << std::endl;
    
    return data;
}

bool Scene::loadState(const SceneData& data) {
    if (data.sceneId != m_sceneId) {
        std::cerr << "Scene ID mismatch: expected " << m_sceneId << ", got " << data.sceneId << std::endl;
        return false;
    }
    
    // Load properties
    m_properties = data.properties;
    
    // Call derived class load logic
    if (!onLoadState(data)) {
        std::cerr << "Failed to load scene state: " << m_sceneId << std::endl;
        return false;
    }
    
    std::cout << "Scene state loaded: " << m_sceneId << std::endl;
    
    return true;
}

void Scene::setProperty(const std::string& key, const std::string& value) {
    m_properties[key] = value;
}

std::string Scene::getProperty(const std::string& key, const std::string& defaultValue) const {
    auto it = m_properties.find(key);
    return (it != m_properties.end()) ? it->second : defaultValue;
}

void Scene::requestTransition(const std::string& targetSceneId, SceneTransitionType transitionType) {
    if (m_transitionCallback) {
        m_transitionCallback(targetSceneId, transitionType);
    } else {
        std::cerr << "No transition callback set for scene: " << m_sceneId << std::endl;
    }
}

} // namespace Scene
} // namespace RPGEngine