#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <nlohmann/json.hpp>

namespace RPGEngine {
    namespace Core {

        /**
         * Configuration value that can hold different types
         */
        class ConfigValue {
        public:
            enum Type {
                Null,
                Boolean,
                Integer,
                Float,
                String,
                Array,
                Object
            };

            ConfigValue() : type_(Type::Null) {}
            ConfigValue(bool value) : type_(Type::Boolean), data_(value) {}
            ConfigValue(int value) : type_(Type::Integer), data_(value) {}
            ConfigValue(float value) : type_(Type::Float), data_(value) {}
            ConfigValue(double value) : type_(Type::Float), data_(static_cast<float>(value)) {}
            ConfigValue(const std::string& value) : type_(Type::String), data_(value) {}
            ConfigValue(const char* value) : type_(Type::String), data_(std::string(value)) {}
            ConfigValue(const nlohmann::json& json);

            Type getType() const { return type_; }
            bool isNull() const { return type_ == Type::Null; }
            bool isBool() const { return type_ == Type::Boolean; }
            bool isInt() const { return type_ == Type::Integer; }
            bool isFloat() const { return type_ == Type::Float; }
            bool isString() const { return type_ == Type::String; }
            bool isArray() const { return type_ == Type::Array; }
            bool isObject() const { return type_ == Type::Object; }

            bool asBool(bool defaultValue = false) const;
            int asInt(int defaultValue = 0) const;
            float asFloat(float defaultValue = 0.0f) const;
            std::string asString(const std::string& defaultValue = "") const;
            
            // Array access
            size_t size() const;
            ConfigValue operator[](size_t index) const;
            
            // Object access
            ConfigValue operator[](const std::string& key) const;
            bool hasKey(const std::string& key) const;
            std::vector<std::string> getKeys() const;

            // Conversion to JSON
            nlohmann::json toJson() const;

        private:
            Type type_;
            std::any data_;
        };

        /**
         * Configuration change callback
         */
        using ConfigChangeCallback = std::function<void(const std::string& path, const ConfigValue& newValue)>;

        /**
         * Manages configuration loading, watching, and hot-reloading
         */
        class ConfigurationManager {
        public:
            ConfigurationManager();
            ~ConfigurationManager();

            // Configuration loading
            bool loadFromFile(const std::string& filePath);
            bool loadFromString(const std::string& jsonString);
            bool saveToFile(const std::string& filePath) const;

            // Configuration access
            ConfigValue get(const std::string& path) const;
            void set(const std::string& path, const ConfigValue& value);
            bool has(const std::string& path) const;
            void remove(const std::string& path);

            // Convenience getters with defaults
            bool getBool(const std::string& path, bool defaultValue = false) const;
            int getInt(const std::string& path, int defaultValue = 0) const;
            float getFloat(const std::string& path, float defaultValue = 0.0f) const;
            std::string getString(const std::string& path, const std::string& defaultValue = "") const;

            // Hot reloading
            void enableHotReloading(bool enable) { hotReloadingEnabled_ = enable; }
            bool isHotReloadingEnabled() const { return hotReloadingEnabled_; }
            void checkForChanges();

            // Change notifications
            void addChangeCallback(const std::string& path, ConfigChangeCallback callback);
            void removeChangeCallback(const std::string& path);

            // Merge configurations
            void merge(const ConfigurationManager& other);
            void mergeFromFile(const std::string& filePath);

            // Get all configuration as JSON
            nlohmann::json getAll() const { return config_; }

            // Clear all configuration
            void clear();

        private:
            nlohmann::json config_;
            std::unordered_map<std::string, std::time_t> fileTimestamps_;
            std::unordered_map<std::string, std::vector<ConfigChangeCallback>> changeCallbacks_;
            bool hotReloadingEnabled_;

            // Helper methods
            ConfigValue getValueAtPath(const nlohmann::json& json, const std::string& path) const;
            void setValueAtPath(nlohmann::json& json, const std::string& path, const ConfigValue& value);
            bool hasValueAtPath(const nlohmann::json& json, const std::string& path) const;
            void removeValueAtPath(nlohmann::json& json, const std::string& path);
            std::vector<std::string> splitPath(const std::string& path) const;
            std::time_t getFileModificationTime(const std::string& filename) const;
            void notifyCallbacks(const std::string& path, const ConfigValue& newValue);
        };

    } // namespace Core
} // namespace RPGEngine