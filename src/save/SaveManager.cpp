#include "SaveManager.h"
#include "JsonSaveSerializer.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>

namespace Engine {
    namespace Save {
        
        SaveManager::SaveManager()
            : m_maxSaveSlots(10)
            , m_autoSaveEnabled(true)
            , m_autoSaveInterval(300.0f) // 5 minutes
            , m_backupEnabled(true)
            , m_maxBackups(3)
            , m_compressionEnabled(false)
            , m_checksumValidation(true)
            , m_initialized(false)
        {
        }
        
        SaveManager::~SaveManager() {
            shutdown();
        }
        
        bool SaveManager::initialize(const std::string& saveDirectory) {
            if (m_initialized) {
                return true;
            }
            
            m_saveDirectory = saveDirectory;
            
            // Create save directory if it doesn't exist
            try {
                if (!std::filesystem::exists(m_saveDirectory)) {
                    std::filesystem::create_directories(m_saveDirectory);
                }
            } catch (const std::exception& e) {
                setError("Failed to create save directory: " + std::string(e.what()));
                return false;
            }
            
            // Set default JSON serializer if none provided
            if (!m_serializer) {
                m_serializer = std::make_unique<JsonSaveSerializer>();
            }
            
            m_initialized = true;
            clearError();
            return true;
        }
        
        void SaveManager::shutdown() {
            m_serializer.reset();
            m_validationCallback = nullptr;
            m_initialized = false;
        }
        
        SaveResult SaveManager::saveGame(const SaveData& data, const std::string& slotName) {
            if (!m_initialized) {
                setError("SaveManager not initialized");
                return SaveResult::FileError;
            }
            
            // Validate save data
            if (!validateSaveData(data)) {
                return SaveResult::ValidationError;
            }
            
            std::string filePath = getSlotFileName(slotName);
            
            // Create backup if enabled and file exists
            if (m_backupEnabled && std::filesystem::exists(filePath)) {
                if (!createBackup(filePath)) {
                    setError("Failed to create backup");
                    return SaveResult::FileError;
                }
            }
            
            return writeToFile(filePath, data);
        }
        
        SaveResult SaveManager::saveGame(const SaveData& data, int slotNumber) {
            if (slotNumber < 0 || slotNumber >= m_maxSaveSlots) {
                setError("Invalid save slot number: " + std::to_string(slotNumber));
                return SaveResult::ValidationError;
            }
            
            return saveGame(data, "slot_" + std::to_string(slotNumber));
        }
        
        SaveResult SaveManager::autoSave(const SaveData& data) {
            if (!m_autoSaveEnabled) {
                setError("Auto-save is disabled");
                return SaveResult::ValidationError;
            }
            
            return saveGame(data, "autosave");
        }
        
        LoadResult SaveManager::loadGame(SaveData& outData, const std::string& slotName) {
            if (!m_initialized) {
                setError("SaveManager not initialized");
                return LoadResult::FileNotFound;
            }
            
            std::string filePath = getSlotFileName(slotName);
            
            if (!std::filesystem::exists(filePath)) {
                setError("Save file not found: " + slotName);
                return LoadResult::FileNotFound;
            }
            
            return readFromFile(filePath, outData);
        }
        
        LoadResult SaveManager::loadGame(SaveData& outData, int slotNumber) {
            if (slotNumber < 0 || slotNumber >= m_maxSaveSlots) {
                setError("Invalid save slot number: " + std::to_string(slotNumber));
                return LoadResult::ValidationError;
            }
            
            return loadGame(outData, "slot_" + std::to_string(slotNumber));
        }
        
        LoadResult SaveManager::loadAutoSave(SaveData& outData) {
            return loadGame(outData, "autosave");
        }
        
        bool SaveManager::saveExists(const std::string& slotName) const {
            if (!m_initialized) return false;
            return std::filesystem::exists(getSlotFileName(slotName));
        }
        
        bool SaveManager::saveExists(int slotNumber) const {
            if (slotNumber < 0 || slotNumber >= m_maxSaveSlots) return false;
            return saveExists("slot_" + std::to_string(slotNumber));
        }
        
        bool SaveManager::deleteSave(const std::string& slotName) {
            if (!m_initialized) return false;
            
            std::string filePath = getSlotFileName(slotName);
            
            try {
                if (std::filesystem::exists(filePath)) {
                    std::filesystem::remove(filePath);
                    
                    // Also remove backups
                    cleanupOldBackups(filePath);
                    return true;
                }
            } catch (const std::exception& e) {
                setError("Failed to delete save file: " + std::string(e.what()));
            }
            
            return false;
        }
        
        bool SaveManager::deleteSave(int slotNumber) {
            if (slotNumber < 0 || slotNumber >= m_maxSaveSlots) return false;
            return deleteSave("slot_" + std::to_string(slotNumber));
        }
        
        std::vector<SaveManager::SaveInfo> SaveManager::getSaveList() const {
            std::vector<SaveInfo> saveList;
            
            if (!m_initialized) return saveList;
            
            try {
                for (const auto& entry : std::filesystem::directory_iterator(m_saveDirectory)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".sav") {
                        SaveInfo info;
                        std::string filename = entry.path().stem().string();
                        
                        if (getSaveInfo(filename, info)) {
                            saveList.push_back(info);
                        }
                    }
                }
            } catch (const std::exception& e) {
                // Log error but continue
            }
            
            // Sort by timestamp (newest first)
            std::sort(saveList.begin(), saveList.end(), 
                [](const SaveInfo& a, const SaveInfo& b) {
                    return a.timestamp > b.timestamp;
                });
            
            return saveList;
        }
        
        bool SaveManager::getSaveInfo(const std::string& slotName, SaveInfo& outInfo) const {
            if (!m_initialized || !saveExists(slotName)) return false;
            
            std::string filePath = getSlotFileName(slotName);
            
            try {
                // Read basic file info
                auto fileTime = std::filesystem::last_write_time(filePath);
                auto fileSize = std::filesystem::file_size(filePath);
                
                // Try to read save data for detailed info
                SaveData saveData;
                if (readFromFile(filePath, const_cast<SaveData&>(saveData)) == LoadResult::Success) {
                    outInfo.slotName = slotName;
                    outInfo.timestamp = saveData.timestamp;
                    outInfo.version = saveData.version;
                    outInfo.playerLevel = saveData.player.stats.level;
                    outInfo.currentMap = saveData.world.currentMap;
                    outInfo.fileSize = fileSize;
                    return true;
                }
            } catch (const std::exception& e) {
                // Log error but continue
            }
            
            return false;
        }
        
        bool SaveManager::getSaveInfo(int slotNumber, SaveInfo& outInfo) const {
            if (slotNumber < 0 || slotNumber >= m_maxSaveSlots) return false;
            return getSaveInfo("slot_" + std::to_string(slotNumber), outInfo);
        }
        
        void SaveManager::setSerializer(std::unique_ptr<ISaveSerializer> serializer) {
            m_serializer = std::move(serializer);
        }
        
        std::string SaveManager::getSlotFileName(const std::string& slotName) const {
            return m_saveDirectory + "/" + slotName + ".sav";
        }
        
        std::string SaveManager::getSlotFileName(int slotNumber) const {
            return getSlotFileName("slot_" + std::to_string(slotNumber));
        }
        
        std::string SaveManager::getBackupFileName(const std::string& originalFile, int backupIndex) const {
            return originalFile + ".bak" + std::to_string(backupIndex);
        }
        
        bool SaveManager::validateSaveData(const SaveData& data) {
            // Version validation
            if (data.version.empty()) {
                setError("Save data version is empty");
                return false;
            }
            
            if (!isVersionSupported(data.version)) {
                setError("Unsupported save data version: " + data.version);
                return false;
            }
            
            // Player data validation
            if (data.player.stats.level < 1 || data.player.stats.level > 999) {
                setError("Invalid player level: " + std::to_string(data.player.stats.level));
                return false;
            }
            
            if (data.player.stats.maxHp <= 0 || data.player.stats.maxHp > 99999) {
                setError("Invalid player max HP: " + std::to_string(data.player.stats.maxHp));
                return false;
            }
            
            if (data.player.stats.hp < 0 || data.player.stats.hp > data.player.stats.maxHp) {
                setError("Invalid player HP: " + std::to_string(data.player.stats.hp));
                return false;
            }
            
            if (data.player.stats.maxMp < 0 || data.player.stats.maxMp > 99999) {
                setError("Invalid player max MP: " + std::to_string(data.player.stats.maxMp));
                return false;
            }
            
            if (data.player.stats.mp < 0 || data.player.stats.mp > data.player.stats.maxMp) {
                setError("Invalid player MP: " + std::to_string(data.player.stats.mp));
                return false;
            }
            
            if (data.player.stats.experience < 0) {
                setError("Invalid player experience: " + std::to_string(data.player.stats.experience));
                return false;
            }
            
            if (data.player.stats.experienceToNext <= 0) {
                setError("Invalid experience to next level: " + std::to_string(data.player.stats.experienceToNext));
                return false;
            }
            
            // Inventory validation
            for (const auto& item : data.player.inventory) {
                if (item.id.empty()) {
                    setError("Inventory item has empty ID");
                    return false;
                }
                if (item.quantity <= 0 || item.quantity > 9999) {
                    setError("Invalid inventory item quantity: " + std::to_string(item.quantity));
                    return false;
                }
            }
            
            // World data validation
            if (data.world.currentMap.empty()) {
                setError("Current map is empty");
                return false;
            }
            
            // Validate quest names are not empty
            for (const auto& quest : data.world.completedQuests) {
                if (quest.empty()) {
                    setError("Completed quest has empty name");
                    return false;
                }
            }
            
            // Custom validation callback
            if (m_validationCallback && !m_validationCallback(data)) {
                setError("Custom validation failed");
                return false;
            }
            
            return true;
        }
        
        bool SaveManager::createBackup(const std::string& filePath) {
            try {
                // Shift existing backups
                for (int i = m_maxBackups - 1; i > 0; --i) {
                    std::string oldBackup = getBackupFileName(filePath, i - 1);
                    std::string newBackup = getBackupFileName(filePath, i);
                    
                    if (std::filesystem::exists(oldBackup)) {
                        std::filesystem::rename(oldBackup, newBackup);
                    }
                }
                
                // Create new backup
                std::string backupPath = getBackupFileName(filePath, 0);
                std::filesystem::copy_file(filePath, backupPath);
                
                return true;
            } catch (const std::exception& e) {
                setError("Failed to create backup: " + std::string(e.what()));
                return false;
            }
        }
        
        void SaveManager::cleanupOldBackups(const std::string& baseFileName) {
            try {
                for (int i = 0; i < m_maxBackups; ++i) {
                    std::string backupFile = getBackupFileName(baseFileName, i);
                    if (std::filesystem::exists(backupFile)) {
                        std::filesystem::remove(backupFile);
                    }
                }
            } catch (const std::exception& e) {
                // Log error but continue
            }
        }
        
        SaveResult SaveManager::writeToFile(const std::string& filePath, const SaveData& data) {
            try {
                // Create a copy of data with current timestamp
                SaveData dataWithTimestamp = data;
                
                // Generate timestamp
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                std::stringstream ss;
                ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
                dataWithTimestamp.timestamp = ss.str();
                
                // Serialize data
                std::string serializedData = m_serializer->serialize(dataWithTimestamp);
                if (serializedData.empty()) {
                    setError("Failed to serialize save data");
                    return SaveResult::SerializationError;
                }
                
                // Compress data if enabled
                std::string processedData = compressData(serializedData);
                
                // Calculate checksum if enabled
                std::string checksum;
                if (m_checksumValidation) {
                    checksum = calculateChecksum(processedData);
                }
                
                // Write to file
                std::ofstream file(filePath, std::ios::binary);
                if (!file.is_open()) {
                    setError("Failed to open file for writing: " + filePath);
                    return SaveResult::FileError;
                }
                
                // Write header with metadata
                if (m_checksumValidation) {
                    file << "CHECKSUM:" << checksum << "\n";
                }
                if (m_compressionEnabled) {
                    file << "COMPRESSED:1\n";
                } else {
                    file << "COMPRESSED:0\n";
                }
                file << "DATA:\n";
                
                file.write(processedData.c_str(), processedData.size());
                file.close();
                
                if (file.fail()) {
                    setError("Failed to write to file: " + filePath);
                    return SaveResult::FileError;
                }
                
                clearError();
                return SaveResult::Success;
                
            } catch (const std::exception& e) {
                setError("Exception during save: " + std::string(e.what()));
                return SaveResult::FileError;
            }
        }
        
        LoadResult SaveManager::readFromFile(const std::string& filePath, SaveData& outData) const {
            try {
                // Read file
                std::ifstream file(filePath, std::ios::binary);
                if (!file.is_open()) {
                    setError("Failed to open file for reading: " + filePath);
                    return LoadResult::FileNotFound;
                }
                
                std::string fileContent((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                file.close();
                
                if (fileContent.empty()) {
                    setError("Save file is empty or corrupted");
                    return LoadResult::FileCorrupted;
                }
                
                // Parse header and extract data
                std::string serializedData;
                std::string expectedChecksum;
                bool isCompressed = false;
                
                std::istringstream stream(fileContent);
                std::string line;
                bool dataSection = false;
                
                while (std::getline(stream, line)) {
                    if (line.find("CHECKSUM:") == 0) {
                        expectedChecksum = line.substr(9);
                    } else if (line.find("COMPRESSED:") == 0) {
                        isCompressed = (line.substr(11) == "1");
                    } else if (line == "DATA:") {
                        dataSection = true;
                    } else if (dataSection) {
                        if (!serializedData.empty()) {
                            serializedData += "\n";
                        }
                        serializedData += line;
                    }
                }
                
                // If no header found, assume it's an old format file
                if (!dataSection) {
                    serializedData = fileContent;
                }
                
                // Verify checksum if available
                if (!expectedChecksum.empty() && !verifyChecksum(serializedData, expectedChecksum)) {
                    setError("Save file checksum verification failed");
                    return LoadResult::FileCorrupted;
                }
                
                // Decompress data if needed
                if (isCompressed) {
                    serializedData = decompressData(serializedData);
                }
                
                // Deserialize data
                if (!m_serializer->deserialize(serializedData, outData)) {
                    setError("Failed to deserialize save data");
                    return LoadResult::DeserializationError;
                }
                
                // Validate loaded data
                if (!const_cast<SaveManager*>(this)->validateSaveData(outData)) {
                    return LoadResult::ValidationError;
                }
                
                clearError();
                return LoadResult::Success;
                
            } catch (const std::exception& e) {
                setError("Exception during load: " + std::string(e.what()));
                return LoadResult::FileCorrupted;
            }
        }
        
        std::string SaveManager::compressData(const std::string& data) {
            // Simple compression placeholder - in a real implementation, 
            // you would use a library like zlib or lz4
            if (!m_compressionEnabled) {
                return data;
            }
            
            // For now, just return the original data
            // TODO: Implement actual compression
            return data;
        }
        
        std::string SaveManager::decompressData(const std::string& compressedData) const {
            // Simple decompression placeholder
            if (!m_compressionEnabled) {
                return compressedData;
            }
            
            // For now, just return the original data
            // TODO: Implement actual decompression
            return compressedData;
        }
        
        std::string SaveManager::calculateChecksum(const std::string& data) const {
            // Simple checksum calculation using hash
            std::hash<std::string> hasher;
            size_t hashValue = hasher(data);
            
            // Convert to hex string
            std::stringstream ss;
            ss << std::hex << hashValue;
            return ss.str();
        }
        
        bool SaveManager::verifyChecksum(const std::string& data, const std::string& expectedChecksum) const {
            if (!m_checksumValidation) {
                return true;
            }
            
            std::string actualChecksum = calculateChecksum(data);
            return actualChecksum == expectedChecksum;
        }
        
        bool SaveManager::migrateSaveData(SaveData& data, const std::string& fromVersion, const std::string& toVersion) {
            if (fromVersion == toVersion) {
                return true; // No migration needed
            }
            
            // Version migration logic
            if (fromVersion == "1.0" && toVersion == "2.0") {
                return migrateFromV1ToV2(data);
            }
            
            // Add more migration paths as needed
            setError("Unsupported version migration from " + fromVersion + " to " + toVersion);
            return false;
        }
        
        bool SaveManager::migrateFromV1ToV2(SaveData& data) {
            // Example migration: add new fields with default values
            data.version = "2.0";
            
            // Add any new fields that were introduced in v2.0
            if (data.world.discoveredLocations.empty()) {
                // Initialize with default discovered locations
                data.world.discoveredLocations["starting_town"] = true;
            }
            
            return true;
        }
        
        bool SaveManager::isVersionSupported(const std::string& version) {
            // List of supported versions
            static const std::vector<std::string> supportedVersions = {
                "1.0", "2.0"
            };
            
            return std::find(supportedVersions.begin(), supportedVersions.end(), version) != supportedVersions.end();
        }
        
    } // namespace Save
} // namespace Engine