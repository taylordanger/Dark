#include "GameScene.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Scene {

GameScene::GameScene(const std::string& sceneId,
                    std::shared_ptr<EntityManager> entityManager,
                    std::shared_ptr<ComponentManager> componentManager,
                    std::shared_ptr<SystemManager> systemManager,
                    std::shared_ptr<Resources::ResourceManager> resourceManager)
    : Scene(sceneId, entityManager, componentManager, systemManager, resourceManager)
    , m_playerEntity(Entity())
    , m_gamePaused(false)
    , m_cameraFollowSpeed(5.0f)
    , m_cameraFollowEnabled(true)
{
}

GameScene::~GameScene() {
    // Cleanup handled by base class
}

bool GameScene::loadMap(const std::string& mapId) {
    if (!m_worldManager) {
        std::cerr << "World manager not set for GameScene" << std::endl;
        return false;
    }
    
    // Load the map through world manager
    if (!m_worldManager->loadMap(mapId)) {
        std::cerr << "Failed to load map: " << mapId << std::endl;
        return false;
    }
    
    m_currentMapId = mapId;
    
    std::cout << "GameScene loaded map: " << mapId << std::endl;
    return true;
}

Entity GameScene::createPlayer(const std::string& name, float x, float y) {
    // Create player entity
    m_playerEntity = getEntityManager()->createEntity("Player_" + name);
    
    // TODO: Add player components (transform, sprite, stats, etc.)
    // This would require the actual component implementations
    
    // Track as game object
    addGameObject(m_playerEntity, "player");
    
    std::cout << "Created player: " << name << " at (" << x << ", " << y << ")" << std::endl;
    
    return m_playerEntity;
}

Entity GameScene::spawnNPC(const std::string& npcId, float x, float y) {
    // Create NPC entity
    Entity npcEntity = getEntityManager()->createEntity("NPC_" + npcId);
    
    // TODO: Add NPC components based on npcId
    
    // Track as game object
    addGameObject(npcEntity, "npc");
    
    std::cout << "Spawned NPC: " << npcId << " at (" << x << ", " << y << ")" << std::endl;
    
    return npcEntity;
}

Entity GameScene::spawnItem(const std::string& itemId, float x, float y) {
    // Create item entity
    Entity itemEntity = getEntityManager()->createEntity("Item_" + itemId);
    
    // TODO: Add item components based on itemId
    
    // Track as game object
    addGameObject(itemEntity, "item");
    
    std::cout << "Spawned item: " << itemId << " at (" << x << ", " << y << ")" << std::endl;
    
    return itemEntity;
}

void GameScene::setBackgroundMusic(const std::string& musicId, bool fadeIn) {
    if (!m_musicManager) {
        std::cerr << "Music manager not set for GameScene" << std::endl;
        return;
    }
    
    if (fadeIn) {
        m_musicManager->playMusic(musicId, 0.8f, true, Audio::MusicTransitionType::FadeIn, 2.0f);
    } else {
        m_musicManager->playMusic(musicId, 0.8f, true, Audio::MusicTransitionType::Immediate);
    }
    
    std::cout << "Set background music: " << musicId << std::endl;
}

void GameScene::playSoundAt(const std::string& soundId, float x, float y, float volume) {
    if (!m_soundManager) {
        std::cerr << "Sound manager not set for GameScene" << std::endl;
        return;
    }
    
    m_soundManager->play3DSound(soundId, x, y, 0.0f, volume, 1.0f, false, Audio::SoundCategory::Action);
    
    std::cout << "Played sound: " << soundId << " at (" << x << ", " << y << ")" << std::endl;
}

void GameScene::addGameObject(Entity entity, const std::string& objectType) {
    m_gameObjects[objectType].push_back(entity);
}

void GameScene::removeGameObject(Entity entity) {
    for (auto& pair : m_gameObjects) {
        auto& objects = pair.second;
        objects.erase(std::remove_if(objects.begin(), objects.end(),
            [entity](const Entity& e) { return e.getID() == entity.getID(); }), objects.end());
    }
}

std::vector<Entity> GameScene::getGameObjectsByType(const std::string& objectType) const {
    auto it = m_gameObjects.find(objectType);
    return (it != m_gameObjects.end()) ? it->second : std::vector<Entity>();
}

bool GameScene::onLoad() {
    std::cout << "GameScene::onLoad() - " << getSceneId() << std::endl;
    
    // Initialize game systems
    initializeGameSystems();
    
    // Set default properties
    setProperty("scene_type", "game");
    setProperty("allow_pause", "true");
    setProperty("camera_follow", "true");
    
    return true;
}

void GameScene::onUnload() {
    std::cout << "GameScene::onUnload() - " << getSceneId() << std::endl;
    
    // Stop background music
    if (m_musicManager) {
        m_musicManager->stopMusic(1.0f);
    }
    
    // Stop all sound effects
    if (m_soundManager) {
        m_soundManager->stopAllSounds(0.5f);
    }
    
    // Clear game objects
    m_gameObjects.clear();
    m_playerEntity = Entity();
    m_currentMapId.clear();
}

void GameScene::onActivate() {
    std::cout << "GameScene::onActivate() - " << getSceneId() << std::endl;
    
    // Resume background music if it was playing
    if (m_musicManager && m_musicManager->isPaused()) {
        m_musicManager->resumeMusic(0.5f);
    }
    
    // Resume game
    setGamePaused(false);
}

void GameScene::onDeactivate() {
    std::cout << "GameScene::onDeactivate() - " << getSceneId() << std::endl;
    
    // Pause background music
    if (m_musicManager && m_musicManager->isPlaying()) {
        m_musicManager->pauseMusic(0.5f);
    }
    
    // Pause game
    setGamePaused(true);
}

void GameScene::onPause() {
    std::cout << "GameScene::onPause() - " << getSceneId() << std::endl;
    
    // Pause game systems
    setGamePaused(true);
    
    // Pause audio
    if (m_musicManager) {
        m_musicManager->pauseMusic(0.2f);
    }
}

void GameScene::onResume() {
    std::cout << "GameScene::onResume() - " << getSceneId() << std::endl;
    
    // Resume game systems
    setGamePaused(false);
    
    // Resume audio
    if (m_musicManager) {
        m_musicManager->resumeMusic(0.2f);
    }
}

void GameScene::onUpdate(float deltaTime) {
    // Don't update if game is paused
    if (m_gamePaused) {
        return;
    }
    
    // Update world manager
    if (m_worldManager) {
        m_worldManager->update(deltaTime);
    }
    
    // Update audio managers
    if (m_musicManager) {
        m_musicManager->update(deltaTime);
    }
    if (m_soundManager) {
        m_soundManager->update(deltaTime);
    }
    
    // Update camera follow
    if (m_cameraFollowEnabled) {
        updateCameraFollow();
    }
    
    // Update game objects
    updateGameObjects(deltaTime);
}

void GameScene::onRender(float deltaTime) {
    // Render world
    if (m_worldManager) {
        // TODO: Implement world rendering
        // m_worldManager->render(deltaTime);
    }
    
    // TODO: Render game objects, UI, etc.
}

void GameScene::onHandleInput(const std::string& event) {
    // Handle pause input
    if (event == "pause" || event == "escape") {
        if (getProperty("allow_pause", "true") == "true") {
            requestTransition("pause_menu", SceneTransitionType::Slide);
            return;
        }
    }
    
    // Handle inventory input
    if (event == "inventory" || event == "i") {
        requestTransition("inventory_scene", SceneTransitionType::Immediate);
        return;
    }
    
    // Handle player input if not paused
    if (!m_gamePaused) {
        handlePlayerInput(event);
    }
}

void GameScene::onSaveState(SceneData& data) {
    std::cout << "GameScene::onSaveState() - " << getSceneId() << std::endl;
    
    // Save current map
    data.properties["current_map"] = m_currentMapId;
    data.properties["game_paused"] = m_gamePaused ? "true" : "false";
    data.properties["camera_follow_enabled"] = m_cameraFollowEnabled ? "true" : "false";
    data.properties["camera_follow_speed"] = std::to_string(m_cameraFollowSpeed);
    
    // Save player entity ID
    if (m_playerEntity.isValid()) {
        data.properties["player_entity_id"] = std::to_string(m_playerEntity.getID());
    }
    
    // Save game object counts
    for (const auto& pair : m_gameObjects) {
        data.properties["object_count_" + pair.first] = std::to_string(pair.second.size());
    }
    
    // TODO: Save more detailed game state (player position, NPC states, etc.)
}

bool GameScene::onLoadState(const SceneData& data) {
    std::cout << "GameScene::onLoadState() - " << getSceneId() << std::endl;
    
    // Load current map
    auto mapIt = data.properties.find("current_map");
    if (mapIt != data.properties.end() && !mapIt->second.empty()) {
        loadMap(mapIt->second);
    }
    
    // Load game state
    auto pausedIt = data.properties.find("game_paused");
    if (pausedIt != data.properties.end()) {
        m_gamePaused = (pausedIt->second == "true");
    }
    
    auto cameraFollowIt = data.properties.find("camera_follow_enabled");
    if (cameraFollowIt != data.properties.end()) {
        m_cameraFollowEnabled = (cameraFollowIt->second == "true");
    }
    
    auto cameraSpeedIt = data.properties.find("camera_follow_speed");
    if (cameraSpeedIt != data.properties.end()) {
        m_cameraFollowSpeed = std::stof(cameraSpeedIt->second);
    }
    
    // TODO: Restore more detailed game state
    
    return true;
}

void GameScene::initializeGameSystems() {
    std::cout << "Initializing game systems for scene: " << getSceneId() << std::endl;
    
    // TODO: Initialize game-specific systems
    // - Physics system
    // - Animation system
    // - AI system
    // - Combat system
    // etc.
}

void GameScene::updateCameraFollow() {
    if (!m_mainCamera || !m_playerEntity.isValid()) {
        return;
    }
    
    // TODO: Implement camera following logic
    // This would require transform components and camera positioning
    // For now, just log that we're updating camera follow
    static int updateCount = 0;
    if (++updateCount % 60 == 0) { // Log every 60 updates
        std::cout << "Updating camera follow for player" << std::endl;
    }
}

void GameScene::handlePlayerInput(const std::string& event) {
    if (!m_playerEntity.isValid()) {
        return;
    }
    
    // TODO: Handle player movement and actions
    // This would require input mapping and player controller components
    
    std::cout << "Player input: " << event << std::endl;
    
    // Example input handling
    if (event == "move_up" || event == "w") {
        // Move player up
        playSoundAt("footstep", 0.0f, 0.0f, 0.5f);
    } else if (event == "move_down" || event == "s") {
        // Move player down
        playSoundAt("footstep", 0.0f, 0.0f, 0.5f);
    } else if (event == "move_left" || event == "a") {
        // Move player left
        playSoundAt("footstep", 0.0f, 0.0f, 0.5f);
    } else if (event == "move_right" || event == "d") {
        // Move player right
        playSoundAt("footstep", 0.0f, 0.0f, 0.5f);
    } else if (event == "interact" || event == "space") {
        // Interact with objects
        playSoundAt("interact", 0.0f, 0.0f, 0.8f);
    }
}

void GameScene::updateGameObjects(float deltaTime) {
    // Update all tracked game objects
    for (const auto& pair : m_gameObjects) {
        const auto& objects = pair.second;
        
        // TODO: Update object-specific logic
        // This would involve component systems
        
        // For now, just validate that entities are still valid
        for (const auto& entity : objects) {
            if (!entity.isValid()) {
                std::cerr << "Invalid entity found in game objects: " << pair.first << std::endl;
            }
        }
    }
}

} // namespace Scene
} // namespace RPGEngine