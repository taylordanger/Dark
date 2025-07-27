#include "SaveLoadUI.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace RPGEngine {
    namespace UI {
        
        SaveLoadUI::SaveLoadUI()
            : m_state(SaveLoadUIState::Hidden)
            , m_selectedSlot(-1)
            , m_confirmSlot(-1)
            , m_loadingTimer(0.0f)
            , m_initialized(false)
        {
        }
        
        SaveLoadUI::~SaveLoadUI() {
            shutdown();
        }
        
        bool SaveLoadUI::initialize(std::shared_ptr<Engine::Save::SaveIntegration> saveIntegration) {
            if (m_initialized) {
                return true;
            }
            
            if (!saveIntegration) {
                return false;
            }
            
            m_saveIntegration = saveIntegration;
            
            // Set up default configuration
            m_config = SaveLoadUIConfig();
            
            // Initialize save slots
            refreshSaveSlots();
            
            m_initialized = true;
            return true;
        }
        
        void SaveLoadUI::shutdown() {
            m_saveIntegration.reset();
            m_saveSlots.clear();
            m_initialized = false;
        }
        
        void SaveLoadUI::update(float deltaTime) {
            if (!m_initialized || m_state == SaveLoadUIState::Hidden) {
                return;
            }
            
            // Update loading timer
            if (isLoading()) {
                m_loadingTimer += deltaTime;
            }
            
            // Auto-refresh save slots periodically
            static float refreshTimer = 0.0f;
            refreshTimer += deltaTime;
            if (refreshTimer >= 5.0f) { // Refresh every 5 seconds
                refreshSaveSlots();
                refreshTimer = 0.0f;
            }
        }
        
        void SaveLoadUI::render() {
            if (!m_initialized || m_state == SaveLoadUIState::Hidden) {
                return;
            }
            
            switch (m_state) {
                case SaveLoadUIState::SaveMenu:
                    renderSaveMenu();
                    break;
                    
                case SaveLoadUIState::LoadMenu:
                    renderLoadMenu();
                    break;
                    
                case SaveLoadUIState::ConfirmOverwrite:
                case SaveLoadUIState::ConfirmDelete:
                    renderConfirmationDialog();
                    break;
                    
                case SaveLoadUIState::Error:
                    renderErrorDialog();
                    break;
                    
                case SaveLoadUIState::Loading:
                case SaveLoadUIState::Saving:
                    renderLoadingOverlay();
                    break;
                    
                default:
                    break;
            }
        }
        
        bool SaveLoadUI::handleInput(const std::string& event) {
            if (!m_initialized || m_state == SaveLoadUIState::Hidden) {
                return false;
            }
            
            // Handle common inputs
            if (event == "escape" || event == "cancel") {
                if (m_state == SaveLoadUIState::ConfirmOverwrite || 
                    m_state == SaveLoadUIState::ConfirmDelete ||
                    m_state == SaveLoadUIState::Error) {
                    m_state = (m_state == SaveLoadUIState::Error) ? SaveLoadUIState::Hidden : 
                             (m_selectedSlot >= 0 ? SaveLoadUIState::SaveMenu : SaveLoadUIState::LoadMenu);
                } else {
                    hide();
                    if (m_callbacks.onCancel) {
                        m_callbacks.onCancel();
                    }
                }
                return true;
            }
            
            // Handle state-specific inputs
            switch (m_state) {
                case SaveLoadUIState::SaveMenu:
                case SaveLoadUIState::LoadMenu:
                    // Handle slot selection (would typically be handled by UI framework)
                    // This is a simplified example
                    if (event.find("slot_") == 0) {
                        try {
                            int slotIndex = std::stoi(event.substr(5));
                            if (m_state == SaveLoadUIState::SaveMenu) {
                                handleSaveSlotSelection(slotIndex);
                            } else {
                                handleLoadSlotSelection(slotIndex);
                            }
                            return true;
                        } catch (const std::exception& e) {
                            // Invalid slot number
                        }
                    }
                    break;
                    
                case SaveLoadUIState::ConfirmOverwrite:
                    if (event == "confirm" || event == "yes") {
                        if (m_callbacks.onSaveSlotSelected && m_confirmSlot >= 0) {
                            m_callbacks.onSaveSlotSelected(m_confirmSlot);
                            m_state = SaveLoadUIState::Saving;
                        }
                        return true;
                    }
                    break;
                    
                case SaveLoadUIState::ConfirmDelete:
                    if (event == "confirm" || event == "yes") {
                        if (m_callbacks.onDeleteSlot && m_confirmSlot >= 0) {
                            m_callbacks.onDeleteSlot(m_confirmSlot);
                            refreshSaveSlots();
                            m_state = SaveLoadUIState::SaveMenu;
                        }
                        return true;
                    }
                    break;
                    
                default:
                    break;
            }
            
            return false;
        }
        
        void SaveLoadUI::showSaveMenu() {
            if (!m_initialized) {
                return;
            }
            
            m_state = SaveLoadUIState::SaveMenu;
            m_selectedSlot = -1;
            refreshSaveSlots();
            clearError();
        }
        
        void SaveLoadUI::showLoadMenu() {
            if (!m_initialized) {
                return;
            }
            
            m_state = SaveLoadUIState::LoadMenu;
            m_selectedSlot = -1;
            refreshSaveSlots();
            clearError();
        }
        
        void SaveLoadUI::hide() {
            m_state = SaveLoadUIState::Hidden;
            m_selectedSlot = -1;
            m_confirmSlot = -1;
            clearError();
        }
        
        void SaveLoadUI::refreshSaveSlots() {
            if (!m_initialized || !m_saveIntegration) {
                return;
            }
            
            m_saveSlots.clear();
            
            auto saveManager = m_saveIntegration->getSaveManager();
            if (!saveManager) {
                return;
            }
            
            // Get all save files
            auto saveList = saveManager->getSaveList();
            
            // Create slot info for numbered slots
            for (int i = 0; i < m_config.maxSaveSlots; ++i) {
                bool found = false;
                
                // Look for this slot in the save list
                for (const auto& saveInfo : saveList) {
                    std::string expectedSlotName = "slot_" + std::to_string(i);
                    if (saveInfo.slotName == expectedSlotName) {
                        m_saveSlots.push_back(createSaveSlotInfo(saveInfo, i));
                        found = true;
                        break;
                    }
                }
                
                // Create empty slot if not found
                if (!found) {
                    m_saveSlots.push_back(createEmptySaveSlotInfo(i));
                }
            }
            
            // Add special slots if configured
            if (m_config.showQuickSave) {
                for (const auto& saveInfo : saveList) {
                    if (saveInfo.slotName == "quicksave") {
                        SaveSlotInfo quickSaveSlot = createSaveSlotInfo(saveInfo, -1);
                        quickSaveSlot.displayName = "Quick Save";
                        quickSaveSlot.isQuickSave = true;
                        m_saveSlots.insert(m_saveSlots.begin(), quickSaveSlot);
                        break;
                    }
                }
            }
            
            // Add auto-save slots if configured
            if (m_config.showAutoSaves) {
                std::vector<SaveSlotInfo> autoSaveSlots;
                
                for (const auto& saveInfo : saveList) {
                    if (saveInfo.slotName.find("autosave_") == 0) {
                        SaveSlotInfo autoSaveSlot = createSaveSlotInfo(saveInfo, -1);
                        autoSaveSlot.displayName = "Auto Save";
                        autoSaveSlot.isAutoSave = true;
                        autoSaveSlots.push_back(autoSaveSlot);
                    }
                }
                
                // Sort auto-saves by timestamp (newest first)
                std::sort(autoSaveSlots.begin(), autoSaveSlots.end(),
                    [](const SaveSlotInfo& a, const SaveSlotInfo& b) {
                        return a.timestamp > b.timestamp;
                    });
                
                // Add auto-saves to the beginning
                m_saveSlots.insert(m_saveSlots.begin(), autoSaveSlots.begin(), autoSaveSlots.end());
            }
        }
        
        void SaveLoadUI::setError(const std::string& error) {
            m_errorMessage = error;
            m_state = SaveLoadUIState::Error;
            
            if (m_callbacks.onError) {
                m_callbacks.onError(error);
            }
        }
        
        void SaveLoadUI::clearError() {
            m_errorMessage.clear();
        }
        
        void SaveLoadUI::setLoading(bool loading) {
            if (loading) {
                m_state = SaveLoadUIState::Loading;
                m_loadingTimer = 0.0f;
            } else if (isLoading()) {
                m_state = SaveLoadUIState::Hidden;
            }
        }
        
        void SaveLoadUI::renderSaveMenu() {
            // This is a simplified text-based rendering example
            // In a real implementation, you would use your UI framework
            
            std::cout << "=== SAVE GAME ===" << std::endl;
            std::cout << "Select a save slot:" << std::endl;
            std::cout << std::endl;
            
            for (size_t i = 0; i < m_saveSlots.size(); ++i) {
                const auto& slot = m_saveSlots[i];
                
                if (renderSaveSlot(slot, static_cast<int>(i), true)) {
                    handleSaveSlotSelection(static_cast<int>(i));
                }
            }
            
            std::cout << std::endl;
            std::cout << "[ESC] Cancel" << std::endl;
        }
        
        void SaveLoadUI::renderLoadMenu() {
            std::cout << "=== LOAD GAME ===" << std::endl;
            std::cout << "Select a save slot:" << std::endl;
            std::cout << std::endl;
            
            for (size_t i = 0; i < m_saveSlots.size(); ++i) {
                const auto& slot = m_saveSlots[i];
                
                if (!slot.isEmpty && renderSaveSlot(slot, static_cast<int>(i), false)) {
                    handleLoadSlotSelection(static_cast<int>(i));
                }
            }
            
            std::cout << std::endl;
            std::cout << "[ESC] Cancel" << std::endl;
        }
        
        bool SaveLoadUI::renderSaveSlot(const SaveSlotInfo& slot, int index, bool isSaveMode) {
            std::string prefix = "[" + std::to_string(index + 1) + "] ";
            
            if (slot.isEmpty) {
                if (isSaveMode) {
                    std::cout << prefix << "Empty Slot" << std::endl;
                    return false; // Can save to empty slot
                } else {
                    return false; // Can't load from empty slot
                }
            } else {
                std::cout << prefix << slot.displayName;
                
                if (!slot.isAutoSave && !slot.isQuickSave) {
                    std::cout << " (Slot " << slot.slotNumber << ")";
                }
                
                std::cout << std::endl;
                std::cout << "    Level " << slot.playerLevel;
                std::cout << " - " << slot.currentMap;
                std::cout << " - " << formatTimestamp(slot.timestamp);
                
                if (m_config.showFileSize) {
                    std::cout << " (" << formatFileSize(slot.fileSize) << ")";
                }
                
                std::cout << std::endl;
                
                if (isSaveMode && m_config.allowSlotDeletion && !slot.isAutoSave) {
                    std::cout << "    [D] Delete" << std::endl;
                }
                
                std::cout << std::endl;
                return true;
            }
        }
        
        void SaveLoadUI::renderConfirmationDialog() {
            if (m_state == SaveLoadUIState::ConfirmOverwrite) {
                std::cout << "=== CONFIRM OVERWRITE ===" << std::endl;
                std::cout << "This will overwrite the existing save file." << std::endl;
                std::cout << "Are you sure you want to continue?" << std::endl;
                std::cout << std::endl;
                std::cout << "[Y] Yes  [N] No" << std::endl;
            } else if (m_state == SaveLoadUIState::ConfirmDelete) {
                std::cout << "=== CONFIRM DELETE ===" << std::endl;
                std::cout << "This will permanently delete the save file." << std::endl;
                std::cout << "Are you sure you want to continue?" << std::endl;
                std::cout << std::endl;
                std::cout << "[Y] Yes  [N] No" << std::endl;
            }
        }
        
        void SaveLoadUI::renderErrorDialog() {
            std::cout << "=== ERROR ===" << std::endl;
            std::cout << m_errorMessage << std::endl;
            std::cout << std::endl;
            std::cout << "[ESC] Close" << std::endl;
        }
        
        void SaveLoadUI::renderLoadingOverlay() {
            std::string action = (m_state == SaveLoadUIState::Saving) ? "Saving" : "Loading";
            
            std::cout << "=== " << action.c_str() << " ===" << std::endl;
            std::cout << action << " game data";
            
            // Simple loading animation
            int dots = static_cast<int>(m_loadingTimer * 2) % 4;
            for (int i = 0; i < dots; ++i) {
                std::cout << ".";
            }
            
            std::cout << std::endl;
            std::cout << "Please wait..." << std::endl;
        }
        
        void SaveLoadUI::handleSaveSlotSelection(int slotIndex) {
            if (slotIndex < 0 || slotIndex >= static_cast<int>(m_saveSlots.size())) {
                return;
            }
            
            const auto& slot = m_saveSlots[slotIndex];
            
            // Check if we need to confirm overwrite
            if (!slot.isEmpty && m_config.confirmOverwrite && !slot.isAutoSave) {
                m_confirmSlot = slotIndex;
                m_state = SaveLoadUIState::ConfirmOverwrite;
                return;
            }
            
            // Proceed with save
            if (m_callbacks.onSaveSlotSelected) {
                m_callbacks.onSaveSlotSelected(slot.slotNumber);
                m_state = SaveLoadUIState::Saving;
            }
        }
        
        void SaveLoadUI::handleLoadSlotSelection(int slotIndex) {
            if (slotIndex < 0 || slotIndex >= static_cast<int>(m_saveSlots.size())) {
                return;
            }
            
            const auto& slot = m_saveSlots[slotIndex];
            
            if (slot.isEmpty) {
                return; // Can't load from empty slot
            }
            
            if (m_callbacks.onLoadSlotSelected) {
                m_callbacks.onLoadSlotSelected(slot.slotNumber);
                m_state = SaveLoadUIState::Loading;
            }
        }
        
        void SaveLoadUI::handleSlotDeletion(int slotIndex) {
            if (slotIndex < 0 || slotIndex >= static_cast<int>(m_saveSlots.size())) {
                return;
            }
            
            const auto& slot = m_saveSlots[slotIndex];
            
            if (slot.isEmpty || slot.isAutoSave) {
                return; // Can't delete empty or auto-save slots
            }
            
            m_confirmSlot = slotIndex;
            m_state = SaveLoadUIState::ConfirmDelete;
        }
        
        std::string SaveLoadUI::formatTimestamp(const std::string& timestamp) const {
            if (timestamp.empty()) {
                return "Unknown";
            }
            
            // Simple timestamp formatting
            // In a real implementation, you would parse the ISO timestamp and format it properly
            return timestamp.substr(0, 19); // Remove timezone info for display
        }
        
        std::string SaveLoadUI::formatFileSize(size_t fileSize) const {
            if (fileSize < 1024) {
                return std::to_string(fileSize) + " B";
            } else if (fileSize < 1024 * 1024) {
                return std::to_string(fileSize / 1024) + " KB";
            } else {
                return std::to_string(fileSize / (1024 * 1024)) + " MB";
            }
        }
        
        SaveSlotInfo SaveLoadUI::createSaveSlotInfo(const Engine::Save::SaveManager::SaveInfo& saveInfo, int slotNumber) const {
            SaveSlotInfo slot;
            slot.slotNumber = slotNumber;
            slot.slotName = saveInfo.slotName;
            slot.displayName = (slotNumber >= 0) ? ("Save Slot " + std::to_string(slotNumber + 1)) : saveInfo.slotName;
            slot.timestamp = saveInfo.timestamp;
            slot.playerLevel = saveInfo.playerLevel;
            slot.currentMap = saveInfo.currentMap;
            slot.fileSize = saveInfo.fileSize;
            slot.isEmpty = false;
            slot.isAutoSave = saveInfo.slotName.find("autosave_") == 0;
            slot.isQuickSave = saveInfo.slotName == "quicksave";
            
            return slot;
        }
        
        SaveSlotInfo SaveLoadUI::createEmptySaveSlotInfo(int slotNumber) const {
            SaveSlotInfo slot;
            slot.slotNumber = slotNumber;
            slot.slotName = "slot_" + std::to_string(slotNumber);
            slot.displayName = "Save Slot " + std::to_string(slotNumber + 1);
            slot.isEmpty = true;
            
            return slot;
        }
        
    } // namespace UI
} // namespace RPGEngine