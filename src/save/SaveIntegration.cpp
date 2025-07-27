#include "SaveIntegration.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
#include "../components/TransformComponent.h"
#include "../components/QuestComponent.h"
#include "../components/DialogueComponent.h"
#include "../components/CombatComponent.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

// Forward declarations to avoid circular dependencies
namespace RPGEngine {
    class Entity;
    class EntityManager;
    class ComponentManager;
    class SystemManager;
    
    namespace Scene {
        class SceneManager;
        class Scene;
        struct SceneData;
    }
    
    namespace World {
        class WorldManager;
    }
    
    namespace Audio {
        class MusicManager;
        class SoundEffectManager;
    }
}

namespace Engine {
    namespace Save {
        
        SaveIntegration::SaveIntegration()
            : m_forceAutoSave(false)
            , m_initialized(false)
        {
            m_lastAutoSave = std::chrono::steady_clock::now();
        }
        
        SaveIntegration::~SaveIntegration() {
            shutdown();
        }
        
        bool SaveIntegration::initialize(std::shared_ptr<SaveManager> saveManager,
                                       std::shared_ptr<RPGEngine::EntityManager> entityManager,
                                       std::shared_ptr<RPGEngine::ComponentManager> componentManager,
                                       std::shared_ptr<RPGEngine::SystemManager> systemManager,
                                       std::shared_ptr<RPGEngine::Scene::SceneManager> sceneManager,
                                       std::shared_ptr<RPGEngine::World::WorldManager> worldManager) {
            
            if (m_initialized) {
                return true;
            }
            
            if (!saveManager || !entityManager || !componentManager || 
                !systemManager || !sceneManager || !worldManager) {
                setError("Invalid manager provided to SaveIntegration");
                return false;
            }
            
            m_saveManager = saveManager;
            m_entityManager = entityManager;
            m_componentManager = componentManager;
            m_systemManager = systemManager;
            m_sceneManager = sceneManager;
            m_worldManager = worldManager;
            
            // Initialize save manager if not already initialized
            if (!m_saveManager->initialize()) {
                setError("Failed to initialize SaveManager: " + m_saveManager->getLastError());
                return false;
            }
            
            m_initialized = true;
            clearError();
            return true;
        }
        
        void SaveIntegration::shutdown() {
            m_saveManager.reset();
            m_entityManager.reset();
            m_componentManager.reset();
            m_systemManager.reset();
            m_sceneManager.reset();
            m_worldManager.reset();
            m_musicManager.reset();
            m_soundManager.reset();
            
            m_systemSerializers.clear();
            m_systemDeserializers.clear();
            
            m_initialized = false;
        }
        
        void SaveIntegration::update(float deltaTime) {
            if (!m_initialized || !m_autoSaveConfig.enabled) {
                return;
            }
            
            if (shouldAutoSave() || m_forceAutoSave) {
                SaveIntegrationResult result = autoSave();
                
                if (m_callbacks.onAutoSave) {
                    if (result == SaveIntegrationResult::Success) {
                        m_callbacks.onAutoSave("Auto-save completed successfully");
                    } else {
                        m_callbacks.onAutoSave("Auto-save failed: " + getLastError());
                    }
                }
                
                m_forceAutoSave = false;
                resetAutoSaveTimer();
            }
        }
        
        SaveIntegrationResult SaveIntegration::saveGameState(const std::string& slotName) {
            if (!m_initialized) {
                setError("SaveIntegration not initialized");
                return SaveIntegrationResult::SystemError;
            }
            
            GameStateData gameState;
            if (!collectGameState(gameState)) {
                return SaveIntegrationResult::DataError;
            }
            
            SaveResult result = m_saveManager->saveGame(gameState.saveData, slotName);
            
            SaveIntegrationResult integrationResult;
            switch (result) {
                case SaveResult::Success:
                    integrationResult = SaveIntegrationResult::Success;
                    break;
                case SaveResult::ValidationError:
                    integrationResult = SaveIntegrationResult::ValidationError;
                    break;
                case SaveResult::FileError:
                    integrationResult = SaveIntegrationResult::FileError;
                    break;
                default:
                    integrationResult = SaveIntegrationResult::SystemError;
                    break;
            }
            
            if (m_callbacks.onSaveComplete) {
                m_callbacks.onSaveComplete(integrationResult);
            }
            
            return integrationResult;
        }
        
        SaveIntegrationResult SaveIntegration::saveGameState(int slotNumber) {
            return saveGameState("slot_" + std::to_string(slotNumber));
        }
        
        LoadResult SaveIntegration::loadGameState(const std::string& slotName) {
            if (!m_initialized) {
                setError("SaveIntegration not initialized");
                return LoadResult::ValidationError;
            }
            
            GameStateData gameState;
            LoadResult result = m_saveManager->loadGame(gameState.saveData, slotName);
            
            if (result == LoadResult::Success) {
                if (!restoreGameState(gameState)) {
                    result = LoadResult::ValidationError;
                }
            }
            
            if (m_callbacks.onLoadComplete) {
                m_callbacks.onLoadComplete(result);
            }
            
            return result;
        }
        
        LoadResult SaveIntegration::loadGameState(int slotNumber) {
            return loadGameState("slot_" + std::to_string(slotNumber));
        }
        
        SaveIntegrationResult SaveIntegration::quickSave() {
            return saveGameState("quicksave");
        }
        
        LoadResult SaveIntegration::quickLoad() {
            return loadGameState("quicksave");
        }
        
        SaveIntegrationResult SaveIntegration::autoSave() {
            if (!m_autoSaveConfig.enabled) {
                setError("Auto-save is disabled");
                return SaveIntegrationResult::SystemError;
            }
            
            // Use rotating auto-save slots
            static int autoSaveIndex = 0;
            std::string autoSaveSlot = "autosave_" + std::to_string(autoSaveIndex);
            
            SaveIntegrationResult result = saveGameState(autoSaveSlot);
            
            if (result == SaveIntegrationResult::Success) {
                autoSaveIndex = (autoSaveIndex + 1) % m_autoSaveConfig.maxAutoSaves;
            }
            
            return result;
        }
        
        void SaveIntegration::setAudioManagers(std::shared_ptr<RPGEngine::Audio::MusicManager> musicManager,
                                             std::shared_ptr<RPGEngine::Audio::SoundEffectManager> soundManager) {
            m_musicManager = musicManager;
            m_soundManager = soundManager;
        }
        
        float SaveIntegration::getTimeUntilAutoSave() const {
            if (!m_autoSaveConfig.enabled) {
                return -1.0f;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAutoSave).count();
            float remaining = m_autoSaveConfig.intervalSeconds - elapsed;
            
            return std::max(0.0f, remaining);
        }
        
        void SaveIntegration::registerSystemStateSerializer(const std::string& systemName,
                                                          std::function<std::string()> serializer,
                                                          std::function<bool(const std::string&)> deserializer) {
            m_systemSerializers[systemName] = serializer;
            m_systemDeserializers[systemName] = deserializer;
        }
        
        void SaveIntegration::unregisterSystemStateSerializer(const std::string& systemName) {
            m_systemSerializers.erase(systemName);
            m_systemDeserializers.erase(systemName);
        }
        
        bool SaveIntegration::collectGameState(GameStateData& outGameState) {
            clearError();
            
            // Collect player data
            if (!collectPlayerData(outGameState.saveData.player)) {
                setError("Failed to collect player data");
                return false;
            }
            
            // Collect world data
            if (!collectWorldData(outGameState.saveData.world)) {
                setError("Failed to collect world data");
                return false;
            }
            
            // Collect scene states
            outGameState.sceneStates = m_sceneManager->saveAllSceneStates();
            outGameState.currentSceneId = m_sceneManager->getCurrentScene() ? 
                m_sceneManager->getCurrentScene()->getSceneId() : "";
            
            // Collect system states
            if (!collectSystemStates(outGameState.systemStates)) {
                setError("Failed to collect system states");
                return false;
            }
            
            // Collect audio states
            if (!collectAudioStates(outGameState.audioStates)) {
                setError("Failed to collect audio states");
                return false;
            }
            
            // Set version and timestamp
            outGameState.saveData.version = "1.0";
            
            return true;
        }
        
        bool SaveIntegration::restoreGameState(const GameStateData& gameState) {
            clearError();
            
            // Restore player data
            if (!restorePlayerData(gameState.saveData.player)) {
                setError("Failed to restore player data");
                return false;
            }
            
            // Restore world data
            if (!restoreWorldData(gameState.saveData.world)) {
                setError("Failed to restore world data");
                return false;
            }
            
            // Restore scene states
            if (!m_sceneManager->loadAllSceneStates(gameState.sceneStates)) {
                setError("Failed to restore scene states");
                return false;
            }
            
            // Switch to the saved current scene
            if (!gameState.currentSceneId.empty()) {
                m_sceneManager->switchToScene(gameState.currentSceneId);
            }
            
            // Restore system states
            if (!restoreSystemStates(gameState.systemStates)) {
                setError("Failed to restore system states");
                return false;
            }
            
            // Restore audio states
            if (!restoreAudioStates(gameState.audioStates)) {
                setError("Failed to restore audio states");
                return false;
            }
            
            return true;
        }
        
        bool SaveIntegration::collectPlayerData(PlayerData& outPlayerData) {
            // Find player entity (assuming there's only one player)
            // This is a simplified approach - in a real game, you'd have a more robust way to identify the player
            auto entities = m_entityManager->getAllEntities();
            RPGEngine::Entity playerEntity;
            
            // Look for entity with player-specific components
            for (const auto& entity : entities) {
                if (m_componentManager->hasComponent<StatsComponent>(entity) &&
                    m_componentManager->hasComponent<InventoryComponent>(entity) &&
                    m_componentManager->hasComponent<TransformComponent>(entity)) {
                    
                    // Additional check - could look for a "Player" tag component or similar
                    playerEntity = entity;
                    break;
                }
            }
            
            if (!playerEntity.isValid()) {
                setError("Player entity not found");
                return false;
            }
            
            // Collect transform data
            if (m_componentManager->hasComponent<TransformComponent>(playerEntity)) {
                auto transform = m_componentManager->getComponent<TransformComponent>(playerEntity);
                outPlayerData.position.x = transform->position.x;
                outPlayerData.position.y = transform->position.y;
            }
            
            // Collect stats data
            if (m_componentManager->hasComponent<StatsComponent>(playerEntity)) {
                auto stats = m_componentManager->getComponent<StatsComponent>(playerEntity);
                outPlayerData.stats.hp = static_cast<int>(stats->getCurrentHP());
                outPlayerData.stats.maxHp = static_cast<int>(stats->getMaxHP());
                outPlayerData.stats.mp = static_cast<int>(stats->getCurrentMP());
                outPlayerData.stats.maxMp = static_cast<int>(stats->getMaxMP());
                outPlayerData.stats.level = stats->getLevel();
                outPlayerData.stats.experience = stats->getCurrentExperience();
                outPlayerData.stats.experienceToNext = stats->getExperienceToNextLevel();
            }
            
            // Collect inventory data
            if (m_componentManager->hasComponent<InventoryComponent>(playerEntity)) {
                auto inventory = m_componentManager->getComponent<InventoryComponent>(playerEntity);
                
                outPlayerData.inventory.clear();
                const auto& items = inventory->getAllItems();
                for (const auto& item : items) {
                    PlayerData::InventoryItem saveItem;
                    saveItem.id = item.getId();
                    saveItem.quantity = item.getQuantity();
                    outPlayerData.inventory.push_back(saveItem);
                }
                
                // Collect equipment
                outPlayerData.equipment.clear();
                const auto& equippedItems = inventory->getEquippedItems();
                for (const auto& [slot, item] : equippedItems) {
                    outPlayerData.equipment[slot] = item.getId();
                }
            }
            
            return true;
        }
        
        bool SaveIntegration::restorePlayerData(const PlayerData& playerData) {
            // Find or create player entity
            auto entities = m_entityManager->getAllEntities();
            RPGEngine::Entity playerEntity;
            
            // Look for existing player entity
            for (const auto& entity : entities) {
                if (m_componentManager->hasComponent<StatsComponent>(entity) &&
                    m_componentManager->hasComponent<InventoryComponent>(entity) &&
                    m_componentManager->hasComponent<TransformComponent>(entity)) {
                    playerEntity = entity;
                    break;
                }
            }
            
            // Create player entity if not found
            if (!playerEntity.isValid()) {
                playerEntity = m_entityManager->createEntity();
                m_componentManager->addComponent<TransformComponent>(playerEntity);
                m_componentManager->addComponent<StatsComponent>(playerEntity);
                m_componentManager->addComponent<InventoryComponent>(playerEntity);
            }
            
            // Restore transform data
            if (m_componentManager->hasComponent<TransformComponent>(playerEntity)) {
                auto transform = m_componentManager->getComponent<TransformComponent>(playerEntity);
                transform->position.x = playerData.position.x;
                transform->position.y = playerData.position.y;
            }
            
            // Restore stats data
            if (m_componentManager->hasComponent<StatsComponent>(playerEntity)) {
                auto stats = m_componentManager->getComponent<StatsComponent>(playerEntity);
                stats->setCurrentHP(static_cast<float>(playerData.stats.hp));
                stats->setBaseMaxHP(static_cast<float>(playerData.stats.maxHp));
                stats->setCurrentMP(static_cast<float>(playerData.stats.mp));
                stats->setBaseMaxMP(static_cast<float>(playerData.stats.maxMp));
                stats->setLevel(playerData.stats.level);
                stats->setCurrentExperience(playerData.stats.experience);
            }
            
            // Restore inventory data
            if (m_componentManager->hasComponent<InventoryComponent>(playerEntity)) {
                auto inventory = m_componentManager->getComponent<InventoryComponent>(playerEntity);
                
                // Clear existing inventory
                inventory->clear();
                
                // Restore items
                for (const auto& saveItem : playerData.inventory) {
                    inventory->addItem(saveItem.id, saveItem.quantity);
                }
                
                // Restore equipment
                for (const auto& [slot, itemId] : playerData.equipment) {
                    inventory->equipItem(itemId, slot);
                }
            }
            
            return true;
        }
        
        bool SaveIntegration::collectWorldData(WorldData& outWorldData) {
            // Get current map from world manager
            outWorldData.currentMap = m_worldManager->getCurrentMapId();
            
            // Collect quest data
            outWorldData.completedQuests.clear();
            auto entities = m_entityManager->getAllEntities();
            
            for (const auto& entity : entities) {
                if (m_componentManager->hasComponent<QuestComponent>(entity)) {
                    auto questComp = m_componentManager->getComponent<QuestComponent>(entity);
                    const auto& completedQuests = questComp->getCompletedQuests();
                    
                    for (const auto& questId : completedQuests) {
                        outWorldData.completedQuests.push_back(questId);
                    }
                }
            }
            
            // Collect game flags (this would typically come from a game state manager)
            // For now, we'll use a simple approach
            outWorldData.gameFlags.clear();
            
            // Collect NPC states
            outWorldData.npcStates.clear();
            for (const auto& entity : entities) {
                if (m_componentManager->hasComponent<RPGEngine::DialogueComponent>(entity)) {
                    // Store basic NPC state - simplified for now
                    std::string entityId = std::to_string(entity.getId());
                    outWorldData.npcStates[entityId] = "default"; // Placeholder
                }
            }
            
            // Collect discovered locations (this would typically come from a discovery system)
            outWorldData.discoveredLocations.clear();
            outWorldData.discoveredLocations[outWorldData.currentMap] = true;
            
            return true;
        }
        
        bool SaveIntegration::restoreWorldData(const WorldData& worldData) {
            // Load the current map
            if (!worldData.currentMap.empty()) {
                if (!m_worldManager->loadMap(worldData.currentMap)) {
                    setError("Failed to load map: " + worldData.currentMap);
                    return false;
                }
            }
            
            // Restore quest states
            auto entities = m_entityManager->getAllEntities();
            for (const auto& entity : entities) {
                if (m_componentManager->hasComponent<RPGEngine::QuestComponent>(entity)) {
                    auto questComp = m_componentManager->getComponent<RPGEngine::QuestComponent>(entity);
                    
                    // Mark quests as completed
                    for (const auto& questId : worldData.completedQuests) {
                        questComp->completeQuest(questId);
                    }
                }
            }
            
            // Restore NPC states
            for (const auto& [entityIdStr, state] : worldData.npcStates) {
                try {
                    uint32_t entityId = std::stoul(entityIdStr);
                    RPGEngine::Entity entity(entityId);
                    
                    if (m_componentManager->hasComponent<RPGEngine::DialogueComponent>(entity)) {
                        // Restore NPC state - simplified for now
                        // In a full implementation, this would restore dialogue state
                    }
                } catch (const std::exception& e) {
                    // Log error but continue
                }
            }
            
            return true;
        }
        
        bool SaveIntegration::collectSystemStates(std::unordered_map<std::string, std::string>& outSystemStates) {
            outSystemStates.clear();
            
            // Collect states from registered system serializers
            for (const auto& [systemName, serializer] : m_systemSerializers) {
                try {
                    std::string systemState = serializer();
                    if (!systemState.empty()) {
                        outSystemStates[systemName] = systemState;
                    }
                } catch (const std::exception& e) {
                    setError("Failed to serialize system state for: " + systemName);
                    return false;
                }
            }
            
            return true;
        }
        
        bool SaveIntegration::restoreSystemStates(const std::unordered_map<std::string, std::string>& systemStates) {
            // Restore states using registered system deserializers
            for (const auto& [systemName, systemState] : systemStates) {
                auto it = m_systemDeserializers.find(systemName);
                if (it != m_systemDeserializers.end()) {
                    try {
                        if (!it->second(systemState)) {
                            setError("Failed to deserialize system state for: " + systemName);
                            return false;
                        }
                    } catch (const std::exception& e) {
                        setError("Exception during system state deserialization for: " + systemName);
                        return false;
                    }
                }
            }
            
            return true;
        }
        
        bool SaveIntegration::collectAudioStates(std::unordered_map<std::string, std::string>& outAudioStates) {
            outAudioStates.clear();
            
            // Collect music state
            if (m_musicManager) {
                nlohmann::json musicState;
                musicState["currentTrack"] = m_musicManager->getCurrentTrack();
                musicState["volume"] = m_musicManager->getVolume();
                musicState["isPlaying"] = m_musicManager->isPlaying();
                musicState["isPaused"] = m_musicManager->isPaused();
                outAudioStates["music"] = musicState.dump();
            }
            
            // Collect sound effect state
            if (m_soundManager) {
                nlohmann::json soundState;
                soundState["volume"] = m_soundManager->getVolume();
                soundState["enabled"] = m_soundManager->isEnabled();
                outAudioStates["sound"] = soundState.dump();
            }
            
            return true;
        }
        
        bool SaveIntegration::restoreAudioStates(const std::unordered_map<std::string, std::string>& audioStates) {
            // Restore music state
            auto musicIt = audioStates.find("music");
            if (musicIt != audioStates.end() && m_musicManager) {
                try {
                    nlohmann::json musicState = nlohmann::json::parse(musicIt->second);
                    
                    std::string currentTrack = musicState.value("currentTrack", "");
                    float volume = musicState.value("volume", 1.0f);
                    bool isPlaying = musicState.value("isPlaying", false);
                    bool isPaused = musicState.value("isPaused", false);
                    
                    m_musicManager->setVolume(volume);
                    
                    if (!currentTrack.empty() && isPlaying) {
                        m_musicManager->playMusic(currentTrack);
                        if (isPaused) {
                            m_musicManager->pauseMusic();
                        }
                    }
                } catch (const std::exception& e) {
                    // Log error but continue
                }
            }
            
            // Restore sound effect state
            auto soundIt = audioStates.find("sound");
            if (soundIt != audioStates.end() && m_soundManager) {
                try {
                    nlohmann::json soundState = nlohmann::json::parse(soundIt->second);
                    
                    float volume = soundState.value("volume", 1.0f);
                    bool enabled = soundState.value("enabled", true);
                    
                    m_soundManager->setVolume(volume);
                    m_soundManager->setEnabled(enabled);
                } catch (const std::exception& e) {
                    // Log error but continue
                }
            }
            
            return true;
        }
        
        bool SaveIntegration::shouldAutoSave() const {
            if (!m_autoSaveConfig.enabled) {
                return false;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAutoSave).count();
            
            return elapsed >= m_autoSaveConfig.intervalSeconds;
        }
        
        void SaveIntegration::resetAutoSaveTimer() {
            m_lastAutoSave = std::chrono::steady_clock::now();
        }
        
    } // namespace Save
} // namespace Engine