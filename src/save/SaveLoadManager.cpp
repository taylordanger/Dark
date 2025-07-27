#include "SaveLoadManager.h"
#include <iostream>
#include <algorithm>

namespace Engine {
    namespace Save {
        
        SaveLoadManager::SaveLoadManager()
            : m_initialized(false)
            , m_isSaving(false)
            , m_isLoading(false)
        {
            m_lastAutoSave = std::chrono::steady_clock::now();
        }
        
        SaveLoadManager::~SaveLoadManager() {
            shutdown();
        }
        
        bool SaveLoadManager::initialize(std::shared_ptr<SaveIntegration> saveIntegration,
                                       std::shared_ptr<RPGEngine::EventSystem> eventSystem) {
            if (m_initialized) {
                return true;
            }
            
            if (!saveIntegration) {
                setError("SaveIntegration is required");
                return false;
            }
            
            m_saveIntegration = saveIntegration;
            m_eventSystem = eventSystem;
            
            // Initialize UI
            m_ui = std::make_shared<RPGEngine::UI::SaveLoadUI>();
            if (!m_ui->initialize(saveIntegration)) {
                setError("Failed to initialize SaveLoadUI");
                return false;
            }
            
            // Set up UI callbacks
            RPGEngine::UI::SaveLoadUICallbacks uiCallbacks;
            uiCallbacks.onSaveSlotSelected = [this](int slotNumber) {
                saveGame(slotNumber);
            };
            uiCallbacks.onLoadSlotSelected = [this](int slotNumber) {
                loadGame(slotNumber);
            };
            uiCallbacks.onQuickSave = [this]() {
                quickSave();
            };
            uiCallbacks.onQuickLoad = [this]() {
                quickLoad();
            };
            uiCallbacks.onDeleteSlot = [this](int slotNumber) {
                deleteSave(slotNumber);
            };
            uiCallbacks.onCancel = [this]() {
                hideUI();
            };
            uiCallbacks.onError = [this](const std::string& error) {
                setError(error);
            };
            
            m_ui->setCallbacks(uiCallbacks);
            
            // Set up save integration callbacks
            SaveIntegrationCallbacks saveCallbacks;
            saveCallbacks.onSaveComplete = [this](SaveIntegrationResult result) {
                m_isSaving = false;
                // Additional handling can be added here
            };
            saveCallbacks.onLoadComplete = [this](LoadResult result) {
                m_isLoading = false;
                // Additional handling can be added here
            };
            saveCallbacks.onAutoSave = [this](const std::string& message) {
                SaveLoadEventData eventData;
                eventData.type = SaveLoadEventType::AutoSaveTriggered;
                eventData.slotName = "autosave";
                sendEvent(eventData);
            };
            saveCallbacks.onError = [this](const std::string& error) {
                setError(error);
            };
            
            m_saveIntegration->setCallbacks(saveCallbacks);
            
            m_initialized = true;
            clearError();
            return true;
        }
        
        void SaveLoadManager::shutdown() {
            if (m_ui) {
                m_ui->shutdown();
                m_ui.reset();
            }
            
            m_saveIntegration.reset();
            m_eventSystem.reset();
            m_eventCallbacks.clear();
            
            m_initialized = false;
        }
        
        void SaveLoadManager::update(float deltaTime) {
            if (!m_initialized) {
                return;
            }
            
            // Update UI
            if (m_ui) {
                m_ui->update(deltaTime);
            }
            
            // Update save integration (handles auto-save timing)
            if (m_saveIntegration) {
                m_saveIntegration->update(deltaTime);
            }
            
            // Check for auto-save triggers
            if (shouldAutoSave()) {
                autoSave();
                resetAutoSaveTimer();
            }
        }
        
        void SaveLoadManager::saveGame(int slotNumber, std::function<void(bool, const std::string&)> callback) {
            if (!m_initialized || m_isSaving) {
                if (callback) callback(false, "Save operation already in progress");
                return;
            }
            
            m_isSaving = true;
            clearError();
            
            // Send save started event
            SaveLoadEventData eventData;
            eventData.type = SaveLoadEventType::SaveStarted;
            eventData.slotNumber = slotNumber;
            eventData.slotName = "slot_" + std::to_string(slotNumber);
            sendEvent(eventData);
            
            auto startTime = std::chrono::steady_clock::now();
            
            SaveIntegrationResult result = m_saveIntegration->saveGameState(slotNumber);
            
            auto endTime = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(endTime - startTime).count();
            
            handleSaveCompletion(result, "slot_" + std::to_string(slotNumber), callback);
            
            // Send completion event
            eventData.type = (result == SaveIntegrationResult::Success) ? 
                SaveLoadEventType::SaveCompleted : SaveLoadEventType::SaveFailed;
            eventData.duration = duration;
            if (result != SaveIntegrationResult::Success) {
                eventData.errorMessage = getLastError();
            }
            sendEvent(eventData);
        }
        
        void SaveLoadManager::saveGame(const std::string& slotName, std::function<void(bool, const std::string&)> callback) {
            if (!m_initialized || m_isSaving) {
                if (callback) callback(false, "Save operation already in progress");
                return;
            }
            
            m_isSaving = true;
            clearError();
            
            // Send save started event
            SaveLoadEventData eventData;
            eventData.type = SaveLoadEventType::SaveStarted;
            eventData.slotName = slotName;
            sendEvent(eventData);
            
            auto startTime = std::chrono::steady_clock::now();
            
            SaveIntegrationResult result = m_saveIntegration->saveGameState(slotName);
            
            auto endTime = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(endTime - startTime).count();
            
            handleSaveCompletion(result, slotName, callback);
            
            // Send completion event
            eventData.type = (result == SaveIntegrationResult::Success) ? 
                SaveLoadEventType::SaveCompleted : SaveLoadEventType::SaveFailed;
            eventData.duration = duration;
            if (result != SaveIntegrationResult::Success) {
                eventData.errorMessage = getLastError();
            }
            sendEvent(eventData);
        }
        
        void SaveLoadManager::loadGame(int slotNumber, std::function<void(bool, const std::string&)> callback) {
            if (!m_initialized || m_isLoading) {
                if (callback) callback(false, "Load operation already in progress");
                return;
            }
            
            m_isLoading = true;
            clearError();
            
            // Send load started event
            SaveLoadEventData eventData;
            eventData.type = SaveLoadEventType::LoadStarted;
            eventData.slotNumber = slotNumber;
            eventData.slotName = "slot_" + std::to_string(slotNumber);
            sendEvent(eventData);
            
            auto startTime = std::chrono::steady_clock::now();
            
            LoadResult result = m_saveIntegration->loadGameState(slotNumber);
            
            auto endTime = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(endTime - startTime).count();
            
            handleLoadCompletion(result, "slot_" + std::to_string(slotNumber), callback);
            
            // Send completion event
            eventData.type = (result == LoadResult::Success) ? 
                SaveLoadEventType::LoadCompleted : SaveLoadEventType::LoadFailed;
            eventData.duration = duration;
            if (result != LoadResult::Success) {
                eventData.errorMessage = getLastError();
            }
            sendEvent(eventData);
        }
        
        void SaveLoadManager::loadGame(const std::string& slotName, std::function<void(bool, const std::string&)> callback) {
            if (!m_initialized || m_isLoading) {
                if (callback) callback(false, "Load operation already in progress");
                return;
            }
            
            m_isLoading = true;
            clearError();
            
            // Send load started event
            SaveLoadEventData eventData;
            eventData.type = SaveLoadEventType::LoadStarted;
            eventData.slotName = slotName;
            sendEvent(eventData);
            
            auto startTime = std::chrono::steady_clock::now();
            
            LoadResult result = m_saveIntegration->loadGameState(slotName);
            
            auto endTime = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(endTime - startTime).count();
            
            handleLoadCompletion(result, slotName, callback);
            
            // Send completion event
            eventData.type = (result == LoadResult::Success) ? 
                SaveLoadEventType::LoadCompleted : SaveLoadEventType::LoadFailed;
            eventData.duration = duration;
            if (result != LoadResult::Success) {
                eventData.errorMessage = getLastError();
            }
            sendEvent(eventData);
        }
        
        void SaveLoadManager::quickSave(std::function<void(bool, const std::string&)> callback) {
            // Send quick save event
            SaveLoadEventData eventData;
            eventData.type = SaveLoadEventType::QuickSaveTriggered;
            eventData.slotName = "quicksave";
            sendEvent(eventData);
            
            saveGame("quicksave", callback);
        }
        
        void SaveLoadManager::quickLoad(std::function<void(bool, const std::string&)> callback) {
            // Send quick load event
            SaveLoadEventData eventData;
            eventData.type = SaveLoadEventType::QuickLoadTriggered;
            eventData.slotName = "quicksave";
            sendEvent(eventData);
            
            loadGame("quicksave", callback);
        }
        
        void SaveLoadManager::autoSave(std::function<void(bool, const std::string&)> callback) {
            if (!isAutoSaveEnabled()) {
                if (callback) callback(false, "Auto-save is disabled");
                return;
            }
            
            SaveIntegrationResult result = m_saveIntegration->autoSave();
            
            bool success = (result == SaveIntegrationResult::Success);
            std::string message = success ? "Auto-save completed" : getLastError();
            
            if (callback) {
                callback(success, message);
            }
        }
        
        bool SaveLoadManager::deleteSave(int slotNumber) {
            if (!m_initialized) {
                return false;
            }
            
            auto saveManager = m_saveIntegration->getSaveManager();
            if (!saveManager) {
                return false;
            }
            
            bool result = saveManager->deleteSave(slotNumber);
            
            // Refresh UI if visible
            if (m_ui && m_ui->isVisible()) {
                m_ui->refreshSaveSlots();
            }
            
            return result;
        }
        
        bool SaveLoadManager::deleteSave(const std::string& slotName) {
            if (!m_initialized) {
                return false;
            }
            
            auto saveManager = m_saveIntegration->getSaveManager();
            if (!saveManager) {
                return false;
            }
            
            bool result = saveManager->deleteSave(slotName);
            
            // Refresh UI if visible
            if (m_ui && m_ui->isVisible()) {
                m_ui->refreshSaveSlots();
            }
            
            return result;
        }
        
        void SaveLoadManager::showSaveMenu() {
            if (m_ui) {
                m_ui->showSaveMenu();
            }
        }
        
        void SaveLoadManager::showLoadMenu() {
            if (m_ui) {
                m_ui->showLoadMenu();
            }
        }
        
        void SaveLoadManager::hideUI() {
            if (m_ui) {
                m_ui->hide();
            }
        }
        
        bool SaveLoadManager::isUIVisible() const {
            return m_ui && m_ui->isVisible();
        }
        
        void SaveLoadManager::setAutoSaveEnabled(bool enabled) {
            if (m_saveIntegration) {
                m_saveIntegration->setAutoSaveEnabled(enabled);
            }
        }
        
        bool SaveLoadManager::isAutoSaveEnabled() const {
            return m_saveIntegration && m_saveIntegration->isAutoSaveEnabled();
        }
        
        void SaveLoadManager::setMaxSaveSlots(int maxSlots) {
            if (m_saveIntegration) {
                auto saveManager = m_saveIntegration->getSaveManager();
                if (saveManager) {
                    saveManager->setMaxSaveSlots(maxSlots);
                }
            }
            
            if (m_ui) {
                auto config = m_ui->getConfig();
                config.maxSaveSlots = maxSlots;
                m_ui->setConfig(config);
            }
        }
        
        int SaveLoadManager::getMaxSaveSlots() const {
            if (m_saveIntegration) {
                auto saveManager = m_saveIntegration->getSaveManager();
                if (saveManager) {
                    return saveManager->getMaxSaveSlots();
                }
            }
            return 10; // Default
        }
        
        void SaveLoadManager::registerEventCallback(std::function<void(const SaveLoadEventData&)> callback) {
            m_eventCallbacks.push_back(callback);
        }
        
        void SaveLoadManager::onGameEvent(const std::string& eventType) {
            if (!isAutoSaveEnabled()) {
                return;
            }
            
            bool shouldTriggerAutoSave = false;
            
            if (eventType == "map_transition" && m_autoSaveTriggers.onMapTransition) {
                shouldTriggerAutoSave = true;
            } else if (eventType == "level_up" && m_autoSaveTriggers.onLevelUp) {
                shouldTriggerAutoSave = true;
            } else if (eventType == "quest_complete" && m_autoSaveTriggers.onQuestComplete) {
                shouldTriggerAutoSave = true;
            } else if (eventType == "combat_end" && m_autoSaveTriggers.onCombatEnd) {
                shouldTriggerAutoSave = true;
            } else if (eventType == "item_acquired" && m_autoSaveTriggers.onItemAcquired) {
                shouldTriggerAutoSave = true;
            } else if (eventType == "dialogue_end" && m_autoSaveTriggers.onDialogueEnd) {
                shouldTriggerAutoSave = true;
            }
            
            if (shouldTriggerAutoSave) {
                autoSave();
                resetAutoSaveTimer();
            }
        }
        
        float SaveLoadManager::getTimeUntilAutoSave() const {
            if (!isAutoSaveEnabled() || !m_autoSaveTriggers.onTimeInterval) {
                return -1.0f;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAutoSave).count();
            float remaining = m_autoSaveTriggers.timeIntervalSeconds - elapsed;
            
            return std::max(0.0f, remaining);
        }
        
        std::vector<RPGEngine::UI::SaveSlotInfo> SaveLoadManager::getSaveSlots() const {
            if (m_ui) {
                return m_ui->getSaveSlots();
            }
            return {};
        }
        
        void SaveLoadManager::handleSaveCompletion(SaveIntegrationResult result, const std::string& slotName,
                                                 std::function<void(bool, const std::string&)> callback) {
            m_isSaving = false;
            
            bool success = (result == SaveIntegrationResult::Success);
            std::string message;
            
            switch (result) {
                case SaveIntegrationResult::Success:
                    message = "Game saved successfully to " + slotName;
                    break;
                case SaveIntegrationResult::SystemError:
                    message = "System error during save: " + m_saveIntegration->getLastError();
                    setError(message);
                    break;
                case SaveIntegrationResult::DataError:
                    message = "Data error during save: " + m_saveIntegration->getLastError();
                    setError(message);
                    break;
                case SaveIntegrationResult::ValidationError:
                    message = "Validation error during save: " + m_saveIntegration->getLastError();
                    setError(message);
                    break;
                case SaveIntegrationResult::FileError:
                    message = "File error during save: " + m_saveIntegration->getLastError();
                    setError(message);
                    break;
            }
            
            // Refresh UI if visible
            if (m_ui && m_ui->isVisible()) {
                m_ui->refreshSaveSlots();
                if (!success) {
                    m_ui->setError(message);
                }
            }
            
            if (callback) {
                callback(success, message);
            }
        }
        
        void SaveLoadManager::handleLoadCompletion(LoadResult result, const std::string& slotName,
                                                 std::function<void(bool, const std::string&)> callback) {
            m_isLoading = false;
            
            bool success = (result == LoadResult::Success);
            std::string message;
            
            switch (result) {
                case LoadResult::Success:
                    message = "Game loaded successfully from " + slotName;
                    break;
                case LoadResult::FileNotFound:
                    message = "Save file not found: " + slotName;
                    setError(message);
                    break;
                case LoadResult::FileCorrupted:
                    message = "Save file corrupted: " + slotName;
                    setError(message);
                    break;
                case LoadResult::DeserializationError:
                    message = "Failed to load save data: " + m_saveIntegration->getLastError();
                    setError(message);
                    break;
                case LoadResult::ValidationError:
                    message = "Save data validation failed: " + m_saveIntegration->getLastError();
                    setError(message);
                    break;
                case LoadResult::VersionMismatch:
                    message = "Save file version mismatch: " + slotName;
                    setError(message);
                    break;
            }
            
            // Hide UI on successful load
            if (success && m_ui) {
                m_ui->hide();
            } else if (m_ui && m_ui->isVisible()) {
                m_ui->setError(message);
            }
            
            if (callback) {
                callback(success, message);
            }
        }
        
        void SaveLoadManager::sendEvent(const SaveLoadEventData& eventData) {
            // Send to registered callbacks
            for (const auto& callback : m_eventCallbacks) {
                callback(eventData);
            }
            
            // Send to event system if available
            if (m_eventSystem) {
                // Convert to engine event format
                // This would depend on your event system implementation
            }
        }
        
        bool SaveLoadManager::shouldAutoSave() const {
            if (!isAutoSaveEnabled() || !m_autoSaveTriggers.onTimeInterval) {
                return false;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAutoSave).count();
            
            return elapsed >= m_autoSaveTriggers.timeIntervalSeconds;
        }
        
        void SaveLoadManager::resetAutoSaveTimer() {
            m_lastAutoSave = std::chrono::steady_clock::now();
        }
        
    } // namespace Save
} // namespace Engine