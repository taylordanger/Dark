#include "ScriptSystem.h"
#include "../scripting/LuaScriptEngine.h"
#include <iostream>
#include <sys/stat.h>

namespace RPGEngine {
    namespace Systems {

        ScriptSystem::ScriptSystem() : System("ScriptSystem"), hotReloadingEnabled(false) {
        }

        bool ScriptSystem::initialize() {
            // Create default Lua script engine if none provided
            if (!scriptEngine) {
                scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
                if (!scriptEngine->initialize()) {
                    std::cerr << "Failed to initialize script engine: " << scriptEngine->getLastError() << std::endl;
                    scriptEngine.reset();
                    return false;
                }
            }

            setupEngineAPI();
            std::cout << "ScriptSystem initialized" << std::endl;
            return true;
        }

        void ScriptSystem::update(float deltaTime) {
            if (!scriptEngine) {
                return;
            }

            // Check for script file changes if hot reloading is enabled
            if (hotReloadingEnabled) {
                checkForScriptChanges();
            }

            // Update all script components
            updateScriptComponents(deltaTime);
        }

        void ScriptSystem::shutdown() {
            scriptComponents.clear();
            if (scriptEngine) {
                scriptEngine->shutdown();
                scriptEngine.reset();
            }
            std::cout << "ScriptSystem shutdown" << std::endl;
        }

        void ScriptSystem::setScriptEngine(std::shared_ptr<Scripting::IScriptEngine> engine) {
            this->scriptEngine = engine;
            
            // Update all existing script components with the new engine
            for (auto& pair : scriptComponents) {
                pair.second->setScriptEngine(engine);
            }
        }

        void ScriptSystem::registerGlobalFunction(const std::string& name, Scripting::ScriptCallback callback) {
            if (scriptEngine) {
                scriptEngine->registerFunction(name, callback);
            }
        }

        void ScriptSystem::registerGlobalVariable(const std::string& name, const Scripting::ScriptValue& value) {
            if (scriptEngine) {
                scriptEngine->registerGlobal(name, value);
            }
        }

        void ScriptSystem::registerEventHandler(const std::string& eventType, Scripting::ScriptEventCallback callback) {
            if (scriptEngine) {
                scriptEngine->registerEventCallback(eventType, callback);
            }
        }

        void ScriptSystem::triggerGlobalEvent(const std::string& eventType, const std::vector<Scripting::ScriptValue>& args) {
            if (scriptEngine) {
                scriptEngine->triggerEvent(eventType, args);
            }

            // Also trigger event on all script components
            for (auto& pair : scriptComponents) {
                pair.second->triggerScriptEvent(eventType, args);
            }
        }

        bool ScriptSystem::executeGlobalScript(const std::string& script) {
            if (!scriptEngine) {
                return false;
            }

            bool result = scriptEngine->executeScript(script);
            if (!result) {
                std::cerr << "Script execution error: " << scriptEngine->getLastError() << std::endl;
            }
            return result;
        }

        bool ScriptSystem::executeScriptFile(const std::string& filename) {
            if (!scriptEngine) {
                return false;
            }

            bool result = scriptEngine->executeFile(filename);
            if (!result) {
                std::cerr << "Script file execution error: " << scriptEngine->getLastError() << std::endl;
            } else {
                // Track file for hot reloading
                scriptFileTimestamps[filename] = getFileModificationTime(filename);
            }
            return result;
        }

        Scripting::ScriptValue ScriptSystem::callGlobalFunction(const std::string& functionName, const std::vector<Scripting::ScriptValue>& args) {
            if (!scriptEngine) {
                return Scripting::ScriptValue();
            }

            Scripting::ScriptValue result = scriptEngine->callFunction(functionName, args);
            if (scriptEngine->hasError()) {
                std::cerr << "Script function call error: " << scriptEngine->getLastError() << std::endl;
            }
            return result;
        }

        void ScriptSystem::onComponentAdded(Components::EntityId entityId, std::shared_ptr<Components::ScriptComponent> component) {
            scriptComponents[entityId] = component;
            
            // Set the script engine for the component
            if (scriptEngine) {
                component->setScriptEngine(scriptEngine);
                component->onInitialize();
            }
        }

        void ScriptSystem::onComponentRemoved(Components::EntityId entityId) {
            auto it = scriptComponents.find(entityId);
            if (it != scriptComponents.end()) {
                it->second->onDestroy();
                scriptComponents.erase(it);
            }
        }

        void ScriptSystem::checkForScriptChanges() {
            for (auto& pair : scriptFileTimestamps) {
                const std::string& filename = pair.first;
                std::time_t& lastModified = pair.second;
                
                std::time_t currentModified = getFileModificationTime(filename);
                if (currentModified > lastModified) {
                    std::cout << "Script file changed, reloading: " << filename << std::endl;
                    executeScriptFile(filename);
                    lastModified = currentModified;
                }
            }
        }

        void ScriptSystem::setupEngineAPI() {
            if (!scriptEngine) {
                return;
            }

            // Register engine-specific functions
            registerGlobalFunction("getCurrentTime", [](const std::vector<Scripting::ScriptValue>&) -> Scripting::ScriptValue {
                return Scripting::ScriptValue(static_cast<double>(std::time(nullptr)));
            });

            registerGlobalFunction("triggerEvent", [this](const std::vector<Scripting::ScriptValue>& args) -> Scripting::ScriptValue {
                if (args.size() >= 1 && args[0].isString()) {
                    std::string eventType = args[0].get<std::string>();
                    std::vector<Scripting::ScriptValue> eventArgs(args.begin() + 1, args.end());
                    this->triggerGlobalEvent(eventType, eventArgs);
                }
                return Scripting::ScriptValue();
            });

            // Register math functions
            registerGlobalFunction("random", [](const std::vector<Scripting::ScriptValue>& args) -> Scripting::ScriptValue {
                if (args.size() == 0) {
                    return Scripting::ScriptValue(static_cast<double>(rand()) / RAND_MAX);
                } else if (args.size() == 1 && args[0].isNumber()) {
                    int max = static_cast<int>(args[0].get<double>());
                    return Scripting::ScriptValue(static_cast<double>(rand() % max));
                } else if (args.size() == 2 && args[0].isNumber() && args[1].isNumber()) {
                    int min = static_cast<int>(args[0].get<double>());
                    int max = static_cast<int>(args[1].get<double>());
                    return Scripting::ScriptValue(static_cast<double>(min + rand() % (max - min + 1)));
                }
                return Scripting::ScriptValue(0.0);
            });

            // Set engine version
            registerGlobalVariable("ENGINE_VERSION", Scripting::ScriptValue("1.0.0"));
        }

        void ScriptSystem::updateScriptComponents(float deltaTime) {
            for (auto& pair : scriptComponents) {
                pair.second->onUpdate(deltaTime);
            }
        }

        std::time_t ScriptSystem::getFileModificationTime(const std::string& filename) {
            struct stat fileInfo;
            if (stat(filename.c_str(), &fileInfo) == 0) {
                return fileInfo.st_mtime;
            }
            return 0;
        }

    } // namespace Systems
} // namespace RPGEngine