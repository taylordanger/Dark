#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include "../core/Types.h"

namespace Engine {
    namespace Save {
        
        // Forward declarations
        struct SaveData;
        struct PlayerData;
        struct WorldData;
        
        // Save result enumeration
        enum class SaveResult {
            Success,
            FileError,
            SerializationError,
            ValidationError,
            VersionMismatch
        };
        
        // Load result enumeration
        enum class LoadResult {
            Success,
            FileNotFound,
            FileCorrupted,
            DeserializationError,
            ValidationError,
            VersionMismatch
        };
        
        // Save data validation callback
        using ValidationCallback = std::function<bool(const SaveData&)>;
        
        // Save data serialization interface
        class ISaveSerializer {
        public:
            virtual ~ISaveSerializer() = default;
            virtual std::string serialize(const SaveData& data) = 0;
            virtual bool deserialize(const std::string& data, SaveData& outData) = 0;
            virtual bool supportsCompression() const { return false; }
            virtual bool supportsEncryption() const { return false; }
        };
        
        // Player data structure
        struct PlayerData {
            struct Position {
                float x = 0.0f;
                float y = 0.0f;
            } position;
            
            struct Stats {
                int hp = 100;
                int maxHp = 100;
                int mp = 50;
                int maxMp = 50;
                int level = 1;
                int experience = 0;
                int experienceToNext = 100;
            } stats;
            
            struct InventoryItem {
                std::string id;
                int quantity = 1;
            };
            std::vector<InventoryItem> inventory;
            
            std::unordered_map<std::string, std::string> equipment;
        };
        
        // World data structure
        struct WorldData {
            std::string currentMap;
            std::vector<std::string> completedQuests;
            std::unordered_map<std::string, bool> gameFlags;
            std::unordered_map<std::string, std::string> npcStates;
            std::unordered_map<std::string, bool> discoveredLocations;
        };
        
        // Complete save data structure
        struct SaveData {
            std::string version = "1.0";
            std::string timestamp;
            PlayerData player;
            WorldData world;
            std::unordered_map<std::string, std::string> customData;
        };
        
        // Main SaveManager class
        class SaveManager {
        public:
            SaveManager();
            ~SaveManager();
            
            // Initialization and cleanup
            bool initialize(const std::string& saveDirectory = "saves");
            void shutdown();
            
            // Save operations
            SaveResult saveGame(const SaveData& data, const std::string& slotName = "quicksave");
            SaveResult saveGame(const SaveData& data, int slotNumber);
            SaveResult autoSave(const SaveData& data);
            
            // Load operations
            LoadResult loadGame(SaveData& outData, const std::string& slotName = "quicksave");
            LoadResult loadGame(SaveData& outData, int slotNumber);
            LoadResult loadAutoSave(SaveData& outData);
            
            // Save file management
            bool saveExists(const std::string& slotName) const;
            bool saveExists(int slotNumber) const;
            bool deleteSave(const std::string& slotName);
            bool deleteSave(int slotNumber);
            
            // Save file information
            struct SaveInfo {
                std::string slotName;
                std::string timestamp;
                std::string version;
                std::string playerName;
                int playerLevel;
                std::string currentMap;
                size_t fileSize;
            };
            
            std::vector<SaveInfo> getSaveList() const;
            bool getSaveInfo(const std::string& slotName, SaveInfo& outInfo) const;
            bool getSaveInfo(int slotNumber, SaveInfo& outInfo) const;
            
            // Configuration
            void setMaxSaveSlots(int maxSlots) { m_maxSaveSlots = maxSlots; }
            int getMaxSaveSlots() const { return m_maxSaveSlots; }
            
            void setAutoSaveEnabled(bool enabled) { m_autoSaveEnabled = enabled; }
            bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }
            
            void setAutoSaveInterval(float seconds) { m_autoSaveInterval = seconds; }
            float getAutoSaveInterval() const { return m_autoSaveInterval; }
            
            // Validation
            void setValidationCallback(ValidationCallback callback) { m_validationCallback = callback; }
            
            // Serialization
            void setSerializer(std::unique_ptr<ISaveSerializer> serializer);
            
            // Error handling
            std::string getLastError() const { return m_lastError; }
            
            // Backup management
            void setBackupEnabled(bool enabled) { m_backupEnabled = enabled; }
            bool isBackupEnabled() const { return m_backupEnabled; }
            void setMaxBackups(int maxBackups) { m_maxBackups = maxBackups; }
            
            // Compression and security
            void setCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }
            bool isCompressionEnabled() const { return m_compressionEnabled; }
            void setChecksumValidation(bool enabled) { m_checksumValidation = enabled; }
            bool isChecksumValidationEnabled() const { return m_checksumValidation; }
            
            // Version migration
            bool migrateSaveData(SaveData& data, const std::string& fromVersion, const std::string& toVersion);
            
        private:
            // Internal helper methods
            std::string getSlotFileName(const std::string& slotName) const;
            std::string getSlotFileName(int slotNumber) const;
            std::string getBackupFileName(const std::string& originalFile, int backupIndex) const;
            
            bool validateSaveData(const SaveData& data);
            bool createBackup(const std::string& filePath);
            void cleanupOldBackups(const std::string& baseFileName);
            
            // Data processing helpers
            std::string compressData(const std::string& data);
            std::string decompressData(const std::string& compressedData) const;
            std::string calculateChecksum(const std::string& data) const;
            bool verifyChecksum(const std::string& data, const std::string& expectedChecksum) const;
            
            // Version migration helpers
            bool migrateFromV1ToV2(SaveData& data);
            bool isVersionSupported(const std::string& version);
            
            SaveResult writeToFile(const std::string& filePath, const SaveData& data);
            LoadResult readFromFile(const std::string& filePath, SaveData& outData) const;
            
            void setError(const std::string& error) const { m_lastError = error; }
            void clearError() const { m_lastError.clear(); }
            
            // Member variables
            std::string m_saveDirectory;
            std::unique_ptr<ISaveSerializer> m_serializer;
            ValidationCallback m_validationCallback;
            
            int m_maxSaveSlots;
            bool m_autoSaveEnabled;
            float m_autoSaveInterval;
            bool m_backupEnabled;
            int m_maxBackups;
            bool m_compressionEnabled;
            bool m_checksumValidation;
            
            mutable std::string m_lastError;
            bool m_initialized;
        };
        
    } // namespace Save
} // namespace Engine