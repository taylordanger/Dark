#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>

namespace Engine {
    namespace Save {
        
        /**
         * System state serialization helper
         * Provides utilities for systems to easily serialize/deserialize their state
         */
        class SystemStateSerializer {
        public:
            /**
             * Serialize data to JSON string
             * @param data Data to serialize
             * @return JSON string
             */
            template<typename T>
            static std::string serializeToJson(const T& data) {
                try {
                    nlohmann::json json = data;
                    return json.dump();
                } catch (const std::exception& e) {
                    return "";
                }
            }
            
            /**
             * Deserialize data from JSON string
             * @param jsonStr JSON string
             * @param outData Output data
             * @return true if deserialization was successful
             */
            template<typename T>
            static bool deserializeFromJson(const std::string& jsonStr, T& outData) {
                try {
                    nlohmann::json json = nlohmann::json::parse(jsonStr);
                    outData = json.get<T>();
                    return true;
                } catch (const std::exception& e) {
                    return false;
                }
            }
            
            /**
             * Create a simple key-value serializer
             * @param data Map of key-value pairs
             * @return JSON string
             */
            static std::string serializeKeyValueMap(const std::unordered_map<std::string, std::string>& data);
            
            /**
             * Deserialize a simple key-value map
             * @param jsonStr JSON string
             * @param outData Output map
             * @return true if deserialization was successful
             */
            static bool deserializeKeyValueMap(const std::string& jsonStr, std::unordered_map<std::string, std::string>& outData);
            
            /**
             * Serialize numeric data
             * @param data Map of key-numeric value pairs
             * @return JSON string
             */
            static std::string serializeNumericMap(const std::unordered_map<std::string, float>& data);
            
            /**
             * Deserialize numeric data
             * @param jsonStr JSON string
             * @param outData Output map
             * @return true if deserialization was successful
             */
            static bool deserializeNumericMap(const std::string& jsonStr, std::unordered_map<std::string, float>& outData);
            
            /**
             * Serialize boolean flags
             * @param data Map of key-boolean pairs
             * @return JSON string
             */
            static std::string serializeBooleanMap(const std::unordered_map<std::string, bool>& data);
            
            /**
             * Deserialize boolean flags
             * @param jsonStr JSON string
             * @param outData Output map
             * @return true if deserialization was successful
             */
            static bool deserializeBooleanMap(const std::string& jsonStr, std::unordered_map<std::string, bool>& outData);
        };
        
        /**
         * System state registration helper
         * Makes it easy for systems to register their save/load functionality
         */
        class SystemStateRegistry {
        public:
            /**
             * Register system state serializers
             * @param saveIntegration Save integration instance
             * @param systemName System name
             * @param serializer Serialization function
             * @param deserializer Deserialization function
             */
            static void registerSystem(std::shared_ptr<class SaveIntegration> saveIntegration,
                                     const std::string& systemName,
                                     std::function<std::string()> serializer,
                                     std::function<bool(const std::string&)> deserializer);
            
            /**
             * Register simple key-value system state
             * @param saveIntegration Save integration instance
             * @param systemName System name
             * @param dataProvider Function that provides the data map
             * @param dataConsumer Function that consumes the data map
             */
            static void registerKeyValueSystem(std::shared_ptr<class SaveIntegration> saveIntegration,
                                             const std::string& systemName,
                                             std::function<std::unordered_map<std::string, std::string>()> dataProvider,
                                             std::function<bool(const std::unordered_map<std::string, std::string>&)> dataConsumer);
            
            /**
             * Register numeric system state
             * @param saveIntegration Save integration instance
             * @param systemName System name
             * @param dataProvider Function that provides the numeric data map
             * @param dataConsumer Function that consumes the numeric data map
             */
            static void registerNumericSystem(std::shared_ptr<class SaveIntegration> saveIntegration,
                                            const std::string& systemName,
                                            std::function<std::unordered_map<std::string, float>()> dataProvider,
                                            std::function<bool(const std::unordered_map<std::string, float>&)> dataConsumer);
            
            /**
             * Register boolean system state
             * @param saveIntegration Save integration instance
             * @param systemName System name
             * @param dataProvider Function that provides the boolean data map
             * @param dataConsumer Function that consumes the boolean data map
             */
            static void registerBooleanSystem(std::shared_ptr<class SaveIntegration> saveIntegration,
                                            const std::string& systemName,
                                            std::function<std::unordered_map<std::string, bool>()> dataProvider,
                                            std::function<bool(const std::unordered_map<std::string, bool>&)> dataConsumer);
        };
        
    } // namespace Save
} // namespace Engine