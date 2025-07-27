#pragma once

#include "Scene.h"
#include "../world/WorldManager.h"
#include "../audio/MusicManager.h"
#include "../audio/SoundEffectManager.h"
#include "../graphics/Camera.h"
#include <string>
#include <memory>
#include <vector>

namespace RPGEngine {
namespace Scene {

/**
 * Game scene
 * Main gameplay scene with world management, player control, and game systems
 */
class GameScene : public Scene {
public:
    /**
     * Constructor
     * @param sceneId Scene identifier
     * @param entityManager Entity manager
     * @param componentManager Component manager
     * @param systemManager System manager
     * @param resourceManager Resource manager
     */
    GameScene(const std::string& sceneId,
              std::shared_ptr<EntityManager> entityManager,
              std::shared_ptr<ComponentManager> componentManager,
              std::shared_ptr<SystemManager> systemManager,
              std::shared_ptr<Resources::ResourceManager> resourceManager);
    
    /**
     * Destructor
     */
    ~GameScene();
    
    /**
     * Set world manager
     * @param worldManager World manager
     */
    void setWorldManager(std::shared_ptr<World::WorldManager> worldManager) {
        m_worldManager = worldManager;
    }
    
    /**
     * Set music manager
     * @param musicManager Music manager
     */
    void setMusicManager(std::shared_ptr<Audio::MusicManager> musicManager) {
        m_musicManager = musicManager;
    }
    
    /**
     * Set sound effect manager
     * @param soundManager Sound effect manager
     */
    void setSoundEffectManager(std::shared_ptr<Audio::SoundEffectManager> soundManager) {
        m_soundManager = soundManager;
    }
    
    /**
     * Set main camera
     * @param camera Main camera
     */
    void setMainCamera(std::shared_ptr<Graphics::Camera> camera) {
        m_mainCamera = camera;
    }
    
    /**
     * Load map
     * @param mapId Map identifier
     * @return true if map was loaded successfully
     */
    bool loadMap(const std::string& mapId);
    
    /**
     * Get current map ID
     * @return Current map ID
     */
    const std::string& getCurrentMapId() const { return m_currentMapId; }
    
    /**
     * Create player entity
     * @param name Player name
     * @param x Starting X position
     * @param y Starting Y position
     * @return Player entity
     */
    Entity createPlayer(const std::string& name, float x, float y);
    
    /**
     * Get player entity
     * @return Player entity, or invalid entity if not found
     */
    Entity getPlayer() const { return m_playerEntity; }
    
    /**
     * Spawn NPC
     * @param npcId NPC identifier
     * @param x X position
     * @param y Y position
     * @return NPC entity
     */
    Entity spawnNPC(const std::string& npcId, float x, float y);
    
    /**
     * Spawn item
     * @param itemId Item identifier
     * @param x X position
     * @param y Y position
     * @return Item entity
     */
    Entity spawnItem(const std::string& itemId, float x, float y);
    
    /**
     * Set background music
     * @param musicId Music resource ID
     * @param fadeIn Whether to fade in the music
     */
    void setBackgroundMusic(const std::string& musicId, bool fadeIn = true);
    
    /**
     * Play sound effect at position
     * @param soundId Sound resource ID
     * @param x X position
     * @param y Y position
     * @param volume Volume (0.0 to 1.0)
     */
    void playSoundAt(const std::string& soundId, float x, float y, float volume = 1.0f);
    
    /**
     * Set game paused state
     * @param paused Whether the game is paused
     */
    void setGamePaused(bool paused) { m_gamePaused = paused; }
    
    /**
     * Check if game is paused
     * @return true if game is paused
     */
    bool isGamePaused() const { return m_gamePaused; }
    
    /**
     * Add game object to track
     * @param entity Entity to track
     * @param objectType Object type for categorization
     */
    void addGameObject(Entity entity, const std::string& objectType);
    
    /**
     * Remove game object
     * @param entity Entity to remove
     */
    void removeGameObject(Entity entity);
    
    /**
     * Get game objects by type
     * @param objectType Object type
     * @return Vector of entities of the specified type
     */
    std::vector<Entity> getGameObjectsByType(const std::string& objectType) const;
    
protected:
    // Scene lifecycle overrides
    bool onLoad() override;
    void onUnload() override;
    void onActivate() override;
    void onDeactivate() override;
    void onPause() override;
    void onResume() override;
    void onUpdate(float deltaTime) override;
    void onRender(float deltaTime) override;
    void onHandleInput(const std::string& event) override;
    void onSaveState(SceneData& data) override;
    bool onLoadState(const SceneData& data) override;
    
private:
    /**
     * Initialize game systems
     */
    void initializeGameSystems();
    
    /**
     * Update camera to follow player
     */
    void updateCameraFollow();
    
    /**
     * Handle player input
     * @param event Input event
     */
    void handlePlayerInput(const std::string& event);
    
    /**
     * Update game objects
     * @param deltaTime Time since last update
     */
    void updateGameObjects(float deltaTime);
    
    // Managers
    std::shared_ptr<World::WorldManager> m_worldManager;
    std::shared_ptr<Audio::MusicManager> m_musicManager;
    std::shared_ptr<Audio::SoundEffectManager> m_soundManager;
    std::shared_ptr<Graphics::Camera> m_mainCamera;
    
    // Game state
    std::string m_currentMapId;
    Entity m_playerEntity;
    bool m_gamePaused;
    
    // Game objects tracking
    std::unordered_map<std::string, std::vector<Entity>> m_gameObjects;
    
    // Game settings
    float m_cameraFollowSpeed;
    bool m_cameraFollowEnabled;
};

} // namespace Scene
} // namespace RPGEngine