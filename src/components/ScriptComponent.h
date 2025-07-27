#pragma once

#include "Component.h"
#include "../scripting/IScriptEngine.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

namespace RPGEngine {
    namespace Components {

        /**
         * Component that allows entities to have script-based behaviors
         */
        class ScriptComponent {
        public:
            ScriptComponent(EntityId entityId);
            ~ScriptComponent() = default;

            // Script management
            void setScript(const std::string& scriptContent);
            void setScriptFile(const std::string& scriptFilePath);
            const std::string& getScript() const { return scriptContent; }
            const std::string& getScriptFile() const { return scriptFilePath; }

            // Script execution
            bool executeScript();
            bool callScriptFunction(const std::string& functionName, const std::vector<Scripting::ScriptValue>& args = {});
            Scripting::ScriptValue getScriptVariable(const std::string& variableName);
            void setScriptVariable(const std::string& variableName, const Scripting::ScriptValue& value);

            // Event handling
            void registerScriptEvent(const std::string& eventName, const std::string& functionName);
            void triggerScriptEvent(const std::string& eventName, const std::vector<Scripting::ScriptValue>& args = {});

            // Script engine management
            void setScriptEngine(std::shared_ptr<Scripting::IScriptEngine> engine);
            std::shared_ptr<Scripting::IScriptEngine> getScriptEngine() const { return scriptEngine; }

            // Context management
            std::shared_ptr<Scripting::ScriptContext> getScriptContext() const { return scriptContext; }

            // Component lifecycle callbacks
            void onInitialize();
            void onUpdate(float deltaTime);
            void onDestroy();

            // Serialization
            nlohmann::json serialize() const;
            void deserialize(const nlohmann::json& json);

        private:
            EntityId entityId;
            std::string scriptContent;
            std::string scriptFilePath;
            std::shared_ptr<Scripting::IScriptEngine> scriptEngine;
            std::shared_ptr<Scripting::ScriptContext> scriptContext;
            std::unordered_map<std::string, std::string> eventHandlers; // event name -> function name
            bool isInitialized = false;

            void initializeScriptContext();
            void setupEntityAPI();
        };

    } // namespace Components
} // namespace RPGEngine