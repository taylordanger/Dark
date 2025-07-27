#include "SystemStateSerializer.h"
#include "SaveIntegration.h"
#include <iostream>

namespace Engine {
    namespace Save {
        
        std::string SystemStateSerializer::serializeKeyValueMap(const std::unordered_map<std::string, std::string>& data) {
            try {
                nlohmann::json json = data;
                return json.dump();
            } catch (const std::exception& e) {
                std::cerr << "Failed to serialize key-value map: " << e.what() << std::endl;
                return "";
            }
        }
        
        bool SystemStateSerializer::deserializeKeyValueMap(const std::string& jsonStr, std::unordered_map<std::string, std::string>& outData) {
            try {
                nlohmann::json json = nlohmann::json::parse(jsonStr);
                outData = json.get<std::unordered_map<std::string, std::string>>();
                return true;
            } catch (const std::exception& e) {
                std::cerr << "Failed to deserialize key-value map: " << e.what() << std::endl;
                return false;
            }
        }
        
        std::string SystemStateSerializer::serializeNumericMap(const std::unordered_map<std::string, float>& data) {
            try {
                nlohmann::json json = data;
                return json.dump();
            } catch (const std::exception& e) {
                std::cerr << "Failed to serialize numeric map: " << e.what() << std::endl;
                return "";
            }
        }
        
        bool SystemStateSerializer::deserializeNumericMap(const std::string& jsonStr, std::unordered_map<std::string, float>& outData) {
            try {
                nlohmann::json json = nlohmann::json::parse(jsonStr);
                outData = json.get<std::unordered_map<std::string, float>>();
                return true;
            } catch (const std::exception& e) {
                std::cerr << "Failed to deserialize numeric map: " << e.what() << std::endl;
                return false;
            }
        }
        
        std::string SystemStateSerializer::serializeBooleanMap(const std::unordered_map<std::string, bool>& data) {
            try {
                nlohmann::json json = data;
                return json.dump();
            } catch (const std::exception& e) {
                std::cerr << "Failed to serialize boolean map: " << e.what() << std::endl;
                return "";
            }
        }
        
        bool SystemStateSerializer::deserializeBooleanMap(const std::string& jsonStr, std::unordered_map<std::string, bool>& outData) {
            try {
                nlohmann::json json = nlohmann::json::parse(jsonStr);
                outData = json.get<std::unordered_map<std::string, bool>>();
                return true;
            } catch (const std::exception& e) {
                std::cerr << "Failed to deserialize boolean map: " << e.what() << std::endl;
                return false;
            }
        }
        
        void SystemStateRegistry::registerSystem(std::shared_ptr<SaveIntegration> saveIntegration,
                                                const std::string& systemName,
                                                std::function<std::string()> serializer,
                                                std::function<bool(const std::string&)> deserializer) {
            if (saveIntegration) {
                saveIntegration->registerSystemStateSerializer(systemName, serializer, deserializer);
            }
        }
        
        void SystemStateRegistry::registerKeyValueSystem(std::shared_ptr<SaveIntegration> saveIntegration,
                                                        const std::string& systemName,
                                                        std::function<std::unordered_map<std::string, std::string>()> dataProvider,
                                                        std::function<bool(const std::unordered_map<std::string, std::string>&)> dataConsumer) {
            if (!saveIntegration) {
                return;
            }
            
            auto serializer = [dataProvider]() -> std::string {
                auto data = dataProvider();
                return SystemStateSerializer::serializeKeyValueMap(data);
            };
            
            auto deserializer = [dataConsumer](const std::string& jsonStr) -> bool {
                std::unordered_map<std::string, std::string> data;
                if (SystemStateSerializer::deserializeKeyValueMap(jsonStr, data)) {
                    return dataConsumer(data);
                }
                return false;
            };
            
            saveIntegration->registerSystemStateSerializer(systemName, serializer, deserializer);
        }
        
        void SystemStateRegistry::registerNumericSystem(std::shared_ptr<SaveIntegration> saveIntegration,
                                                       const std::string& systemName,
                                                       std::function<std::unordered_map<std::string, float>()> dataProvider,
                                                       std::function<bool(const std::unordered_map<std::string, float>&)> dataConsumer) {
            if (!saveIntegration) {
                return;
            }
            
            auto serializer = [dataProvider]() -> std::string {
                auto data = dataProvider();
                return SystemStateSerializer::serializeNumericMap(data);
            };
            
            auto deserializer = [dataConsumer](const std::string& jsonStr) -> bool {
                std::unordered_map<std::string, float> data;
                if (SystemStateSerializer::deserializeNumericMap(jsonStr, data)) {
                    return dataConsumer(data);
                }
                return false;
            };
            
            saveIntegration->registerSystemStateSerializer(systemName, serializer, deserializer);
        }
        
        void SystemStateRegistry::registerBooleanSystem(std::shared_ptr<SaveIntegration> saveIntegration,
                                                       const std::string& systemName,
                                                       std::function<std::unordered_map<std::string, bool>()> dataProvider,
                                                       std::function<bool(const std::unordered_map<std::string, bool>&)> dataConsumer) {
            if (!saveIntegration) {
                return;
            }
            
            auto serializer = [dataProvider]() -> std::string {
                auto data = dataProvider();
                return SystemStateSerializer::serializeBooleanMap(data);
            };
            
            auto deserializer = [dataConsumer](const std::string& jsonStr) -> bool {
                std::unordered_map<std::string, bool> data;
                if (SystemStateSerializer::deserializeBooleanMap(jsonStr, data)) {
                    return dataConsumer(data);
                }
                return false;
            };
            
            saveIntegration->registerSystemStateSerializer(systemName, serializer, deserializer);
        }
        
    } // namespace Save
} // namespace Engine