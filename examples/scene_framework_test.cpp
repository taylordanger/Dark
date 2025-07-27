#include "../src/scene/SceneManager.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/resources/ResourceManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Scene;

/**
 * Test scene implementation
 */
class TestScene : public Scene {
public:
    TestScene(const std::string& sceneId,
              std::shared_ptr<EntityManager> entityManager,
              std::shared_ptr<ComponentManager> componentManager,
              std::shared_ptr<SystemManager> systemManager,
              std::shared_ptr<Resources::ResourceManager> resourceManager)
        : Scene(sceneId, entityManager, componentManager, systemManager, resourceManager)
        , m_updateCount(0)
        , m_renderCount(0)
    {
    }
    
protected:
    bool onLoad() override {
        std::cout << "TestScene::onLoad() - " << getSceneId() << std::endl;
        
        // Create some test entities
        auto entity1 = getEntityManager()->createEntity("TestEntity1");
        auto entity2 = getEntityManager()->createEntity("TestEntity2");
        
        std::cout << "Created entities: " << entity1.getID() << ", " << entity2.getID() << std::endl;
        
        // Set some scene properties
        setProperty("background_music", "test_music.ogg");
        setProperty("lighting", "day");
        
        return true;
    }
    
    void onUnload() override {
        std::cout << "TestScene::onUnload() - " << getSceneId() << std::endl;
        m_updateCount = 0;
        m_renderCount = 0;
    }
    
    void onActivate() override {
        std::cout << "TestScene::onActivate() - " << getSceneId() << std::endl;
    }
    
    void onDeactivate() override {
        std::cout << "TestScene::onDeactivate() - " << getSceneId() << std::endl;
    }
    
    void onPause() override {
        std::cout << "TestScene::onPause() - " << getSceneId() << std::endl;
    }
    
    void onResume() override {
        std::cout << "TestScene::onResume() - " << getSceneId() << std::endl;
    }
    
    void onUpdate(float deltaTime) override {
        m_updateCount++;
        
        if (m_updateCount % 10 == 0) {
            std::cout << "TestScene::onUpdate() - " << getSceneId() 
                      << " (updates: " << m_updateCount << ")" << std::endl;
        }
        
        // Test scene transition after some updates
        if (getSceneId() == "main_menu" && m_updateCount == 15) {
            std::cout << "Requesting transition to game scene..." << std::endl;
            requestTransition("game_scene", SceneTransitionType::Fade);
        }
        
        if (getSceneId() == "game_scene" && m_updateCount == 20) {
            std::cout << "Requesting transition to pause menu..." << std::endl;
            requestTransition("pause_menu", SceneTransitionType::Slide);
        }
    }
    
    void onRender(float deltaTime) override {
        m_renderCount++;
        
        if (m_renderCount % 10 == 0) {
            std::cout << "TestScene::onRender() - " << getSceneId() 
                      << " (renders: " << m_renderCount << ")" << std::endl;
        }
    }
    
    void onHandleInput(const std::string& event) override {
        std::cout << "TestScene::onHandleInput() - " << getSceneId() 
                  << " received: " << event << std::endl;
        
        // Test scene stack operations
        if (event == "open_inventory" && getSceneId() == "game_scene") {
            requestTransition("inventory_scene", SceneTransitionType::Immediate);
        }
    }
    
    void onSaveState(SceneData& data) override {
        std::cout << "TestScene::onSaveState() - " << getSceneId() << std::endl;
        
        // Save scene-specific data
        data.properties["update_count"] = std::to_string(m_updateCount);
        data.properties["render_count"] = std::to_string(m_renderCount);
        
        // Save some binary data
        data.binaryData.resize(16);
        for (size_t i = 0; i < data.binaryData.size(); ++i) {
            data.binaryData[i] = static_cast<uint8_t>(i + m_updateCount);
        }
    }
    
    bool onLoadState(const SceneData& data) override {
        std::cout << "TestScene::onLoadState() - " << getSceneId() << std::endl;
        
        // Load scene-specific data
        auto updateCountStr = data.properties.find("update_count");
        if (updateCountStr != data.properties.end()) {
            m_updateCount = std::stoi(updateCountStr->second);
        }
        
        auto renderCountStr = data.properties.find("render_count");
        if (renderCountStr != data.properties.end()) {
            m_renderCount = std::stoi(renderCountStr->second);
        }
        
        std::cout << "Restored counts - Updates: " << m_updateCount 
                  << ", Renders: " << m_renderCount << std::endl;
        
        // Verify binary data
        if (data.binaryData.size() == 16) {
            std::cout << "Binary data restored successfully" << std::endl;
        }
        
        return true;
    }
    
private:
    int m_updateCount;
    int m_renderCount;
};

int main() {
    std::cout << "Scene Framework Test" << std::endl;
    
    // Create managers
    auto entityManager = std::make_shared<EntityManager>();
    auto componentManager = std::make_shared<ComponentManager>();
    auto systemManager = std::make_shared<SystemManager>();
    auto resourceManager = std::make_shared<Resources::ResourceManager>();
    
    // Initialize managers
    entityManager->initialize();
    componentManager->initialize();
    systemManager->initialize();
    resourceManager->initialize();
    
    // Create scene manager
    SceneManager sceneManager(entityManager, componentManager, systemManager, resourceManager);
    
    // Create test scenes
    auto mainMenuScene = std::make_shared<TestScene>("main_menu", entityManager, componentManager, systemManager, resourceManager);
    auto gameScene = std::make_shared<TestScene>("game_scene", entityManager, componentManager, systemManager, resourceManager);
    auto pauseMenuScene = std::make_shared<TestScene>("pause_menu", entityManager, componentManager, systemManager, resourceManager);
    auto inventoryScene = std::make_shared<TestScene>("inventory_scene", entityManager, componentManager, systemManager, resourceManager);
    
    // Register scenes
    std::cout << "\nRegistering scenes..." << std::endl;
    sceneManager.registerScene(mainMenuScene);
    sceneManager.registerScene(gameScene);
    sceneManager.registerScene(pauseMenuScene);
    sceneManager.registerScene(inventoryScene);
    
    // Test scene loading
    std::cout << "\nTesting scene loading..." << std::endl;
    sceneManager.loadScene("main_menu");
    sceneManager.loadScene("game_scene");
    
    std::cout << "Main menu loaded: " << (sceneManager.isSceneLoaded("main_menu") ? "Yes" : "No") << std::endl;
    std::cout << "Game scene loaded: " << (sceneManager.isSceneLoaded("game_scene") ? "Yes" : "No") << std::endl;
    
    // Test scene switching
    std::cout << "\nTesting scene switching..." << std::endl;
    sceneManager.switchToScene("main_menu", SceneTransitionType::Immediate);
    
    std::cout << "Current scene: " << (sceneManager.getCurrentScene() ? sceneManager.getCurrentScene()->getSceneId() : "None") << std::endl;
    std::cout << "Main menu active: " << (sceneManager.isSceneActive("main_menu") ? "Yes" : "No") << std::endl;
    
    // Test scene updates and transitions
    std::cout << "\nTesting scene updates and transitions..." << std::endl;
    
    for (int i = 0; i < 30; i++) {
        sceneManager.update(0.1f);
        sceneManager.render(0.1f);
        
        // Test input handling
        if (i == 10) {
            sceneManager.handleInput("menu_select");
        }
        
        // Show transition progress
        if (sceneManager.isTransitioning()) {
            std::cout << "Transition progress: " << (sceneManager.getTransitionProgress() * 100.0f) << "%" << std::endl;
        }
        
        // Show current scene changes
        static std::string lastSceneId;
        auto currentScene = sceneManager.getCurrentScene();
        if (currentScene && currentScene->getSceneId() != lastSceneId) {
            lastSceneId = currentScene->getSceneId();
            std::cout << "Current scene changed to: " << lastSceneId << std::endl;
        }
    }
    
    // Test scene stack (push/pop)
    std::cout << "\nTesting scene stack..." << std::endl;
    
    std::cout << "Stack size before push: " << sceneManager.getSceneStackSize() << std::endl;
    
    // Push inventory scene
    sceneManager.pushScene("inventory_scene", true);
    std::cout << "Pushed inventory scene" << std::endl;
    std::cout << "Stack size after push: " << sceneManager.getSceneStackSize() << std::endl;
    std::cout << "Current scene: " << sceneManager.getCurrentScene()->getSceneId() << std::endl;
    
    // Update with inventory scene active
    for (int i = 0; i < 5; i++) {
        sceneManager.update(0.1f);
        sceneManager.render(0.1f);
    }
    
    // Pop inventory scene
    sceneManager.popScene();
    std::cout << "Popped inventory scene" << std::endl;
    std::cout << "Stack size after pop: " << sceneManager.getSceneStackSize() << std::endl;
    std::cout << "Current scene: " << sceneManager.getCurrentScene()->getSceneId() << std::endl;
    
    // Test scene state persistence
    std::cout << "\nTesting scene state persistence..." << std::endl;
    
    // Save current scene state
    auto currentScene = sceneManager.getCurrentScene();
    if (currentScene) {
        SceneData savedState = sceneManager.saveSceneState(currentScene->getSceneId());
        std::cout << "Saved state for scene: " << savedState.sceneId << std::endl;
        std::cout << "Properties saved: " << savedState.properties.size() << std::endl;
        std::cout << "Binary data size: " << savedState.binaryData.size() << std::endl;
        
        // Modify scene state
        sceneManager.update(0.1f);
        sceneManager.update(0.1f);
        
        // Restore scene state
        if (sceneManager.loadSceneState(savedState)) {
            std::cout << "Scene state restored successfully" << std::endl;
        }
    }
    
    // Test saving all scene states
    std::cout << "\nTesting save/load all scene states..." << std::endl;
    
    auto allStates = sceneManager.saveAllSceneStates();
    std::cout << "Saved states for " << allStates.size() << " scenes" << std::endl;
    
    // Modify scenes
    for (int i = 0; i < 5; i++) {
        sceneManager.update(0.1f);
    }
    
    // Restore all states
    if (sceneManager.loadAllSceneStates(allStates)) {
        std::cout << "All scene states restored successfully" << std::endl;
    }
    
    // Test scene properties
    std::cout << "\nTesting scene properties..." << std::endl;
    
    if (currentScene) {
        std::cout << "Background music: " << currentScene->getProperty("background_music", "none") << std::endl;
        std::cout << "Lighting: " << currentScene->getProperty("lighting", "none") << std::endl;
        std::cout << "Non-existent property: " << currentScene->getProperty("non_existent", "default_value") << std::endl;
        
        // Set new property
        currentScene->setProperty("player_name", "TestPlayer");
        std::cout << "Player name: " << currentScene->getProperty("player_name") << std::endl;
    }
    
    // Test scene unloading
    std::cout << "\nTesting scene unloading..." << std::endl;
    
    sceneManager.unloadScene("pause_menu");
    std::cout << "Pause menu loaded after unload: " << (sceneManager.isSceneLoaded("pause_menu") ? "Yes" : "No") << std::endl;
    
    // Test transition effects callback
    std::cout << "\nTesting transition effects..." << std::endl;
    
    int transitionCallbacks = 0;
    sceneManager.setTransitionEffectCallback([&transitionCallbacks](const SceneTransition& transition) {
        transitionCallbacks++;
        std::cout << "Transition effect: " << transition.fromSceneId << " -> " << transition.toSceneId 
                  << " (" << (transition.progress * 100.0f) << "%)" << std::endl;
    });
    
    // Trigger a transition
    sceneManager.switchToScene("pause_menu", SceneTransitionType::Fade, 0.5f);
    
    // Update to process transition
    for (int i = 0; i < 10; i++) {
        sceneManager.update(0.1f);
        sceneManager.render(0.1f);
    }
    
    std::cout << "Transition callbacks received: " << transitionCallbacks << std::endl;
    
    // Cleanup
    std::cout << "\nCleaning up..." << std::endl;
    
    sceneManager.clearAllScenes();
    
    systemManager->shutdown();
    componentManager->shutdown();
    entityManager->shutdown();
    resourceManager->shutdown();
    
    std::cout << "\nScene framework test completed successfully!" << std::endl;
    
    return 0;
}