#include "BattleScene.h"
#include <iostream>
#include <algorithm>
#include <random>

namespace RPGEngine {
namespace Scene {

BattleScene::BattleScene(const std::string& sceneId,
                        std::shared_ptr<EntityManager> entityManager,
                        std::shared_ptr<ComponentManager> componentManager,
                        std::shared_ptr<SystemManager> systemManager,
                        std::shared_ptr<Resources::ResourceManager> resourceManager)
    : Scene(sceneId, entityManager, componentManager, systemManager, resourceManager)
    , m_battleState(BattleState::Initializing)
    , m_currentTurnIndex(0)
    , m_turnBasedMode(true)
    , m_actionDelay(1.0f)
    , m_actionTimer(0.0f)
    , m_selectedActionIndex(0)
    , m_selectedTargetIndex(0)
{
    // Initialize available actions
    m_availableActions = {"Attack", "Magic", "Item", "Defend", "Escape"};
}

BattleScene::~BattleScene() {
    // Cleanup handled by base class
}

bool BattleScene::initializeBattle(const std::vector<Entity>& playerParties, const std::vector<Entity>& enemyParties) {
    std::cout << "Initializing battle with " << playerParties.size() 
              << " players and " << enemyParties.size() << " enemies" << std::endl;
    
    // Clear existing participants
    m_participants.clear();
    
    // Add players
    for (size_t i = 0; i < playerParties.size(); ++i) {
        addPlayer(playerParties[i], "Player" + std::to_string(i + 1));
    }
    
    // Add enemies
    for (size_t i = 0; i < enemyParties.size(); ++i) {
        addEnemy(enemyParties[i], "Enemy" + std::to_string(i + 1));
    }
    
    // Initialize turn order
    initializeTurnOrder();
    
    // Set battle state
    m_battleState = BattleState::PlayerTurn;
    m_currentTurnIndex = 0;
    
    std::cout << "Battle initialized successfully" << std::endl;
    return true;
}

void BattleScene::addPlayer(Entity entity, const std::string& name) {
    BattleParticipant participant(entity, name, true);
    
    // TODO: Initialize stats from components
    // For now, use default values
    participant.maxHP = 100.0f;
    participant.currentHP = 100.0f;
    participant.maxMP = 50.0f;
    participant.currentMP = 50.0f;
    
    m_participants.push_back(participant);
    
    std::cout << "Added player to battle: " << name << std::endl;
}

void BattleScene::addEnemy(Entity entity, const std::string& name) {
    BattleParticipant participant(entity, name, false);
    
    // TODO: Initialize stats from components
    // For now, use default values with some variation
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> hpDist(80.0f, 120.0f);
    std::uniform_real_distribution<float> mpDist(30.0f, 60.0f);
    
    participant.maxHP = hpDist(gen);
    participant.currentHP = participant.maxHP;
    participant.maxMP = mpDist(gen);
    participant.currentMP = participant.maxMP;
    
    m_participants.push_back(participant);
    
    std::cout << "Added enemy to battle: " << name << std::endl;
}

void BattleScene::queueAction(const BattleAction& action) {
    m_actionQueue.push(action);
    
    std::cout << "Queued battle action: " << static_cast<int>(action.type) << std::endl;
}

const BattleParticipant* BattleScene::getCurrentTurnParticipant() const {
    if (m_currentTurnIndex >= 0 && m_currentTurnIndex < static_cast<int>(m_participants.size())) {
        return &m_participants[m_currentTurnIndex];
    }
    return nullptr;
}

std::vector<BattleParticipant> BattleScene::getPlayerParticipants() const {
    std::vector<BattleParticipant> players;
    
    for (const auto& participant : m_participants) {
        if (participant.isPlayer) {
            players.push_back(participant);
        }
    }
    
    return players;
}

std::vector<BattleParticipant> BattleScene::getEnemyParticipants() const {
    std::vector<BattleParticipant> enemies;
    
    for (const auto& participant : m_participants) {
        if (!participant.isPlayer) {
            enemies.push_back(participant);
        }
    }
    
    return enemies;
}

bool BattleScene::isBattleOver() const {
    return m_battleState == BattleState::Victory || 
           m_battleState == BattleState::Defeat || 
           m_battleState == BattleState::Escaped;
}

void BattleScene::endBattle(bool victory) {
    m_battleState = victory ? BattleState::Victory : BattleState::Defeat;
    
    std::cout << "Battle ended - " << (victory ? "Victory!" : "Defeat!") << std::endl;
    
    // Stop battle music
    if (m_musicManager) {
        m_musicManager->stopMusic(1.0f);
    }
    
    // Play victory/defeat sound
    if (victory) {
        playBattleSound("victory", 0.8f);
    } else {
        playBattleSound("defeat", 0.8f);
    }
    
    // Call completion callback
    if (m_battleCompletionCallback) {
        m_battleCompletionCallback(victory, m_battleState == BattleState::Escaped);
    }
    
    // Transition back to game scene after a delay
    // TODO: Implement delayed transition
    requestTransition("game_scene", SceneTransitionType::Fade);
}

void BattleScene::setBattleMusic(const std::string& musicId) {
    if (!m_musicManager) {
        std::cerr << "Music manager not set for BattleScene" << std::endl;
        return;
    }
    
    m_musicManager->playMusic(musicId, 0.9f, true, Audio::MusicTransitionType::Crossfade, 1.0f);
    
    std::cout << "Set battle music: " << musicId << std::endl;
}

void BattleScene::playBattleSound(const std::string& soundId, float volume) {
    if (!m_soundManager) {
        return;
    }
    
    m_soundManager->play2DSound(soundId, volume, 1.0f, false, Audio::SoundCategory::Combat);
}

bool BattleScene::onLoad() {
    std::cout << "BattleScene::onLoad() - " << getSceneId() << std::endl;
    
    // Set default properties
    setProperty("scene_type", "battle");
    setProperty("pausable", "true");
    setProperty("turn_based", "true");
    
    return true;
}

void BattleScene::onUnload() {
    std::cout << "BattleScene::onUnload() - " << getSceneId() << std::endl;
    
    // Stop battle music
    if (m_musicManager) {
        m_musicManager->stopMusic(0.5f);
    }
    
    // Clear battle state
    m_participants.clear();
    while (!m_actionQueue.empty()) {
        m_actionQueue.pop();
    }
    
    m_battleState = BattleState::Initializing;
    m_currentTurnIndex = 0;
}

void BattleScene::onActivate() {
    std::cout << "BattleScene::onActivate() - " << getSceneId() << std::endl;
    
    // Resume battle music
    if (m_musicManager && m_musicManager->isPaused()) {
        m_musicManager->resumeMusic(0.3f);
    }
    
    // Set battle music if not already playing
    if (m_musicManager && !m_musicManager->isPlaying()) {
        setBattleMusic("battle_theme");
    }
}

void BattleScene::onDeactivate() {
    std::cout << "BattleScene::onDeactivate() - " << getSceneId() << std::endl;
    
    // Pause battle music
    if (m_musicManager && m_musicManager->isPlaying()) {
        m_musicManager->pauseMusic(0.3f);
    }
}

void BattleScene::onPause() {
    std::cout << "BattleScene::onPause() - " << getSceneId() << std::endl;
    
    // Pause audio
    if (m_musicManager) {
        m_musicManager->pauseMusic(0.2f);
    }
}

void BattleScene::onResume() {
    std::cout << "BattleScene::onResume() - " << getSceneId() << std::endl;
    
    // Resume audio
    if (m_musicManager) {
        m_musicManager->resumeMusic(0.2f);
    }
}

void BattleScene::onUpdate(float deltaTime) {
    // Don't update if battle is over
    if (isBattleOver()) {
        return;
    }
    
    // Update audio managers
    if (m_musicManager) {
        m_musicManager->update(deltaTime);
    }
    if (m_soundManager) {
        m_soundManager->update(deltaTime);
    }
    
    // Update action timer
    m_actionTimer += deltaTime;
    
    // Process action queue
    if (m_actionTimer >= m_actionDelay) {
        processActionQueue();
        m_actionTimer = 0.0f;
    }
    
    // Update battle effects
    updateBattleEffects(deltaTime);
    
    // Check for battle end conditions
    checkBattleEndConditions();
    
    // Process turns
    if (m_battleState == BattleState::EnemyTurn) {
        processNextTurn();
    }
}

void BattleScene::onRender(float deltaTime) {
    // TODO: Render battle scene
    // - Battle background
    // - Participant sprites
    // - UI elements (HP/MP bars, action menu, etc.)
    // - Battle effects and animations
}

void BattleScene::onHandleInput(const std::string& event) {
    // Handle pause input
    if (event == "pause" || event == "escape") {
        if (getProperty("pausable", "true") == "true") {
            requestTransition("pause_menu", SceneTransitionType::Slide);
            return;
        }
    }
    
    // Handle battle input if it's player's turn
    if (m_battleState == BattleState::PlayerTurn) {
        handleBattleInput(event);
    }
}

void BattleScene::onSaveState(SceneData& data) {
    std::cout << "BattleScene::onSaveState() - " << getSceneId() << std::endl;
    
    // Save battle state
    data.properties["battle_state"] = std::to_string(static_cast<int>(m_battleState));
    data.properties["current_turn_index"] = std::to_string(m_currentTurnIndex);
    data.properties["turn_based_mode"] = m_turnBasedMode ? "true" : "false";
    data.properties["action_delay"] = std::to_string(m_actionDelay);
    data.properties["participant_count"] = std::to_string(m_participants.size());
    
    // Save participants
    for (size_t i = 0; i < m_participants.size(); ++i) {
        const auto& participant = m_participants[i];
        std::string prefix = "participant_" + std::to_string(i) + "_";
        
        data.properties[prefix + "name"] = participant.name;
        data.properties[prefix + "is_player"] = participant.isPlayer ? "true" : "false";
        data.properties[prefix + "is_alive"] = participant.isAlive ? "true" : "false";
        data.properties[prefix + "current_hp"] = std::to_string(participant.currentHP);
        data.properties[prefix + "max_hp"] = std::to_string(participant.maxHP);
        data.properties[prefix + "current_mp"] = std::to_string(participant.currentMP);
        data.properties[prefix + "max_mp"] = std::to_string(participant.maxMP);
        data.properties[prefix + "turn_order"] = std::to_string(participant.turnOrder);
    }
}

bool BattleScene::onLoadState(const SceneData& data) {
    std::cout << "BattleScene::onLoadState() - " << getSceneId() << std::endl;
    
    // Load battle state
    auto stateIt = data.properties.find("battle_state");
    if (stateIt != data.properties.end()) {
        m_battleState = static_cast<BattleState>(std::stoi(stateIt->second));
    }
    
    auto turnIt = data.properties.find("current_turn_index");
    if (turnIt != data.properties.end()) {
        m_currentTurnIndex = std::stoi(turnIt->second);
    }
    
    auto turnBasedIt = data.properties.find("turn_based_mode");
    if (turnBasedIt != data.properties.end()) {
        m_turnBasedMode = (turnBasedIt->second == "true");
    }
    
    auto delayIt = data.properties.find("action_delay");
    if (delayIt != data.properties.end()) {
        m_actionDelay = std::stof(delayIt->second);
    }
    
    // Load participants
    auto countIt = data.properties.find("participant_count");
    if (countIt != data.properties.end()) {
        int participantCount = std::stoi(countIt->second);
        
        m_participants.clear();
        
        for (int i = 0; i < participantCount; ++i) {
            std::string prefix = "participant_" + std::to_string(i) + "_";
            
            // Create dummy entity for now
            Entity entity = getEntityManager()->createEntity("BattleParticipant_" + std::to_string(i));
            BattleParticipant participant(entity, "", false);
            
            auto nameIt = data.properties.find(prefix + "name");
            if (nameIt != data.properties.end()) {
                participant.name = nameIt->second;
            }
            
            auto playerIt = data.properties.find(prefix + "is_player");
            if (playerIt != data.properties.end()) {
                participant.isPlayer = (playerIt->second == "true");
            }
            
            auto aliveIt = data.properties.find(prefix + "is_alive");
            if (aliveIt != data.properties.end()) {
                participant.isAlive = (aliveIt->second == "true");
            }
            
            auto currentHpIt = data.properties.find(prefix + "current_hp");
            if (currentHpIt != data.properties.end()) {
                participant.currentHP = std::stof(currentHpIt->second);
            }
            
            auto maxHpIt = data.properties.find(prefix + "max_hp");
            if (maxHpIt != data.properties.end()) {
                participant.maxHP = std::stof(maxHpIt->second);
            }
            
            auto currentMpIt = data.properties.find(prefix + "current_mp");
            if (currentMpIt != data.properties.end()) {
                participant.currentMP = std::stof(currentMpIt->second);
            }
            
            auto maxMpIt = data.properties.find(prefix + "max_mp");
            if (maxMpIt != data.properties.end()) {
                participant.maxMP = std::stof(maxMpIt->second);
            }
            
            auto turnOrderIt = data.properties.find(prefix + "turn_order");
            if (turnOrderIt != data.properties.end()) {
                participant.turnOrder = std::stoi(turnOrderIt->second);
            }
            
            m_participants.push_back(participant);
        }
    }
    
    return true;
}

void BattleScene::initializeTurnOrder() {
    // TODO: Calculate turn order based on speed/agility stats
    // For now, just assign sequential order
    for (size_t i = 0; i < m_participants.size(); ++i) {
        m_participants[i].turnOrder = static_cast<int>(i);
    }
    
    // Sort participants by turn order
    std::sort(m_participants.begin(), m_participants.end(),
        [](const BattleParticipant& a, const BattleParticipant& b) {
            return a.turnOrder < b.turnOrder;
        });
    
    std::cout << "Initialized turn order for " << m_participants.size() << " participants" << std::endl;
}

void BattleScene::processNextTurn() {
    if (m_participants.empty()) {
        return;
    }
    
    // Find next alive participant
    int startIndex = m_currentTurnIndex;
    do {
        m_currentTurnIndex = (m_currentTurnIndex + 1) % static_cast<int>(m_participants.size());
        
        if (m_participants[m_currentTurnIndex].isAlive) {
            break;
        }
        
        // Prevent infinite loop
        if (m_currentTurnIndex == startIndex) {
            break;
        }
    } while (true);
    
    const auto& currentParticipant = m_participants[m_currentTurnIndex];
    
    if (currentParticipant.isPlayer) {
        m_battleState = BattleState::PlayerTurn;
        std::cout << "Player turn: " << currentParticipant.name << std::endl;
    } else {
        m_battleState = BattleState::EnemyTurn;
        std::cout << "Enemy turn: " << currentParticipant.name << std::endl;
        
        // Process AI turn
        processAITurn(currentParticipant);
    }
}

void BattleScene::processActionQueue() {
    if (m_actionQueue.empty()) {
        return;
    }
    
    m_battleState = BattleState::Processing;
    
    while (!m_actionQueue.empty()) {
        BattleAction action = m_actionQueue.front();
        m_actionQueue.pop();
        
        executeBattleAction(action);
    }
    
    // Move to next turn
    processNextTurn();
}

void BattleScene::executeBattleAction(const BattleAction& action) {
    std::cout << "Executing battle action: " << static_cast<int>(action.type) << std::endl;
    
    auto* actor = getParticipant(action.actor);
    if (!actor || !actor->isAlive) {
        return;
    }
    
    switch (action.type) {
        case BattleActionType::Attack: {
            auto* target = getParticipant(action.target);
            if (target && target->isAlive) {
                float damage = calculateDamage(action);
                applyDamage(*target, damage);
                
                playBattleSound("attack", 0.8f);
                std::cout << actor->name << " attacks " << target->name << " for " << damage << " damage!" << std::endl;
            }
            break;
        }
        
        case BattleActionType::Magic: {
            // TODO: Implement magic system
            playBattleSound("magic", 0.7f);
            std::cout << actor->name << " casts " << action.actionId << "!" << std::endl;
            break;
        }
        
        case BattleActionType::Item: {
            // TODO: Implement item usage
            playBattleSound("item", 0.6f);
            std::cout << actor->name << " uses " << action.actionId << "!" << std::endl;
            break;
        }
        
        case BattleActionType::Defend: {
            playBattleSound("defend", 0.5f);
            std::cout << actor->name << " defends!" << std::endl;
            break;
        }
        
        case BattleActionType::Escape: {
            // TODO: Calculate escape chance
            playBattleSound("escape", 0.6f);
            std::cout << actor->name << " attempts to escape!" << std::endl;
            
            if (actor->isPlayer) {
                m_battleState = BattleState::Escaped;
            }
            break;
        }
    }
}

float BattleScene::calculateDamage(const BattleAction& action) {
    // TODO: Implement proper damage calculation based on stats
    // For now, return random damage between 10-30
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> damageDist(10.0f, 30.0f);
    
    return damageDist(gen);
}

void BattleScene::applyDamage(BattleParticipant& participant, float damage) {
    participant.currentHP = std::max(0.0f, participant.currentHP - damage);
    
    if (participant.currentHP <= 0.0f) {
        participant.isAlive = false;
        std::cout << participant.name << " has been defeated!" << std::endl;
    }
}

void BattleScene::checkBattleEndConditions() {
    if (isBattleOver()) {
        return;
    }
    
    // Check if all players are defeated
    bool allPlayersDead = true;
    for (const auto& participant : m_participants) {
        if (participant.isPlayer && participant.isAlive) {
            allPlayersDead = false;
            break;
        }
    }
    
    if (allPlayersDead) {
        endBattle(false);
        return;
    }
    
    // Check if all enemies are defeated
    bool allEnemiesDead = true;
    for (const auto& participant : m_participants) {
        if (!participant.isPlayer && participant.isAlive) {
            allEnemiesDead = false;
            break;
        }
    }
    
    if (allEnemiesDead) {
        endBattle(true);
        return;
    }
}

void BattleScene::handleBattleInput(const std::string& event) {
    // Handle action selection
    if (event == "up" || event == "w") {
        m_selectedActionIndex = (m_selectedActionIndex - 1 + static_cast<int>(m_availableActions.size())) % static_cast<int>(m_availableActions.size());
        playBattleSound("menu_select", 0.5f);
    } else if (event == "down" || event == "s") {
        m_selectedActionIndex = (m_selectedActionIndex + 1) % static_cast<int>(m_availableActions.size());
        playBattleSound("menu_select", 0.5f);
    } else if (event == "select" || event == "enter" || event == "space") {
        // Execute selected action
        const auto* currentParticipant = getCurrentTurnParticipant();
        if (currentParticipant) {
            BattleAction action(currentParticipant->entity, static_cast<BattleActionType>(m_selectedActionIndex));
            
            // For attack actions, select first alive enemy as target
            if (action.type == BattleActionType::Attack) {
                for (const auto& participant : m_participants) {
                    if (!participant.isPlayer && participant.isAlive) {
                        action.target = participant.entity;
                        break;
                    }
                }
            }
            
            queueAction(action);
        }
    }
}

void BattleScene::processAITurn(const BattleParticipant& participant) {
    // Simple AI: always attack first alive player
    BattleAction action(participant.entity, BattleActionType::Attack);
    
    for (const auto& target : m_participants) {
        if (target.isPlayer && target.isAlive) {
            action.target = target.entity;
            break;
        }
    }
    
    queueAction(action);
}

BattleParticipant* BattleScene::getParticipant(Entity entity) {
    for (auto& participant : m_participants) {
        if (participant.entity.getID() == entity.getID()) {
            return &participant;
        }
    }
    return nullptr;
}

void BattleScene::updateBattleEffects(float deltaTime) {
    // TODO: Update battle animations, particle effects, etc.
    // For now, just a placeholder
}

} // namespace Scene
} // namespace RPGEngine