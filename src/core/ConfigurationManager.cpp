#include "ConfigurationManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

namespace RPGEngine {
    namespace Core {

        // ConfigValue implementation
        ConfigValue::ConfigValue(const nlohmann::json& json) {
            if (json.is_null()) {
                type_ = Type::Null;
            } else if (json.is_boolean()) {
                type_ = Type::Boolean;
                data_ = json.get<bool>();
            } else if (json.is_number_integer()) {
                type_ = Type::Integer;
                data_ = json.get<int>();
            } else if (json.is_number_float()) {
                type_ = Type::Float;
                data_ = json.get<float>();
            } else if (json.is_string()) {
                type_ = Type::String;
                data_ = json.get<std::string>();
            } else if (json.is_array()) {
                type_ = Type::Array;
                data_ = json;
            } else if (json.is_object()) {
                type_ = Type::Object;
                data_ = json;
            } else {
                type_ = Type::Null;
            }
        }

        bool ConfigValue::asBool(bool defaultValue) const {
            if (type_ == Type::Boolean) {
                return std::any_cast<bool>(data_);
            }
            return defaultValue;
        }

        int ConfigValue::asInt(int defaultValue) const {
            if (type_ == Type::Integer) {
                return std::any_cast<int>(data_);
            } else if (type_ == Type::Float) {
                return static_cast<int>(std::any_cast<float>(data_));
            }
            return defaultValue;
        }

        float ConfigValue::asFloat(float defaultValue) const {
            if (type_ == Type::Float) {
                return std::any_cast<float>(data_);
            } else if (type_ == Type::Integer) {
                return static_cast<float>(std::any_cast<int>(data_));
            }
            return defaultValue;
        }

        std::string ConfigValue::asString(const std::string& defaultValue) const {
            if (type_ == Type::String) {
                return std::any_cast<std::string>(data_);
            }
            return defaultValue;
        }

        size_t ConfigValue::size() const {
            if (type_ == Type::Array) {
                return std::any_cast<nlohmann::json>(data_).size();
            }
            return 0;
        }

        ConfigValue ConfigValue::operator[](size_t index) const {
            if (type_ == Type::Array) {
                const auto& jsonArray = std::any_cast<nlohmann::json>(data_);
                if (index < jsonArray.size()) {
                    return ConfigValue(jsonArray[index]);
                }
            }
            return ConfigValue();
        }

        ConfigValue ConfigValue::operator[](const std::string& key) const {
            if (type_ == Type::Object) {
                const auto& jsonObject = std::any_cast<nlohmann::json>(data_);
                if (jsonObject.contains(key)) {
                    return ConfigValue(jsonObject[key]);
                }
            }
            return ConfigValue();
        }

        bool ConfigValue::hasKey(const std::string& key) const {
            if (type_ == Type::Object) {
                const auto& jsonObject = std::any_cast<nlohmann::json>(data_);
                return jsonObject.contains(key);
            }
            return false;
        }

        std::vector<std::string> ConfigValue::getKeys() const {
            std::vector<std::string> keys;
            if (type_ == Type::Object) {
                const auto& jsonObject = std::any_cast<nlohmann::json>(data_);
                for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
                    keys.push_back(it.key());
                }
            }
            return keys;
        }

        nlohmann::json ConfigValue::toJson() const {
            switch (type_) {
                case Type::Null:
                    return nlohmann::json();
                case Type::Boolean:
                    return std::any_cast<bool>(data_);
                case Type::Integer:
                    return std::any_cast<int>(data_);
                case Type::Float:
                    return std::any_cast<float>(data_);
                case Type::String:
                    return std::any_cast<std::string>(data_);
                case Type::Array:
                case Type::Object:
                    return std::any_cast<nlohmann::json>(data_);
            }
            return nlohmann::json();
        }

        // ConfigurationManager implementation
        ConfigurationManager::ConfigurationManager() : hotReloadingEnabled_(false) {
        }

        ConfigurationManager::~ConfigurationManager() {
        }

        bool ConfigurationManager::loadFromFile(const std::string& filePath) {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "Failed to open configuration file: " << filePath << std::endl;
                return false;
            }

            try {
                file >> config_;
                fileTimestamps_[filePath] = getFileModificationTime(filePath);
                return true;
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "JSON parsing error in " << filePath << ": " << e.what() << std::endl;
                return false;
            }
        }

        bool ConfigurationManager::loadFromString(const std::string& jsonString) {
            try {
                config_ = nlohmann::json::parse(jsonString);
                return true;
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "JSON parsing error: " << e.what() << std::endl;
                return false;
            }
        }

        bool ConfigurationManager::saveToFile(const std::string& filePath) const {
            std::ofstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "Failed to open file for writing: " << filePath << std::endl;
                return false;
            }

            file << config_.dump(4);
            return true;
        }

        ConfigValue ConfigurationManager::get(const std::string& path) const {
            return getValueAtPath(config_, path);
        }

        void ConfigurationManager::set(const std::string& path, const ConfigValue& value) {
            ConfigValue oldValue = get(path);
            setValueAtPath(config_, path, value);
            
            // Notify callbacks if value changed
            if (oldValue.toJson() != value.toJson()) {
                notifyCallbacks(path, value);
            }
        }

        bool ConfigurationManager::has(const std::string& path) const {
            return hasValueAtPath(config_, path);
        }

        void ConfigurationManager::remove(const std::string& path) {
            removeValueAtPath(config_, path);
        }

        bool ConfigurationManager::getBool(const std::string& path, bool defaultValue) const {
            return get(path).asBool(defaultValue);
        }

        int ConfigurationManager::getInt(const std::string& path, int defaultValue) const {
            return get(path).asInt(defaultValue);
        }

        float ConfigurationManager::getFloat(const std::string& path, float defaultValue) const {
            return get(path).asFloat(defaultValue);
        }

        std::string ConfigurationManager::getString(const std::string& path, const std::string& defaultValue) const {
            return get(path).asString(defaultValue);
        }

        void ConfigurationManager::checkForChanges() {
            if (!hotReloadingEnabled_) {
                return;
            }

            for (auto& pair : fileTimestamps_) {
                const std::string& filePath = pair.first;
                std::time_t& lastModified = pair.second;
                
                std::time_t currentModified = getFileModificationTime(filePath);
                if (currentModified > lastModified) {
                    std::cout << "Configuration file changed, reloading: " << filePath << std::endl;
                    
                    // Store old config for comparison
                    nlohmann::json oldConfig = config_;
                    
                    // Reload file
                    if (loadFromFile(filePath)) {
                        lastModified = currentModified;
                        
                        // TODO: Compare configs and notify specific path changes
                        // For now, just notify that the root changed
                        notifyCallbacks("", ConfigValue(config_));
                    }
                }
            }
        }

        void ConfigurationManager::addChangeCallback(const std::string& path, ConfigChangeCallback callback) {
            changeCallbacks_[path].push_back(callback);
        }

        void ConfigurationManager::removeChangeCallback(const std::string& path) {
            changeCallbacks_.erase(path);
        }

        void ConfigurationManager::merge(const ConfigurationManager& other) {
            // Merge JSON objects
            config_.update(other.config_);
        }

        void ConfigurationManager::mergeFromFile(const std::string& filePath) {
            ConfigurationManager temp;
            if (temp.loadFromFile(filePath)) {
                merge(temp);
            }
        }

        void ConfigurationManager::clear() {
            config_.clear();
            fileTimestamps_.clear();
            changeCallbacks_.clear();
        }

        ConfigValue ConfigurationManager::getValueAtPath(const nlohmann::json& json, const std::string& path) const {
            if (path.empty()) {
                return ConfigValue(json);
            }

            std::vector<std::string> pathParts = splitPath(path);
            const nlohmann::json* current = &json;

            for (const std::string& part : pathParts) {
                if (current->is_object() && current->contains(part)) {
                    current = &(*current)[part];
                } else {
                    return ConfigValue(); // Return null value
                }
            }

            return ConfigValue(*current);
        }

        void ConfigurationManager::setValueAtPath(nlohmann::json& json, const std::string& path, const ConfigValue& value) {
            if (path.empty()) {
                json = value.toJson();
                return;
            }

            std::vector<std::string> pathParts = splitPath(path);
            nlohmann::json* current = &json;

            // Navigate to the parent of the target
            for (size_t i = 0; i < pathParts.size() - 1; ++i) {
                const std::string& part = pathParts[i];
                if (!current->is_object()) {
                    *current = nlohmann::json::object();
                }
                current = &(*current)[part];
            }

            // Set the final value
            if (!current->is_object()) {
                *current = nlohmann::json::object();
            }
            (*current)[pathParts.back()] = value.toJson();
        }

        bool ConfigurationManager::hasValueAtPath(const nlohmann::json& json, const std::string& path) const {
            if (path.empty()) {
                return true;
            }

            std::vector<std::string> pathParts = splitPath(path);
            const nlohmann::json* current = &json;

            for (const std::string& part : pathParts) {
                if (current->is_object() && current->contains(part)) {
                    current = &(*current)[part];
                } else {
                    return false;
                }
            }

            return true;
        }

        void ConfigurationManager::removeValueAtPath(nlohmann::json& json, const std::string& path) {
            if (path.empty()) {
                json.clear();
                return;
            }

            std::vector<std::string> pathParts = splitPath(path);
            nlohmann::json* current = &json;

            // Navigate to the parent of the target
            for (size_t i = 0; i < pathParts.size() - 1; ++i) {
                const std::string& part = pathParts[i];
                if (current->is_object() && current->contains(part)) {
                    current = &(*current)[part];
                } else {
                    return; // Path doesn't exist
                }
            }

            // Remove the final key
            if (current->is_object()) {
                current->erase(pathParts.back());
            }
        }

        std::vector<std::string> ConfigurationManager::splitPath(const std::string& path) const {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string part;

            while (std::getline(ss, part, '.')) {
                if (!part.empty()) {
                    parts.push_back(part);
                }
            }

            return parts;
        }

        std::time_t ConfigurationManager::getFileModificationTime(const std::string& filename) const {
            struct stat fileInfo;
            if (stat(filename.c_str(), &fileInfo) == 0) {
                return fileInfo.st_mtime;
            }
            return 0;
        }

        void ConfigurationManager::notifyCallbacks(const std::string& path, const ConfigValue& newValue) {
            auto it = changeCallbacks_.find(path);
            if (it != changeCallbacks_.end()) {
                for (const auto& callback : it->second) {
                    callback(path, newValue);
                }
            }
        }

    } // namespace Core
} // namespace RPGEngine