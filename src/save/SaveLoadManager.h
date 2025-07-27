#pragma once

#include "SaveIntegration.h"
#include "../ui/SaveLoadUI.h"
#include "../core/Event.h"
#include <memory>
#include <functional>
#include <chrono>

namespace Engine {
    namespace Save {
        
        /**
         * Save/Load event types
         */
        enum class SaveLoadEventType {
            SaveStarted,
            SaveCompleted,
            SaveFailed,
            LoadStarted,
            LoadCompleted,
            LoadFailed,
            AutoSaveTriggered,
            QuickSaveTriggered,
            QuickLoadTriggered
        };
        
        /**
         * Save/Load event data
         */
        struct SaveLoadEventData {
            SaveLoadEventType type;
            std::string slotName;
            int slotNumber;
            std::string errorMessage;
            float duration;
        };
        
        /**
         * Auto-save trigger conditions
         */
        struct AutoSaveTriggers {
            bool onMapTransition = true;
            bool onLevelUp = true;
            bool onQuestComplete = true;
            bool onCombatEnd = true;
            bool onItemAcquired = false;
            bool onDialogueEnd = false;
            bool onTimeInterval = true;
            float timeIntervalSeconds = 300.0f; // 5 minutes
        };
        
        /**
         * Complete save/load management system
         * Integrates SaveIntegration, SaveLoadUI, and provides high-level save/load operations
         */
        class SaveLoadManager {
        public:
            /**
             * Constructor
             */
            SaveLoadManager();
            
            /**
             * Destructor
             */
            ~SaveLoadManager();
            
            /**
             * Initialize save/load manager
             * @param saveIntegration Save integration system
             * @param eventSystem Event system for notifications
             * @return true if initialization was successful
             */
            bool initialize(std::shared_ptr<SaveIntegration> saveIntegration,
                          std::shared_ptr<RPGEngine::EventSystem> eventSystem = nullptr);
            
            /**
             * Shutdown save/load manager
             */
            void shutdown();
            
            /**
             * Update save/load manager
             * @param deltaTime Time since last update
             */
            void update(float deltaTime);
            
            /**
             * Get save/load UI
             * @return Save/load UI instance
             */
            std::shared_ptr<RPGEngine::UI::SaveLoadUI> getUI() const { return m_ui; }
            
            /**
             * Get save integration
             * @return Save integration instance
             */
            std::shared_ptr<SaveIntegration> getSaveIntegration() const { return m_saveIntegration; }
            
            // High-level save operations
            
            /**
             * Save game to specific slot
             * @param slotNumber Slot number (0-based)
             * @param callback Optional completion callback
             */
            void saveGame(int slotNumber, std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Save game to named slot
             * @param slotName Slot name
             * @param callback Optional completion callback
             */
            void saveGame(const std::string& slotName, std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Load game from specific slot
             * @param slotNumber Slot number (0-based)
             * @param callback Optional completion callback
             */
            void loadGame(int slotNumber, std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Load game from named slot
             * @param slotName Slot name
             * @param callback Optional completion callback
             */
            void loadGame(const std::string& slotName, std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Quick save (uses quicksave slot)
             * @param callback Optional completion callback
             */
            void quickSave(std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Quick load (loads from quicksave slot)
             * @param callback Optional completion callback
             */
            void quickLoad(std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Auto save
             * @param callback Optional completion callback
             */
            void autoSave(std::function<void(bool, const std::string&)> callback = nullptr);
            
            /**
             * Delete save slot
             * @param slotNumber Slot number (0-based)
             * @return true if deletion was successful
             */
            bool deleteSave(int slotNumber);
            
            /**
             * Delete named save slot
             * @param slotName Slot name
             * @return true if deletion was successful
             */
            bool deleteSave(const std::string& slotName);
            
            // UI operations
            
            /**
             * Show save menu
             */
            void showSaveMenu();
            
            /**
             * Show load menu
             */
            void showLoadMenu();
            
            /**
             * Hide save/load UI
             */
            void hideUI();
            
            /**
             * Check if save/load UI is visible
             * @return true if UI is visible
             */
            bool isUIVisible() const;
            
            // Configuration
            
            /**
             * Set auto-save triggers
             * @param triggers Auto-save trigger configuration
             */
            void setAutoSaveTriggers(const AutoSaveTriggers& triggers) { m_autoSaveTriggers = triggers; }
            
            /**
             * Get auto-save triggers
             * @return Auto-save trigger configuration
             */
            const AutoSaveTriggers& getAutoSaveTriggers() const { return m_autoSaveTriggers; }
            
            /**
             * Enable/disable auto-save
             * @param enabled Whether auto-save is enabled
             */
            void setAutoSaveEnabled(bool enabled);
            
            /**
             * Check if auto-save is enabled
             * @return true if auto-save is enabled
             */
            bool isAutoSaveEnabled() const;
            
            /**
             * Set maximum number of save slots
             * @param maxSlots Maximum number of save slots
             */
            void setMaxSaveSlots(int maxSlots);
            
            /**
             * Get maximum number of save slots
             * @return Maximum number of save slots
             */
            int getMaxSaveSlots() const;
            
            // Event handling
            
            /**
             * Register for save/load events
             * @param callback Event callback function
             */
            void registerEventCallback(std::function<void(const SaveLoadEventData&)> callback);
            
            /**
             * Trigger auto-save based on game event
             * @param eventType Game event type
             */
            void onGameEvent(const std::string& eventType);
            
            // Status and information
            
            /**
             * Check if currently saving
             * @return true if save operation is in progress
             */
            bool isSaving() const { return m_isSaving; }
            
            /**
             * Check if currently loading
             * @return true if load operation is in progress
             */
            bool isLoading() const { return m_isLoading; }
            
            /**
             * Get time until next auto-save
             * @return Time in seconds until next auto-save
             */
            float getTimeUntilAutoSave() const;
            
            /**
             * Get last error message
             * @return Last error message
             */
            const std::string& getLastError() const { return m_lastError; }
            
            /**
             * Get save slot information
             * @return Vector of save slot information
             */
            std::vector<RPGEngine::UI::SaveSlotInfo> getSaveSlots() const;
            
        private:
            /**
             * Handle save completion
             * @param result Save result
             * @param slotName Slot name
             * @param callback Completion callback
             */
            void handleSaveCompletion(SaveIntegrationResult result, const std::string& slotName,
                                    std::function<void(bool, const std::string&)> callback);
            
            /**
             * Handle load completion
             * @param result Load result
             * @param slotName Slot name
             * @param callback Completion callback
             */
            void handleLoadCompletion(LoadResult result, const std::string& slotName,
                                    std::function<void(bool, const std::string&)> callback);
            
            /**
             * Send save/load event
             * @param eventData Event data
             */
            void sendEvent(const SaveLoadEventData& eventData);
            
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
            
            // Core systems
            std::shared_ptr<SaveIntegration> m_saveIntegration;
            std::shared_ptr<RPGEngine::UI::SaveLoadUI> m_ui;
            std::shared_ptr<RPGEngine::EventSystem> m_eventSystem;
            
            // Configuration
            AutoSaveTriggers m_autoSaveTriggers;
            
            // State
            bool m_initialized;
            bool m_isSaving;
            bool m_isLoading;
            std::string m_lastError;
            
            // Auto-save timing
            std::chrono::steady_clock::time_point m_lastAutoSave;
            
            // Event callbacks
            std::vector<std::function<void(const SaveLoadEventData&)>> m_eventCallbacks;
        };
        
    } // namespace Save
} // namespace Engine