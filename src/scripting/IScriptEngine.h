#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <any>
#include <vector>

namespace RPGEngine {
    namespace Scripting {

        // Forward declarations
        class ScriptContext;
        class ScriptFunction;

        // Script value types for parameter passing
        enum class ScriptValueType {
            Nil,
            Boolean,
            Number,
            String,
            Function,
            Table
        };

        struct ScriptValue {
            ScriptValueType type = ScriptValueType::Nil;
            std::any value;

            ScriptValue() = default;
            ScriptValue(bool b) : type(ScriptValueType::Boolean), value(b) {}
            ScriptValue(double d) : type(ScriptValueType::Number), value(d) {}
            ScriptValue(const std::string& s) : type(ScriptValueType::String), value(s) {}
            ScriptValue(const char* s) : type(ScriptValueType::String), value(std::string(s)) {}

            template<typename T>
            T get() const {
                return std::any_cast<T>(value);
            }

            bool isNil() const { return type == ScriptValueType::Nil; }
            bool isBool() const { return type == ScriptValueType::Boolean; }
            bool isNumber() const { return type == ScriptValueType::Number; }
            bool isString() const { return type == ScriptValueType::String; }
        };

        // Callback function type for C++ functions exposed to scripts
        using ScriptCallback = std::function<ScriptValue(const std::vector<ScriptValue>&)>;

        // Event callback type for script event handling
        using ScriptEventCallback = std::function<void(const std::string&, const std::vector<ScriptValue>&)>;

        /**
         * Abstract interface for script engines
         */
        class IScriptEngine {
        public:
            virtual ~IScriptEngine() = default;

            // Engine lifecycle
            virtual bool initialize() = 0;
            virtual void shutdown() = 0;

            // Script execution
            virtual bool executeScript(const std::string& script) = 0;
            virtual bool executeFile(const std::string& filename) = 0;
            virtual ScriptValue callFunction(const std::string& functionName, const std::vector<ScriptValue>& args = {}) = 0;

            // Function registration
            virtual void registerFunction(const std::string& name, ScriptCallback callback) = 0;
            virtual void registerGlobal(const std::string& name, const ScriptValue& value) = 0;

            // Event system
            virtual void registerEventCallback(const std::string& eventType, ScriptEventCallback callback) = 0;
            virtual void triggerEvent(const std::string& eventType, const std::vector<ScriptValue>& args = {}) = 0;

            // Error handling
            virtual std::string getLastError() const = 0;
            virtual bool hasError() const = 0;
            virtual void clearError() = 0;

            // Context management
            virtual std::shared_ptr<ScriptContext> createContext() = 0;
            virtual void setActiveContext(std::shared_ptr<ScriptContext> context) = 0;
        };

        /**
         * Script context for sandboxed execution
         */
        class ScriptContext {
        public:
            virtual ~ScriptContext() = default;
            virtual void setGlobal(const std::string& name, const ScriptValue& value) = 0;
            virtual ScriptValue getGlobal(const std::string& name) = 0;
            virtual bool executeInContext(const std::string& script) = 0;
        };

    } // namespace Scripting
} // namespace RPGEngine