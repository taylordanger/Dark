#include "LuaScriptEngine.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace RPGEngine {
    namespace Scripting {

        // Global pointer to current engine instance for C callback wrapper
        static LuaScriptEngine* g_currentEngine = nullptr;

        LuaScriptEngine::LuaScriptEngine() : L(nullptr) {
        }

        LuaScriptEngine::~LuaScriptEngine() {
            shutdown();
        }

        bool LuaScriptEngine::initialize() {
            // Create new Lua state
            L = luaL_newstate();
            if (!L) {
                lastError = "Failed to create Lua state";
                return false;
            }

            // Set global engine pointer for callbacks
            g_currentEngine = this;

            // Open standard libraries
            setupStandardLibraries();

            // Setup engine-specific API
            setupEngineAPI();

            clearError();
            return true;
        }

        void LuaScriptEngine::shutdown() {
            if (L) {
                lua_close(L);
                L = nullptr;
            }
            g_currentEngine = nullptr;
            registeredFunctions.clear();
            eventCallbacks.clear();
            activeContext.reset();
        }

        bool LuaScriptEngine::executeScript(const std::string& script) {
            if (!L) {
                lastError = "Lua state not initialized";
                return false;
            }

            int result = luaL_dostring(L, script.c_str());
            return checkLuaError(result);
        }

        bool LuaScriptEngine::executeFile(const std::string& filename) {
            if (!L) {
                lastError = "Lua state not initialized";
                return false;
            }

            // Read file content
            std::ifstream file(filename);
            if (!file.is_open()) {
                lastError = "Failed to open script file: " + filename;
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            return executeScript(buffer.str());
        }

        ScriptValue LuaScriptEngine::callFunction(const std::string& functionName, const std::vector<ScriptValue>& args) {
            if (!L) {
                lastError = "Lua state not initialized";
                return ScriptValue();
            }

            // Get function from global table
            lua_getglobal(L, functionName.c_str());
            if (!lua_isfunction(L, -1)) {
                lua_pop(L, 1);
                lastError = "Function not found: " + functionName;
                return ScriptValue();
            }

            // Push arguments
            for (const auto& arg : args) {
                pushScriptValue(arg);
            }

            // Call function
            int result = lua_pcall(L, static_cast<int>(args.size()), 1, 0);
            if (!checkLuaError(result)) {
                return ScriptValue();
            }

            // Get return value
            ScriptValue returnValue = popScriptValue();
            return returnValue;
        }

        void LuaScriptEngine::registerFunction(const std::string& name, ScriptCallback callback) {
            if (!L) {
                return;
            }

            registeredFunctions[name] = callback;

            // Create a closure with the function name as upvalue
            lua_pushstring(L, name.c_str());
            lua_pushcclosure(L, luaFunctionWrapper, 1);
            lua_setglobal(L, name.c_str());
        }

        void LuaScriptEngine::registerGlobal(const std::string& name, const ScriptValue& value) {
            if (!L) {
                return;
            }

            pushScriptValue(value);
            lua_setglobal(L, name.c_str());
        }

        void LuaScriptEngine::registerEventCallback(const std::string& eventType, ScriptEventCallback callback) {
            eventCallbacks[eventType].push_back(callback);
        }

        void LuaScriptEngine::triggerEvent(const std::string& eventType, const std::vector<ScriptValue>& args) {
            auto it = eventCallbacks.find(eventType);
            if (it != eventCallbacks.end()) {
                for (const auto& callback : it->second) {
                    callback(eventType, args);
                }
            }

            // Also trigger Lua event handlers if they exist
            if (L) {
                lua_getglobal(L, "onEvent");
                if (lua_isfunction(L, -1)) {
                    lua_pushstring(L, eventType.c_str());
                    for (const auto& arg : args) {
                        pushScriptValue(arg);
                    }
                    lua_pcall(L, static_cast<int>(args.size()) + 1, 0, 0);
                } else {
                    lua_pop(L, 1);
                }
            }
        }

        std::string LuaScriptEngine::getLastError() const {
            return lastError;
        }

        bool LuaScriptEngine::hasError() const {
            return !lastError.empty();
        }

        void LuaScriptEngine::clearError() {
            lastError.clear();
        }

        std::shared_ptr<ScriptContext> LuaScriptEngine::createContext() {
            if (!L) {
                return nullptr;
            }
            return std::make_shared<LuaScriptContext>(L);
        }

        void LuaScriptEngine::setActiveContext(std::shared_ptr<ScriptContext> context) {
            activeContext = std::dynamic_pointer_cast<LuaScriptContext>(context);
        }

        void LuaScriptEngine::pushScriptValue(const ScriptValue& value) {
            switch (value.type) {
                case ScriptValueType::Nil:
                    lua_pushnil(L);
                    break;
                case ScriptValueType::Boolean:
                    lua_pushboolean(L, value.get<bool>() ? 1 : 0);
                    break;
                case ScriptValueType::Number:
                    lua_pushnumber(L, value.get<double>());
                    break;
                case ScriptValueType::String:
                    lua_pushstring(L, value.get<std::string>().c_str());
                    break;
                default:
                    lua_pushnil(L);
                    break;
            }
        }

        ScriptValue LuaScriptEngine::popScriptValue() {
            if (lua_gettop(L) == 0) {
                return ScriptValue();
            }

            ScriptValue result;
            int type = lua_type(L, -1);

            switch (type) {
                case LUA_TNIL:
                    result = ScriptValue();
                    break;
                case LUA_TBOOLEAN:
                    result = ScriptValue(lua_toboolean(L, -1) != 0);
                    break;
                case LUA_TNUMBER:
                    result = ScriptValue(lua_tonumber(L, -1));
                    break;
                case LUA_TSTRING:
                    result = ScriptValue(lua_tostring(L, -1));
                    break;
                default:
                    result = ScriptValue();
                    break;
            }

            lua_pop(L, 1);
            return result;
        }

        bool LuaScriptEngine::checkLuaError(int result) {
            if (result != LUA_OK) {
                if (lua_gettop(L) > 0) {
                    lastError = lua_tostring(L, -1);
                    lua_pop(L, 1);
                } else {
                    lastError = "Unknown Lua error";
                }
                return false;
            }
            clearError();
            return true;
        }

        void LuaScriptEngine::setupStandardLibraries() {
            // Open safe standard libraries
            luaL_openlibs(L);
            
            // Remove potentially dangerous functions
            lua_pushnil(L);
            lua_setglobal(L, "dofile");
            lua_pushnil(L);
            lua_setglobal(L, "loadfile");
            lua_pushnil(L);
            lua_setglobal(L, "require");
        }

        void LuaScriptEngine::setupEngineAPI() {
            // Create engine table
            lua_newtable(L);
            
            // Add engine version
            lua_pushstring(L, "1.0.0");
            lua_setfield(L, -2, "version");
            
            // Set as global
            lua_setglobal(L, "Engine");

            // Register basic logging function
            registerFunction("print", [](const std::vector<ScriptValue>& args) -> ScriptValue {
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
                return ScriptValue();
            });
        }

        int LuaScriptEngine::luaFunctionWrapper(lua_State* L) {
            if (!g_currentEngine) {
                lua_pushstring(L, "No active script engine");
                lua_error(L);
                return 0;
            }

            // Get function name from upvalue
            const char* functionName = lua_tostring(L, lua_upvalueindex(1));
            if (!functionName) {
                lua_pushstring(L, "Invalid function name");
                lua_error(L);
                return 0;
            }

            // Find registered function
            auto it = g_currentEngine->registeredFunctions.find(functionName);
            if (it == g_currentEngine->registeredFunctions.end()) {
                lua_pushstring(L, ("Function not registered: " + std::string(functionName)).c_str());
                lua_error(L);
                return 0;
            }

            // Convert Lua arguments to ScriptValues
            int argc = lua_gettop(L);
            std::vector<ScriptValue> args;
            args.reserve(argc);

            for (int i = 1; i <= argc; ++i) {
                ScriptValue arg;
                int type = lua_type(L, i);
                
                switch (type) {
                    case LUA_TNIL:
                        arg = ScriptValue();
                        break;
                    case LUA_TBOOLEAN:
                        arg = ScriptValue(lua_toboolean(L, i) != 0);
                        break;
                    case LUA_TNUMBER:
                        arg = ScriptValue(lua_tonumber(L, i));
                        break;
                    case LUA_TSTRING:
                        arg = ScriptValue(lua_tostring(L, i));
                        break;
                    default:
                        arg = ScriptValue();
                        break;
                }
                args.push_back(arg);
            }

            // Call the registered function
            ScriptValue result = it->second(args);

            // Push result back to Lua
            g_currentEngine->pushScriptValue(result);
            return 1;
        }

        // LuaScriptContext implementation
        LuaScriptContext::LuaScriptContext(lua_State* L) : L(L), contextRef(LUA_NOREF) {
            // Create a new table for this context
            lua_newtable(L);
            contextRef = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        LuaScriptContext::~LuaScriptContext() {
            if (L && contextRef != LUA_NOREF) {
                luaL_unref(L, LUA_REGISTRYINDEX, contextRef);
                contextRef = LUA_NOREF;
            }
        }

        void LuaScriptContext::setGlobal(const std::string& name, const ScriptValue& value) {
            if (!L || contextRef == LUA_NOREF) {
                return;
            }

            // Get context table
            lua_rawgeti(L, LUA_REGISTRYINDEX, contextRef);
            
            // Push value and set in context table
            switch (value.type) {
                case ScriptValueType::Nil:
                    lua_pushnil(L);
                    break;
                case ScriptValueType::Boolean:
                    lua_pushboolean(L, value.get<bool>() ? 1 : 0);
                    break;
                case ScriptValueType::Number:
                    lua_pushnumber(L, value.get<double>());
                    break;
                case ScriptValueType::String:
                    lua_pushstring(L, value.get<std::string>().c_str());
                    break;
                default:
                    lua_pushnil(L);
                    break;
            }
            
            lua_setfield(L, -2, name.c_str());
            lua_pop(L, 1); // Pop context table
        }

        ScriptValue LuaScriptContext::getGlobal(const std::string& name) {
            if (!L || contextRef == LUA_NOREF) {
                return ScriptValue();
            }

            // Get context table
            lua_rawgeti(L, LUA_REGISTRYINDEX, contextRef);
            
            // Get value from context table
            lua_getfield(L, -1, name.c_str());
            
            ScriptValue result;
            int type = lua_type(L, -1);
            
            switch (type) {
                case LUA_TNIL:
                    result = ScriptValue();
                    break;
                case LUA_TBOOLEAN:
                    result = ScriptValue(lua_toboolean(L, -1) != 0);
                    break;
                case LUA_TNUMBER:
                    result = ScriptValue(lua_tonumber(L, -1));
                    break;
                case LUA_TSTRING:
                    result = ScriptValue(lua_tostring(L, -1));
                    break;
                default:
                    result = ScriptValue();
                    break;
            }
            
            lua_pop(L, 2); // Pop value and context table
            return result;
        }

        bool LuaScriptContext::executeInContext(const std::string& script) {
            if (!L || contextRef == LUA_NOREF) {
                return false;
            }

            // For now, just execute in global context
            // Context isolation can be improved later
            int result = luaL_dostring(L, script.c_str());
            return result == LUA_OK;
        }

    } // namespace Scripting
} // namespace RPGEngine