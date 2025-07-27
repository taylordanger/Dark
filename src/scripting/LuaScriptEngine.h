#pragma once

#include "IScriptEngine.h"
#include <lua.hpp>
#include <memory>
#include <unordered_map>

namespace RPGEngine {
    namespace Scripting {

        class LuaScriptContext;

        /**
         * Lua implementation of the script engine
         */
        class LuaScriptEngine : public IScriptEngine {
        public:
            LuaScriptEngine();
            ~LuaScriptEngine() override;

            // IScriptEngine implementation
            bool initialize() override;
            void shutdown() override;

            bool executeScript(const std::string& script) override;
            bool executeFile(const std::string& filename) override;
            ScriptValue callFunction(const std::string& functionName, const std::vector<ScriptValue>& args = {}) override;

            void registerFunction(const std::string& name, ScriptCallback callback) override;
            void registerGlobal(const std::string& name, const ScriptValue& value) override;

            void registerEventCallback(const std::string& eventType, ScriptEventCallback callback) override;
            void triggerEvent(const std::string& eventType, const std::vector<ScriptValue>& args = {}) override;

            std::string getLastError() const override;
            bool hasError() const override;
            void clearError() override;

            std::shared_ptr<ScriptContext> createContext() override;
            void setActiveContext(std::shared_ptr<ScriptContext> context) override;

            // Lua-specific methods
            lua_State* getLuaState() const { return L; }

        private:
            lua_State* L;
            std::string lastError;
            std::unordered_map<std::string, ScriptCallback> registeredFunctions;
            std::unordered_map<std::string, std::vector<ScriptEventCallback>> eventCallbacks;
            std::shared_ptr<LuaScriptContext> activeContext;

            // Helper methods
            void pushScriptValue(const ScriptValue& value);
            ScriptValue popScriptValue();
            bool checkLuaError(int result);
            void setupStandardLibraries();
            void setupEngineAPI();

            // Static callback wrapper for Lua C functions
            static int luaFunctionWrapper(lua_State* L);
        };

        /**
         * Lua-specific script context
         */
        class LuaScriptContext : public ScriptContext {
        public:
            LuaScriptContext(lua_State* L);
            ~LuaScriptContext() override;

            void setGlobal(const std::string& name, const ScriptValue& value) override;
            ScriptValue getGlobal(const std::string& name) override;
            bool executeInContext(const std::string& script) override;

        private:
            lua_State* L;
            int contextRef; // Reference to the context table in Lua registry
        };

    } // namespace Scripting
} // namespace RPGEngine