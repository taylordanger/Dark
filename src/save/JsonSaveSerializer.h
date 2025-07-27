#pragma once

#include "SaveManager.h"
#include <nlohmann/json.hpp>

namespace Engine {
    namespace Save {
        
        class JsonSaveSerializer : public ISaveSerializer {
        public:
            JsonSaveSerializer() = default;
            ~JsonSaveSerializer() override = default;
            
            std::string serialize(const SaveData& data) override;
            bool deserialize(const std::string& jsonString, SaveData& outData) override;
            
        private:
            // Helper methods for serialization
            nlohmann::json serializePlayerData(const PlayerData& player);
            nlohmann::json serializeWorldData(const WorldData& world);
            
            // Helper methods for deserialization
            bool deserializePlayerData(const nlohmann::json& json, PlayerData& outPlayer);
            bool deserializeWorldData(const nlohmann::json& json, WorldData& outWorld);
            
            // Validation helpers
            bool validateJsonStructure(const nlohmann::json& json);
        };
        
    } // namespace Save
} // namespace Engine