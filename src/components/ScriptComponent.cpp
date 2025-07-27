#include "ScriptComponent.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace RPGEngine {
    namespace Components {

        ScriptComponent::ScriptComponent(EntityId entityId) : entityId(entityId) {
        }

        void ScriptComponent::setScript(const std::string& scriptContent) {
            this->scriptContent = scriptContent;
            this->scriptFilePath.clear();
            isInitialized = false;
        }

        void ScriptComponent::setScriptFile(const std::string& scriptFilePath) {
            this->scriptFilePath = scriptFilePath;
            
            // Load script content from file
            std::ifstream file(scriptFilePath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                this->scriptContent = buffer.str();
                file.close();
                isInitialized = false;
            } else {
                std::cerr << "Failed to load script file: " << scriptFilePath << std::endl;
                this->scriptContent.clear();
            }
        }

        bool ScriptComponent::executeScript() {
            if (!scriptEngine || scriptContent.empty()) {
                return false;
            }

            if (!isInitialized) {
                initializeScriptContext();
            }

            if (scriptContext) {
                return scriptContext->executeInContext(scriptContent);
            } else {
                return scriptEngine->executeScript(scriptContent);
            }
        }

        bool ScriptComponent::callScriptFunction(const std::string& functionName, const std::vector<Scripting::ScriptValue>& args) {
            if (!scriptEngine) {
                return false;
            }

            if (!isInitialized) {
                executeScript(); // Initialize if not already done
            }

            Scripting::ScriptValue result = scriptEngine->callFunction(functionName, args);
            return !scriptEngine->hasError();
        }

        Scripting::ScriptValue ScriptComponent::getScriptVariable(const std::string& variableName) {
            if (!scriptContext) {
                return Scripting::ScriptValue();
            }

            return scriptContext->getGlobal(variableName);
        }

        void ScriptComponent::setScriptVariable(const std::string& variableName, const Scripting::ScriptValue& value) {
            if (!scriptContext) {
                return;
            }

            scriptContext->setGlobal(variableName, value);
        }

        void ScriptComponent::registerScriptEvent(const std::string& eventName, const std::string& functionName) {
            eventHandlers[eventName] = functionName;
        }

        void ScriptComponent::triggerScriptEvent(const std::string& eventName, const std::vector<Scripting::ScriptValue>& args) {
            auto it = eventHandlers.find(eventName);
            if (it != eventHandlers.end()) {
                callScriptFunction(it->second, args);
            }
        }

        void ScriptComponent::setScriptEngine(std::shared_ptr<Scripting::IScriptEngine> engine) {
            this->scriptEngine = engine;
            isInitialized = false;
        }

        void ScriptComponent::onInitialize() {
            if (!scriptContent.empty()) {
                executeScript();
                callScriptFunction("onInitialize");
            }
        }

        void ScriptComponent::onUpdate(float deltaTime) {
            if (isInitialized) {
                std::vector<Scripting::ScriptValue> args = { Scripting::ScriptValue(static_cast<double>(deltaTime)) };
                callScriptFunction("onUpdate", args);
            }
        }

        void ScriptComponent::onDestroy() {
            if (isInitialized) {
                callScriptFunction("onDestroy");
            }
        }

        void ScriptComponent::initializeScriptContext() {
            if (!scriptEngine) {
                return;
            }

            scriptContext = scriptEngine->createContext();
            if (scriptContext) {
                setupEntityAPI();
                isInitialized = true;
            }
        }

        void ScriptComponent::setupEntityAPI() {
            if (!scriptContext) {
                return;
            }

            // Set entity ID as a global variable
            scriptContext->setGlobal("entityId", Scripting::ScriptValue(static_cast<double>(entityId)));

            // Register entity-specific functions through the script engine
            if (scriptEngine) {
                // Register entity API functions
                scriptEngine->registerFunction("getEntityId", [this](const std::vector<Scripting::ScriptValue>&) -> Scripting::ScriptValue {
                    return Scripting::ScriptValue(static_cast<double>(this->entityId));
                });

                scriptEngine->registerFunction("log", [](const std::vector<Scripting::ScriptValue>& args) -> Scripting::ScriptValue {
                    std::cout << "[Script] ";
                    for (size_t i = 0; i < args.size(); ++i) {
                        if (i > 0) std::cout << " ";
                        if (args[i].isString()) {
                            std::cout << args[i].get<std::string>();
                        } else if (args[i].isNumber()) {
                            std::cout << args[i].get<double>();
                        } else if (args[i].isBool()) {
                            std::cout << (args[i].get<bool>() ? "true" : "false");
                        } else {
                            std::cout << "nil";
                        }
                    }
                    std::cout << std::endl;
                    return Scripting::ScriptValue();
                });
            }
        }

        nlohmann::json ScriptComponent::serialize() const {
            nlohmann::json json;
            json["type"] = "ScriptComponent";
            json["entityId"] = entityId;
            json["scriptContent"] = scriptContent;
            json["scriptFilePath"] = scriptFilePath;
            
            // Serialize event handlers
            nlohmann::json eventHandlersJson;
            for (const auto& pair : eventHandlers) {
                eventHandlersJson[pair.first] = pair.second;
            }
            json["eventHandlers"] = eventHandlersJson;
            
            return json;
        }

        void ScriptComponent::deserialize(const nlohmann::json& json) {
            if (json.contains("entityId")) {
                entityId = json["entityId"];
            }
            
            if (json.contains("scriptContent")) {
                scriptContent = json["scriptContent"];
            }
            
            if (json.contains("scriptFilePath")) {
                scriptFilePath = json["scriptFilePath"];
                if (!scriptFilePath.empty()) {
                    setScriptFile(scriptFilePath); // This will load the script content
                }
            }
            
            if (json.contains("eventHandlers")) {
                eventHandlers.clear();
                for (const auto& item : json["eventHandlers"].items()) {
                    eventHandlers[item.key()] = item.value();
                }
            }
            
            isInitialized = false;
        }

    } // namespace Components
} // namespace RPGEngine