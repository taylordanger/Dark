#include "JsonSaveSerializer.h"
#include <stdexcept>

namespace Engine {
    namespace Save {
        
        std::string JsonSaveSerializer::serialize(const SaveData& data) {
            try {
                nlohmann::json json;
                
                // Basic save data
                json["version"] = data.version;
                json["timestamp"] = data.timestamp;
                
                // Player data
                json["player"] = serializePlayerData(data.player);
                
                // World data
                json["world"] = serializeWorldData(data.world);
                
                // Custom data
                if (!data.customData.empty()) {
                    json["customData"] = data.customData;
                }
                
                return json.dump(4); // Pretty print with 4 spaces
                
            } catch (const std::exception& e) {
                return ""; // Return empty string on error
            }
        }
        
        bool JsonSaveSerializer::deserialize(const std::string& jsonString, SaveData& outData) {
            try {
                nlohmann::json json = nlohmann::json::parse(jsonString);
                
                // Validate JSON structure
                if (!validateJsonStructure(json)) {
                    return false;
                }
                
                // Basic save data
                outData.version = json.value("version", "1.0");
                outData.timestamp = json.value("timestamp", "");
                
                // Player data
                if (json.contains("player")) {
                    if (!deserializePlayerData(json["player"], outData.player)) {
                        return false;
                    }
                }
                
                // World data
                if (json.contains("world")) {
                    if (!deserializeWorldData(json["world"], outData.world)) {
                        return false;
                    }
                }
                
                // Custom data
                if (json.contains("customData") && json["customData"].is_object()) {
                    outData.customData = json["customData"];
                }
                
                return true;
                
            } catch (const std::exception& e) {
                return false;
            }
        }
        
        nlohmann::json JsonSaveSerializer::serializePlayerData(const PlayerData& player) {
            nlohmann::json json;
            
            // Position
            json["position"]["x"] = player.position.x;
            json["position"]["y"] = player.position.y;
            
            // Stats
            json["stats"]["hp"] = player.stats.hp;
            json["stats"]["maxHp"] = player.stats.maxHp;
            json["stats"]["mp"] = player.stats.mp;
            json["stats"]["maxMp"] = player.stats.maxMp;
            json["stats"]["level"] = player.stats.level;
            json["stats"]["experience"] = player.stats.experience;
            json["stats"]["experienceToNext"] = player.stats.experienceToNext;
            
            // Inventory
            json["inventory"] = nlohmann::json::array();
            for (const auto& item : player.inventory) {
                nlohmann::json itemJson;
                itemJson["id"] = item.id;
                itemJson["quantity"] = item.quantity;
                json["inventory"].push_back(itemJson);
            }
            
            // Equipment
            if (!player.equipment.empty()) {
                json["equipment"] = player.equipment;
            }
            
            return json;
        }
        
        nlohmann::json JsonSaveSerializer::serializeWorldData(const WorldData& world) {
            nlohmann::json json;
            
            json["currentMap"] = world.currentMap;
            json["completedQuests"] = world.completedQuests;
            
            if (!world.gameFlags.empty()) {
                json["gameFlags"] = world.gameFlags;
            }
            
            if (!world.npcStates.empty()) {
                json["npcStates"] = world.npcStates;
            }
            
            if (!world.discoveredLocations.empty()) {
                json["discoveredLocations"] = world.discoveredLocations;
            }
            
            return json;
        }
        
        bool JsonSaveSerializer::deserializePlayerData(const nlohmann::json& json, PlayerData& outPlayer) {
            try {
                // Position
                if (json.contains("position")) {
                    outPlayer.position.x = json["position"].value("x", 0.0f);
                    outPlayer.position.y = json["position"].value("y", 0.0f);
                }
                
                // Stats
                if (json.contains("stats")) {
                    const auto& stats = json["stats"];
                    outPlayer.stats.hp = stats.value("hp", 100);
                    outPlayer.stats.maxHp = stats.value("maxHp", 100);
                    outPlayer.stats.mp = stats.value("mp", 50);
                    outPlayer.stats.maxMp = stats.value("maxMp", 50);
                    outPlayer.stats.level = stats.value("level", 1);
                    outPlayer.stats.experience = stats.value("experience", 0);
                    outPlayer.stats.experienceToNext = stats.value("experienceToNext", 100);
                }
                
                // Inventory
                if (json.contains("inventory") && json["inventory"].is_array()) {
                    outPlayer.inventory.clear();
                    for (const auto& itemJson : json["inventory"]) {
                        if (itemJson.contains("id") && itemJson.contains("quantity")) {
                            PlayerData::InventoryItem item;
                            item.id = itemJson["id"];
                            item.quantity = itemJson["quantity"];
                            outPlayer.inventory.push_back(item);
                        }
                    }
                }
                
                // Equipment
                if (json.contains("equipment") && json["equipment"].is_object()) {
                    outPlayer.equipment = json["equipment"];
                }
                
                return true;
                
            } catch (const std::exception& e) {
                return false;
            }
        }
        
        bool JsonSaveSerializer::deserializeWorldData(const nlohmann::json& json, WorldData& outWorld) {
            try {
                outWorld.currentMap = json.value("currentMap", "");
                
                // Completed quests
                if (json.contains("completedQuests") && json["completedQuests"].is_array()) {
                    outWorld.completedQuests.clear();
                    for (const auto& quest : json["completedQuests"]) {
                        if (quest.is_string()) {
                            outWorld.completedQuests.push_back(quest);
                        }
                    }
                }
                
                // Game flags
                if (json.contains("gameFlags") && json["gameFlags"].is_object()) {
                    outWorld.gameFlags = json["gameFlags"];
                }
                
                // NPC states
                if (json.contains("npcStates") && json["npcStates"].is_object()) {
                    outWorld.npcStates = json["npcStates"];
                }
                
                // Discovered locations
                if (json.contains("discoveredLocations") && json["discoveredLocations"].is_object()) {
                    outWorld.discoveredLocations = json["discoveredLocations"];
                }
                
                return true;
                
            } catch (const std::exception& e) {
                return false;
            }
        }
        
        bool JsonSaveSerializer::validateJsonStructure(const nlohmann::json& json) {
            // Check required fields
            if (!json.contains("version") || !json["version"].is_string()) {
                return false;
            }
            
            if (!json.contains("timestamp") || !json["timestamp"].is_string()) {
                return false;
            }
            
            // Check player section
            if (json.contains("player")) {
                const auto& player = json["player"];
                
                if (player.contains("position")) {
                    const auto& pos = player["position"];
                    if (!pos.contains("x") || !pos.contains("y") || 
                        !pos["x"].is_number() || !pos["y"].is_number()) {
                        return false;
                    }
                }
                
                if (player.contains("stats")) {
                    const auto& stats = player["stats"];
                    if (stats.contains("level") && (!stats["level"].is_number_integer() || stats["level"] < 1)) {
                        return false;
                    }
                    if (stats.contains("hp") && !stats["hp"].is_number_integer()) {
                        return false;
                    }
                    if (stats.contains("mp") && !stats["mp"].is_number_integer()) {
                        return false;
                    }
                }
                
                if (player.contains("inventory") && !player["inventory"].is_array()) {
                    return false;
                }
            }
            
            // Check world section
            if (json.contains("world")) {
                const auto& world = json["world"];
                
                if (world.contains("currentMap") && !world["currentMap"].is_string()) {
                    return false;
                }
                
                if (world.contains("completedQuests") && !world["completedQuests"].is_array()) {
                    return false;
                }
            }
            
            return true;
        }
        
    } // namespace Save
} // namespace Engine