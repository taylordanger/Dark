#pragma once

#include "SaveManager.h"
#include "../entities/EntityManager.h"
#include "../components/ComponentManager.h"
#include "../systems/SystemManager.h"
#include "../scene/SceneManager.h"
#include "../world/WorldManager.h"
#include "../audio/MusicManager.h"
#include "../audio/SoundEffectManager.h"
#include <memory>
#include <functional>
#include <chrono>

namespace Engine {
    namespace Save {
        
        /**
         * Save integration result
         */
        enum class SaveIntegrationResult {
            Success,
            SystemError,
            DataError,
            ValidationError,
            FileError
        };
        
        /**
         * Auto-save configuration
         */
        struct AutoSaveConfig {
            bool enabled = true;
            float intervalSeconds = 300.0f; // 5 minutes
            int maxAutoSaves = 5;
            bool saveOnMapTransition = true;
            bool saveOnLevelUp = true;
            bool saveOnQuestComplete = true;
            bool saveOnCombatEnd = true;
        };
        
        /**
         * Save integration callbacks
         */
        struct SaveIntegrationCallbacks {
            std::function<void(SaveIntegrationResult)> onSaveComplete;
            std::function<void(LoadResult)> onLoadComplete;
            std::function<void(const std::string&)> onAutoSave;
            std::function<void(const std::string&)> onError;
        };
        
        /**
         * Complete game state data
         */
        struct GameStateData {
            SaveData saveData;
            std::unordered_map<std::string, RPGEngine::Scene::SceneData> sceneStates;
            std::string currentSceneId;
            std::unordered_map<std::string, std::string> systemStates;
            std::vector<uint8_t> worldData;
            std::unordered_map<std::string, std::string> audioStates;
        };
        
        /**
         * Save integration manager
         * Coordinates saving/loading across all game systems
         */
        class SaveIntegration {
        public:
            /**
             * Constructor
             */
            SaveIntegration();
            
            /**
             * Destructor
             */
            ~SaveIntegration();
            
            /**
             * Initialize save integration
             * @param saveManager Save manager
             * @param entityManager Entity manager
             * @param componentManager Component manager
             * @param systemManager System manager
             * @param sceneManager Scene manager
             * @param worldManager World manager
             * @return true if initialization was successful
             */
            bool initialize(std::shared_ptr<SaveManager> saveManager,
                          std::shared_ptr<RPGEngine::EntityManager> entityManager,
                          std::shared_ptr<RPGEngine::ComponentManager> componentManager,
                          std::shared_ptr<RPGEngine::SystemManager> systemManager,
                          std::shared_ptr<RPGEngine::Scene::SceneManager> sceneManager,
                          std::shared_ptr<RPGEngine::World::WorldManager> worldManager);
            
            /**
             * Shutdown save integration
             */
            void shutdown();
            
            /**
             * Update save integration (handles auto-save timing)
             * @param deltaTime Time since last update
             */
            void update(float deltaTime);
            
            /**
             * Save complete game state
             * @param slotName Save slot name
             * @return Save integration result
             */
            SaveIntegrationResult saveGameState(const std::string& slotName);
            
            /**
             * Save complete game state to numbered slot
             * @param slotNumber Save slot number
             * @return Save integration result
             */
            SaveIntegrationResult saveGameState(int slotNumber);
            
            /**
             * Load complete game state
             * @param slotName Save slot name
             * @return Load result
             */
            LoadResult loadGameState(const std::string& slotName);
            
            /**
             * Load complete game state from numbered slot
             * @param slotNumber Save slot number
             * @return Load result
             */
            LoadResult loadGameState(int slotNumber);
            
            /**
             * Quick save (uses quicksave slot)
             * @return Save integration result
             */
            SaveIntegrationResult quickSave();
            
            /**
             * Quick load (loads from quicksave slot)
             * @return Load result
             */
            LoadResult quickLoad();
            
            /**
             * Auto save
             * @return Save integration result
             */
            SaveIntegrationResult autoSave();
            
            /**
             * Set audio managers for save integration
             * @param musicManager Music manager
             * @param soundManager Sound effect manager
             */
            void setAudioManagers(std::shared_ptr<RPGEngine::Audio::MusicManager> musicManager,
                                std::shared_ptr<RPGEngine::Audio::SoundEffectManager> soundManager);
            
            /**
             * Set auto-save configuration
             * @param config Auto-save configuration
             */
            void setAutoSaveConfig(const AutoSaveConfig& config) { m_autoSaveConfig = config; }
            
            /**
             * Get auto-save configuration
             * @return Auto-save configuration
             */
            const AutoSaveConfig& getAutoSaveConfig() const { return m_autoSaveConfig; }
            
            /**
             * Set save integration callbacks
             * @param callbacks Callback functions
             */
            void setCallbacks(const SaveIntegrationCallbacks& callbacks) { m_callbacks = callbacks; }
            
            /**
             * Enable/disable auto-save
             * @param enabled Whether auto-save is enabled
             */
            void setAutoSaveEnabled(bool enabled) { m_autoSaveConfig.enabled = enabled; }
            
            /**
             * Check if auto-save is enabled
             * @return true if auto-save is enabled
             */
            bool isAutoSaveEnabled() const { return m_autoSaveConfig.enabled; }
            
            /**
             * Force auto-save on next update
             */
            void forceAutoSave() { m_forceAutoSave = true; }
            
            /**
             * Get time until next auto-save
             * @return Time in seconds until next auto-save
             */
            float getTimeUntilAutoSave() const;
            
            /**
             * Get save manager
             * @return Save manager
             */
            std::shared_ptr<SaveManager> getSaveManager() const { return m_saveManager; }
            
            /**
             * Register system state serializer
             * @param systemName System name
             * @param serializer Serialization function
             * @param deserializer Deserialization function
             */
            void registerSystemStateSerializer(const std::string& systemName,
                                             std::function<std::string()> serializer,
                                             std::function<bool(const std::string&)> deserializer);
            
            /**
             * Unregister system state serializer
             * @param systemName System name
             */
            void unregisterSystemStateSerializer(const std::string& systemName);
            
            /**
             * Get last error message
             * @return Last error message
             */
            const std::string& getLastError() const { return m_lastError; }
            
        private:
            /**
             * Collect complete game state
             * @param outGameState Output game state data
             * @return true if collection was successful
             */
            bool collectGameState(GameStateData& outGameState);
            
            /**
             * Restore complete game state
             * @param gameState Game state data to restore
             * @return true if restoration was successful
             */
            bool restoreGameState(const GameStateData& gameState);
            
            /**
             * Collect player data
             * @param outPlayerData Output player data
             * @return true if collection was successful
             */
            bool collectPlayerData(PlayerData& outPlayerData);
            
            /**
             * Restore player data
             * @param playerData Player data to restore
             * @return true if restoration was successful
             */
            bool restorePlayerData(const PlayerData& playerData);
            
            /**
             * Collect world data
             * @param outWorldData Output world data
             * @return true if collection was successful
             */
            bool collectWorldData(WorldData& outWorldData);
            
            /**
             * Restore world data
             * @param worldData World data to restore
             * @return true if restoration was successful
             */
            bool restoreWorldData(const WorldData& worldData);
            
            /**
             * Collect system states
             * @param outSystemStates Output system states
             * @return true if collection was successful
             */
            bool collectSystemStates(std::unordered_map<std::string, std::string>& outSystemStates);
            
            /**
             * Restore system states
             * @param systemStates System states to restore
             * @return true if restoration was successful
             */
            bool restoreSystemStates(const std::unordered_map<std::string, std::string>& systemStates);
            
            /**
             * Collect audio states
             * @param outAudioStates Output audio states
             * @return true if collection was successful
             */
            bool collectAudioStates(std::unordered_map<std::string, std::string>& outAudioStates);
            
            /**
             * Restore audio states
             * @param audioStates Audio states to restore
             * @return true if restoration was successful
             */
            bool restoreAudioStates(const std::unordered_map<std::string, std::string>& audioStates);
            
            /**
             * Check if auto-save should trigger
             * @return true if auto-save should trigger
             */
            bool shouldAutoSave() const;
            
            /**
             * Reset auto-save timer
             */
            void resetAutoSaveTimer();
            
            /**
             * Set error message
             * @param error Error message
             */
            void setError(const std::string& error) { m_lastError = error; }
            
            /**
             * Clear error message
             */
            void clearError() { m_lastError.clear(); }
            
            // Managers
            std::shared_ptr<SaveManager> m_saveManager;
            std::shared_ptr<RPGEngine::EntityManager> m_entityManager;
            std::shared_ptr<RPGEngine::ComponentManager> m_componentManager;
            std::shared_ptr<RPGEngine::SystemManager> m_systemManager;
            std::shared_ptr<RPGEngine::Scene::SceneManager> m_sceneManager;
            std::shared_ptr<RPGEngine::World::WorldManager> m_worldManager;
            std::shared_ptr<RPGEngine::Audio::MusicManager> m_musicManager;
            std::shared_ptr<RPGEngine::Audio::SoundEffectManager> m_soundManager;
            
            // Auto-save state
            AutoSaveConfig m_autoSaveConfig;
            std::chrono::steady_clock::time_point m_lastAutoSave;
            bool m_forceAutoSave;
            
            // System state serializers
            std::unordered_map<std::string, std::function<std::string()>> m_systemSerializers;
            std::unordered_map<std::string, std::function<bool(const std::string&)>> m_systemDeserializers;
            
            // Callbacks
            SaveIntegrationCallbacks m_callbacks;
            
            // State
            bool m_initialized;
            std::string m_lastError;
        };
        
    } // namespace Save
} // namespace Engine