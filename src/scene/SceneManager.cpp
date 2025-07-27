#include "SceneManager.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Scene {

SceneManager::SceneManager(std::shared_ptr<EntityManager> entityManager,
                          std::shared_ptr<ComponentManager> componentManager,
                          std::shared_ptr<SystemManager> systemManager,
                          std::shared_ptr<Resources::ResourceManager> resourceManager)
    : m_entityManager(entityManager)
    , m_componentManager(componentManager)
    , m_systemManager(systemManager)
    , m_resourceManager(resourceManager)
{
}

SceneManager::~SceneManager() {
    clearAllScenes();
}

void SceneManager::update(float deltaTime) {
    // Update transition if in progress
    if (m_currentTransition) {
        updateTransition(deltaTime);
    }
    
    // Update current scene
    auto currentScene = getCurrentScene();
    if (currentScene) {
        currentScene->update(deltaTime);
    }
}

void SceneManager::render(float deltaTime) {
    // Render current scene
    auto currentScene = getCurrentScene();
    if (currentScene) {
        currentScene->render(deltaTime);
    }
    
    // Apply transition effects if in progress
    if (m_currentTransition && m_transitionEffectCallback) {
        m_transitionEffectCallback(*m_currentTransition);
    }
}

void SceneManager::handleInput(const std::string& event) {
    // Forward input to current scene
    auto currentScene = getCurrentScene();
    if (currentScene) {
        currentScene->handleInput(event);
    }
}

bool SceneManager::registerScene(std::shared_ptr<Scene> scene) {
    if (!scene) {
        std::cerr << "Cannot register null scene" << std::endl;
        return false;
    }
    
    const std::string& sceneId = scene->getSceneId();
    
    if (hasScene(sceneId)) {
        std::cerr << "Scene already registered: " << sceneId << std::endl;
        return false;
    }
    
    // Set transition callback
    scene->setTransitionCallback([this](const std::string& targetSceneId, SceneTransitionType transitionType) {
        handleTransitionRequest(targetSceneId, transitionType);
    });
    
    m_scenes[sceneId] = scene;
    
    std::cout << "Scene registered: " << sceneId << std::endl;
    return true;
}

bool SceneManager::unregisterScene(const std::string& sceneId) {
    auto it = m_scenes.find(sceneId);
    if (it == m_scenes.end()) {
        std::cerr << "Scene not found: " << sceneId << std::endl;
        return false;
    }
    
    // Unload scene if loaded
    if (it->second->isLoaded()) {
        it->second->unload();
    }
    
    // Remove from current scene if it's the current one
    if (m_currentSceneId == sceneId) {
        m_currentSceneId.clear();
    }
    
    // Remove from scene stack
    std::stack<std::string> tempStack;
    while (!m_sceneStack.empty()) {
        std::string topSceneId = m_sceneStack.top();
        m_sceneStack.pop();
        
        if (topSceneId != sceneId) {
            tempStack.push(topSceneId);
        }
    }
    
    // Restore stack without the removed scene
    while (!tempStack.empty()) {
        m_sceneStack.push(tempStack.top());
        tempStack.pop();
    }
    
    m_scenes.erase(it);
    
    std::cout << "Scene unregistered: " << sceneId << std::endl;
    return true;
}

bool SceneManager::loadScene(const std::string& sceneId) {
    auto scene = getScene(sceneId);
    if (!scene) {
        std::cerr << "Scene not found: " << sceneId << std::endl;
        return false;
    }
    
    if (scene->isLoaded()) {
        std::cout << "Scene already loaded: " << sceneId << std::endl;
        return true;
    }
    
    return scene->load();
}

bool SceneManager::unloadScene(const std::string& sceneId) {
    auto scene = getScene(sceneId);
    if (!scene) {
        std::cerr << "Scene not found: " << sceneId << std::endl;
        return false;
    }
    
    if (!scene->isLoaded()) {
        std::cout << "Scene already unloaded: " << sceneId << std::endl;
        return true;
    }
    
    // Don't unload if it's the current scene or in the stack
    if (m_currentSceneId == sceneId) {
        std::cerr << "Cannot unload current scene: " << sceneId << std::endl;
        return false;
    }
    
    // Check if scene is in stack
    std::stack<std::string> tempStack = m_sceneStack;
    while (!tempStack.empty()) {
        if (tempStack.top() == sceneId) {
            std::cerr << "Cannot unload scene in stack: " << sceneId << std::endl;
            return false;
        }
        tempStack.pop();
    }
    
    scene->unload();
    return true;
}

bool SceneManager::switchToScene(const std::string& sceneId, SceneTransitionType transitionType,
                                float transitionDuration, std::function<void()> onComplete) {
    if (isTransitioning()) {
        std::cerr << "Cannot switch scenes while transition is in progress" << std::endl;
        return false;
    }
    
    auto targetScene = getScene(sceneId);
    if (!targetScene) {
        std::cerr << "Target scene not found: " << sceneId << std::endl;
        return false;
    }
    
    // Load target scene if not loaded
    if (!targetScene->isLoaded()) {
        if (!targetScene->load()) {
            std::cerr << "Failed to load target scene: " << sceneId << std::endl;
            return false;
        }
    }
    
    // If no current scene, just activate the target
    if (m_currentSceneId.empty()) {
        m_currentSceneId = sceneId;
        targetScene->activate();
        
        if (onComplete) {
            onComplete();
        }
        
        return true;
    }
    
    // Start transition
    m_currentTransition = std::make_unique<SceneTransition>(m_currentSceneId, sceneId, transitionType, transitionDuration);
    m_currentTransition->onComplete = onComplete;
    
    std::cout << "Starting scene transition: " << m_currentSceneId << " -> " << sceneId << std::endl;
    
    return true;
}

bool SceneManager::pushScene(const std::string& sceneId, bool pauseCurrentScene) {
    auto scene = getScene(sceneId);
    if (!scene) {
        std::cerr << "Scene not found: " << sceneId << std::endl;
        return false;
    }
    
    // Load scene if not loaded
    if (!scene->isLoaded()) {
        if (!scene->load()) {
            std::cerr << "Failed to load scene: " << sceneId << std::endl;
            return false;
        }
    }
    
    // Pause current scene if requested
    if (pauseCurrentScene && !m_currentSceneId.empty()) {
        auto currentScene = getCurrentScene();
        if (currentScene) {
            currentScene->pause();
        }
    }
    
    // Push current scene to stack
    if (!m_currentSceneId.empty()) {
        m_sceneStack.push(m_currentSceneId);
    }
    
    // Set new current scene
    m_currentSceneId = sceneId;
    scene->activate();
    
    std::cout << "Pushed scene: " << sceneId << std::endl;
    return true;
}

bool SceneManager::popScene() {
    if (m_sceneStack.empty()) {
        std::cerr << "No scenes to pop" << std::endl;
        return false;
    }
    
    // Deactivate current scene
    auto currentScene = getCurrentScene();
    if (currentScene) {
        currentScene->deactivate();
    }
    
    // Pop previous scene from stack
    std::string previousSceneId = m_sceneStack.top();
    m_sceneStack.pop();
    
    m_currentSceneId = previousSceneId;
    
    // Resume previous scene
    auto previousScene = getCurrentScene();
    if (previousScene) {
        if (previousScene->isPaused()) {
            previousScene->resume();
        } else {
            previousScene->activate();
        }
    }
    
    std::cout << "Popped scene, returned to: " << previousSceneId << std::endl;
    return true;
}

std::shared_ptr<Scene> SceneManager::getCurrentScene() const {
    if (m_currentSceneId.empty()) {
        return nullptr;
    }
    
    return getScene(m_currentSceneId);
}

std::shared_ptr<Scene> SceneManager::getScene(const std::string& sceneId) const {
    auto it = m_scenes.find(sceneId);
    return (it != m_scenes.end()) ? it->second : nullptr;
}

bool SceneManager::hasScene(const std::string& sceneId) const {
    return m_scenes.find(sceneId) != m_scenes.end();
}

bool SceneManager::isSceneLoaded(const std::string& sceneId) const {
    auto scene = getScene(sceneId);
    return scene && scene->isLoaded();
}

bool SceneManager::isSceneActive(const std::string& sceneId) const {
    auto scene = getScene(sceneId);
    return scene && scene->isActive();
}

float SceneManager::getTransitionProgress() const {
    return m_currentTransition ? m_currentTransition->progress : 0.0f;
}

SceneData SceneManager::saveSceneState(const std::string& sceneId) {
    auto scene = getScene(sceneId);
    if (!scene) {
        std::cerr << "Scene not found: " << sceneId << std::endl;
        return SceneData();
    }
    
    return scene->saveState();
}

bool SceneManager::loadSceneState(const SceneData& data) {
    auto scene = getScene(data.sceneId);
    if (!scene) {
        std::cerr << "Scene not found: " << data.sceneId << std::endl;
        return false;
    }
    
    return scene->loadState(data);
}

std::unordered_map<std::string, SceneData> SceneManager::saveAllSceneStates() {
    std::unordered_map<std::string, SceneData> sceneStates;
    
    for (const auto& pair : m_scenes) {
        if (pair.second->isLoaded()) {
            sceneStates[pair.first] = pair.second->saveState();
        }
    }
    
    std::cout << "Saved " << sceneStates.size() << " scene states" << std::endl;
    return sceneStates;
}

bool SceneManager::loadAllSceneStates(const std::unordered_map<std::string, SceneData>& sceneStates) {
    bool allSuccessful = true;
    
    for (const auto& pair : sceneStates) {
        if (!loadSceneState(pair.second)) {
            allSuccessful = false;
        }
    }
    
    std::cout << "Loaded scene states, all successful: " << (allSuccessful ? "Yes" : "No") << std::endl;
    return allSuccessful;
}

void SceneManager::clearAllScenes() {
    // Clear current transition
    m_currentTransition.reset();
    
    // Unload all scenes
    for (auto& pair : m_scenes) {
        if (pair.second->isLoaded()) {
            pair.second->unload();
        }
    }
    
    // Clear containers
    m_scenes.clear();
    while (!m_sceneStack.empty()) {
        m_sceneStack.pop();
    }
    m_currentSceneId.clear();
    
    std::cout << "All scenes cleared" << std::endl;
}

void SceneManager::handleTransitionRequest(const std::string& targetSceneId, SceneTransitionType transitionType) {
    switchToScene(targetSceneId, transitionType);
}

void SceneManager::updateTransition(float deltaTime) {
    if (!m_currentTransition) {
        return;
    }
    
    m_currentTransition->progress += deltaTime / m_currentTransition->duration;
    
    if (m_currentTransition->progress >= 1.0f) {
        m_currentTransition->progress = 1.0f;
        completeTransition();
    }
}

void SceneManager::completeTransition() {
    if (!m_currentTransition) {
        return;
    }
    
    // Deactivate current scene
    auto currentScene = getScene(m_currentTransition->fromSceneId);
    if (currentScene) {
        currentScene->deactivate();
    }
    
    // Activate target scene
    auto targetScene = getScene(m_currentTransition->toSceneId);
    if (targetScene) {
        targetScene->activate();
        m_currentSceneId = m_currentTransition->toSceneId;
    }
    
    // Call completion callback
    if (m_currentTransition->onComplete) {
        m_currentTransition->onComplete();
    }
    
    std::cout << "Scene transition completed: " << m_currentTransition->fromSceneId 
              << " -> " << m_currentTransition->toSceneId << std::endl;
    
    // Clear transition
    m_currentTransition.reset();
}

} // namespace Scene
} // namespace RPGEngine