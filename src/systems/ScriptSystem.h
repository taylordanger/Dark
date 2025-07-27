#pragma once

#include "System.h"
#include "../scripting/IScriptEngine.h"
#include "../components/ScriptComponent.h"
#include "../components/Component.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <ctime>

namespace RPGEngine {
    namespace Systems {

        /**
         * System that manages script components and handles script execution
         */
        class ScriptSystem : public System {
        public:
            ScriptSystem();
            ~ScriptSystem() override = default;

            // System lifecycle
            bool initialize() override;
            void update(float deltaTime) override;
            void shutdown() override;

            // Script engine management
            void setScriptEngine(std::shared_ptr<Scripting::IScriptEngine> engine);
            std::shared_ptr<Scripting::IScriptEngine> getScriptEngine() const { return scriptEngine; }

            // Global script functions
            void registerGlobalFunction(const std::string& name, Scripting::ScriptCallback callback);
            void registerGlobalVariable(const std::string& name, const Scripting::ScriptValue& value);

            // Event handling
            void registerEventHandler(const std::string& eventType, Scripting::ScriptEventCallback callback);
            void triggerGlobalEvent(const std::string& eventType, const std::vector<Scripting::ScriptValue>& args = {});

            // Script execution
            bool executeGlobalScript(const std::string& script);
            bool executeScriptFile(const std::string& filename);
            Scripting::ScriptValue callGlobalFunction(const std::string& functionName, const std::vector<Scripting::ScriptValue>& args = {});

            // Component management
            void onComponentAdded(Components::EntityId entityId, std::shared_ptr<Components::ScriptComponent> component);
            void onComponentRemoved(Components::EntityId entityId);

            // Hot reloading support
            void enableHotReloading(bool enable) { hotReloadingEnabled = enable; }
            void checkForScriptChanges();

        private:
            std::shared_ptr<Scripting::IScriptEngine> scriptEngine;
            std::unordered_map<Components::EntityId, std::shared_ptr<Components::ScriptComponent>> scriptComponents;
            std::unordered_map<std::string, std::time_t> scriptFileTimestamps; // For hot reloading
            bool hotReloadingEnabled = false;

            void setupEngineAPI();
            void updateScriptComponents(float deltaTime);
            std::time_t getFileModificationTime(const std::string& filename);
        };

    } // namespace Systems
} // namespace RPGEngine