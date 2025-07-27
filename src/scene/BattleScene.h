#pragma once

#include "Scene.h"
#include "../audio/MusicManager.h"
#include "../audio/SoundEffectManager.h"
#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <functional>

namespace RPGEngine {
namespace Scene {

/**
 * Battle state enumeration
 */
enum class BattleState {
    Initializing,   // Battle is being set up
    PlayerTurn,     // Player's turn to act
    EnemyTurn,      // Enemy's turn to act
    Processing,     // Processing actions
    Victory,        // Player won
    Defeat,         // Player lost
    Escaped         // Player escaped
};

/**
 * Battle action type
 */
enum class BattleActionType {
    Attack,         // Physical attack
    Magic,          // Magic spell
    Item,           // Use item
    Defend,         // Defend/guard
    Escape          // Attempt to escape
};

/**
 * Battle action structure
 */
struct BattleAction {
    Entity actor;                   // Entity performing the action
    BattleActionType type;          // Type of action
    Entity target;                  // Target entity (if applicable)
    std::string actionId;           // Specific action/skill/item ID
    float damage;                   // Calculated damage/effect
    bool success;                   // Whether action succeeded
    std::string description;        // Action description for UI
    
    BattleAction(Entity a, BattleActionType t, const std::string& id = "")
        : actor(a), type(t), actionId(id), damage(0.0f), success(false) {}
};

/**
 * Battle participant structure
 */
struct BattleParticipant {
    Entity entity;
    std::string name;
    bool isPlayer;
    bool isAlive;
    float currentHP;
    float maxHP;
    float currentMP;
    float maxMP;
    int turnOrder;
    
    BattleParticipant(Entity e, const std::string& n, bool player = false)
        : entity(e), name(n), isPlayer(player), isAlive(true), 
          currentHP(100.0f), maxHP(100.0f), currentMP(50.0f), maxMP(50.0f), turnOrder(0) {}
};

/**
 * Battle scene
 * Handles turn-based combat encounters
 */
class BattleScene : public Scene {
public:
    /**
     * Constructor
     * @param sceneId Scene identifier
     * @param entityManager Entity manager
     * @param componentManager Component manager
     * @param systemManager System manager
     * @param resourceManager Resource manager
     */
    BattleScene(const std::string& sceneId,
                std::shared_ptr<EntityManager> entityManager,
                std::shared_ptr<ComponentManager> componentManager,
                std::shared_ptr<SystemManager> systemManager,
                std::shared_ptr<Resources::ResourceManager> resourceManager);
    
    /**
     * Destructor
     */
    ~BattleScene();
    
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
     * Initialize battle
     * @param playerParties Player entities
     * @param enemyParties Enemy entities
     * @return true if battle was initialized successfully
     */
    bool initializeBattle(const std::vector<Entity>& playerParties, const std::vector<Entity>& enemyParties);
    
    /**
     * Add player to battle
     * @param entity Player entity
     * @param name Player name
     */
    void addPlayer(Entity entity, const std::string& name);
    
    /**
     * Add enemy to battle
     * @param entity Enemy entity
     * @param name Enemy name
     */
    void addEnemy(Entity entity, const std::string& name);
    
    /**
     * Queue battle action
     * @param action Battle action to queue
     */
    void queueAction(const BattleAction& action);
    
    /**
     * Get current battle state
     * @return Battle state
     */
    BattleState getBattleState() const { return m_battleState; }
    
    /**
     * Get current turn participant
     * @return Current participant, or nullptr if no active turn
     */
    const BattleParticipant* getCurrentTurnParticipant() const;
    
    /**
     * Get all participants
     * @return Vector of all battle participants
     */
    const std::vector<BattleParticipant>& getParticipants() const { return m_participants; }
    
    /**
     * Get player participants
     * @return Vector of player participants
     */
    std::vector<BattleParticipant> getPlayerParticipants() const;
    
    /**
     * Get enemy participants
     * @return Vector of enemy participants
     */
    std::vector<BattleParticipant> getEnemyParticipants() const;
    
    /**
     * Check if battle is over
     * @return true if battle has ended
     */
    bool isBattleOver() const;
    
    /**
     * End battle
     * @param victory Whether player won
     */
    void endBattle(bool victory);
    
    /**
     * Set battle completion callback
     * @param callback Callback function (victory, escaped)
     */
    void setBattleCompletionCallback(std::function<void(bool, bool)> callback) {
        m_battleCompletionCallback = callback;
    }
    
    /**
     * Set background music
     * @param musicId Music resource ID
     */
    void setBattleMusic(const std::string& musicId);
    
    /**
     * Play battle sound effect
     * @param soundId Sound resource ID
     * @param volume Volume (0.0 to 1.0)
     */
    void playBattleSound(const std::string& soundId, float volume = 1.0f);
    
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
     * Initialize turn order
     */
    void initializeTurnOrder();
    
    /**
     * Process next turn
     */
    void processNextTurn();
    
    /**
     * Process action queue
     */
    void processActionQueue();
    
    /**
     * Execute battle action
     * @param action Action to execute
     */
    void executeBattleAction(const BattleAction& action);
    
    /**
     * Calculate damage for action
     * @param action Battle action
     * @return Calculated damage
     */
    float calculateDamage(const BattleAction& action);
    
    /**
     * Apply damage to participant
     * @param participant Target participant
     * @param damage Damage amount
     */
    void applyDamage(BattleParticipant& participant, float damage);
    
    /**
     * Check for battle end conditions
     */
    void checkBattleEndConditions();
    
    /**
     * Handle player input during battle
     * @param event Input event
     */
    void handleBattleInput(const std::string& event);
    
    /**
     * Process AI turn for enemy
     * @param participant Enemy participant
     */
    void processAITurn(const BattleParticipant& participant);
    
    /**
     * Get participant by entity
     * @param entity Entity to find
     * @return Participant, or nullptr if not found
     */
    BattleParticipant* getParticipant(Entity entity);
    
    /**
     * Update battle animations and effects
     * @param deltaTime Time since last update
     */
    void updateBattleEffects(float deltaTime);
    
    // Managers
    std::shared_ptr<Audio::MusicManager> m_musicManager;
    std::shared_ptr<Audio::SoundEffectManager> m_soundManager;
    
    // Battle state
    BattleState m_battleState;
    std::vector<BattleParticipant> m_participants;
    std::queue<BattleAction> m_actionQueue;
    int m_currentTurnIndex;
    
    // Battle settings
    bool m_turnBasedMode;
    float m_actionDelay;
    float m_actionTimer;
    
    // Callbacks
    std::function<void(bool, bool)> m_battleCompletionCallback;
    
    // Battle UI state
    int m_selectedActionIndex;
    int m_selectedTargetIndex;
    std::vector<std::string> m_availableActions;
};

} // namespace Scene
} // namespace RPGEngine