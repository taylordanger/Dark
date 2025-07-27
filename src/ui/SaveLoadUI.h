#pragma once

#include "../save/SaveManager.h"
#include "../save/SaveIntegration.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace RPGEngine {
    namespace UI {
        
        /**
         * Save slot information for UI display
         */
        struct SaveSlotInfo {
            int slotNumber;
            std::string slotName;
            std::string displayName;
            std::string timestamp;
            std::string playerName;
            int playerLevel;
            std::string currentMap;
            std::string playtime;
            size_t fileSize;
            bool isEmpty;
            bool isAutoSave;
            bool isQuickSave;
            
            SaveSlotInfo() : slotNumber(-1), playerLevel(1), fileSize(0), 
                           isEmpty(true), isAutoSave(false), isQuickSave(false) {}
        };
        
        /**
         * Save/Load UI state
         */
        enum class SaveLoadUIState {
            Hidden,
            SaveMenu,
            LoadMenu,
            ConfirmOverwrite,
            ConfirmDelete,
            Loading,
            Saving,
            Error
        };
        
        /**
         * Save/Load UI callbacks
         */
        struct SaveLoadUICallbacks {
            std::function<void(int)> onSaveSlotSelected;
            std::function<void(int)> onLoadSlotSelected;
            std::function<void()> onQuickSave;
            std::function<void()> onQuickLoad;
            std::function<void(int)> onDeleteSlot;
            std::function<void()> onCancel;
            std::function<void(const std::string&)> onError;
        };
        
        /**
         * Save/Load UI configuration
         */
        struct SaveLoadUIConfig {
            int maxSaveSlots = 10;
            bool showAutoSaves = true;
            bool showQuickSave = true;
            bool allowSlotDeletion = true;
            bool confirmOverwrite = true;
            bool showFileSize = true;
            bool showPlaytime = false;
            std::string dateTimeFormat = "%Y-%m-%d %H:%M";
        };
        
        /**
         * Save/Load UI manager
         * Provides UI for saving and loading game states
         */
        class SaveLoadUI {
        public:
            /**
             * Constructor
             */
            SaveLoadUI();
            
            /**
             * Destructor
             */
            ~SaveLoadUI();
            
            /**
             * Initialize save/load UI
             * @param saveIntegration Save integration system
             * @return true if initialization was successful
             */
            bool initialize(std::shared_ptr<Engine::Save::SaveIntegration> saveIntegration);
            
            /**
             * Shutdown save/load UI
             */
            void shutdown();
            
            /**
             * Update save/load UI
             * @param deltaTime Time since last update
             */
            void update(float deltaTime);
            
            /**
             * Render save/load UI
             */
            void render();
            
            /**
             * Handle input event
             * @param event Input event
             * @return true if input was handled
             */
            bool handleInput(const std::string& event);
            
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
            void hide();
            
            /**
             * Check if UI is visible
             * @return true if UI is visible
             */
            bool isVisible() const { return m_state != SaveLoadUIState::Hidden; }
            
            /**
             * Get current UI state
             * @return Current UI state
             */
            SaveLoadUIState getState() const { return m_state; }
            
            /**
             * Set UI configuration
             * @param config UI configuration
             */
            void setConfig(const SaveLoadUIConfig& config) { m_config = config; }
            
            /**
             * Get UI configuration
             * @return UI configuration
             */
            const SaveLoadUIConfig& getConfig() const { return m_config; }
            
            /**
             * Set UI callbacks
             * @param callbacks UI callbacks
             */
            void setCallbacks(const SaveLoadUICallbacks& callbacks) { m_callbacks = callbacks; }
            
            /**
             * Refresh save slot information
             */
            void refreshSaveSlots();
            
            /**
             * Get save slot information
             * @return Vector of save slot information
             */
            const std::vector<SaveSlotInfo>& getSaveSlots() const { return m_saveSlots; }
            
            /**
             * Set error message
             * @param error Error message
             */
            void setError(const std::string& error);
            
            /**
             * Clear error message
             */
            void clearError();
            
            /**
             * Get current error message
             * @return Current error message
             */
            const std::string& getError() const { return m_errorMessage; }
            
            /**
             * Set loading state
             * @param loading Whether UI is in loading state
             */
            void setLoading(bool loading);
            
            /**
             * Check if UI is in loading state
             * @return true if UI is loading
             */
            bool isLoading() const { return m_state == SaveLoadUIState::Loading || m_state == SaveLoadUIState::Saving; }
            
        private:
            /**
             * Render save menu
             */
            void renderSaveMenu();
            
            /**
             * Render load menu
             */
            void renderLoadMenu();
            
            /**
             * Render save slot
             * @param slot Save slot information
             * @param index Slot index
             * @param isSaveMode Whether in save mode
             * @return true if slot was clicked
             */
            bool renderSaveSlot(const SaveSlotInfo& slot, int index, bool isSaveMode);
            
            /**
             * Render confirmation dialog
             */
            void renderConfirmationDialog();
            
            /**
             * Render error dialog
             */
            void renderErrorDialog();
            
            /**
             * Render loading overlay
             */
            void renderLoadingOverlay();
            
            /**
             * Handle save slot selection
             * @param slotIndex Slot index
             */
            void handleSaveSlotSelection(int slotIndex);
            
            /**
             * Handle load slot selection
             * @param slotIndex Slot index
             */
            void handleLoadSlotSelection(int slotIndex);
            
            /**
             * Handle slot deletion
             * @param slotIndex Slot index
             */
            void handleSlotDeletion(int slotIndex);
            
            /**
             * Format timestamp for display
             * @param timestamp Timestamp string
             * @return Formatted timestamp
             */
            std::string formatTimestamp(const std::string& timestamp) const;
            
            /**
             * Format file size for display
             * @param fileSize File size in bytes
             * @return Formatted file size string
             */
            std::string formatFileSize(size_t fileSize) const;
            
            /**
             * Create save slot info from save manager info
             * @param saveInfo Save manager info
             * @param slotNumber Slot number
             * @return Save slot info
             */
            SaveSlotInfo createSaveSlotInfo(const Engine::Save::SaveManager::SaveInfo& saveInfo, int slotNumber) const;
            
            /**
             * Create empty save slot info
             * @param slotNumber Slot number
             * @return Empty save slot info
             */
            SaveSlotInfo createEmptySaveSlotInfo(int slotNumber) const;
            
            // Save integration
            std::shared_ptr<Engine::Save::SaveIntegration> m_saveIntegration;
            
            // UI state
            SaveLoadUIState m_state;
            SaveLoadUIConfig m_config;
            SaveLoadUICallbacks m_callbacks;
            
            // Save slot data
            std::vector<SaveSlotInfo> m_saveSlots;
            int m_selectedSlot;
            int m_confirmSlot;
            
            // UI state
            std::string m_errorMessage;
            float m_loadingTimer;
            bool m_initialized;
        };
        
    } // namespace UI
} // namespace RPGEngine