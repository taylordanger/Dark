#include <iostream>
#include <memory>
#include "../src/scripting/LuaScriptEngine.h"

using namespace RPGEngine;

void testBasicScriptExecution() {
    std::cout << "\n=== Testing Basic Script Execution ===" << std::endl;
    
    auto scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
    if (!scriptEngine->initialize()) {
        std::cerr << "Failed to initialize script engine" << std::endl;
        return;
    }

    // Test basic script execution
    std::string testScript = 
        "function greet(name)\n"
        "    return \"Hello, \" .. name .. \"!\"\n"
        "end\n"
        "\n"
        "function add(a, b)\n"
        "    return a + b\n"
        "end\n"
        "\n"
        "testVariable = 42";

    bool result = scriptEngine->executeScript(testScript);
    std::cout << "Script execution result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << scriptEngine->getLastError() << std::endl;
    }

    // Test function calls
    std::vector<Scripting::ScriptValue> args = { Scripting::ScriptValue("World") };
    Scripting::ScriptValue greeting = scriptEngine->callFunction("greet", args);
    if (greeting.isString()) {
        std::cout << "Function call result: " << greeting.get<std::string>() << std::endl;
    }

    // Test math function
    std::vector<Scripting::ScriptValue> mathArgs = { 
        Scripting::ScriptValue(10.0), 
        Scripting::ScriptValue(5.0) 
    };
    Scripting::ScriptValue sum = scriptEngine->callFunction("add", mathArgs);
    if (sum.isNumber()) {
        std::cout << "Math result: " << sum.get<double>() << std::endl;
    }

    scriptEngine->shutdown();
}

void testEventHandling() {
    std::cout << "\n=== Testing Event Handling ===" << std::endl;
    
    auto scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
    if (!scriptEngine->initialize()) {
        std::cerr << "Failed to initialize script engine" << std::endl;
        return;
    }

    // Register C++ event handler
    scriptEngine->registerEventCallback("player_action", [](const std::string& eventType, const std::vector<Scripting::ScriptValue>& args) {
        std::cout << "C++ handler received event: " << eventType;
        for (const auto& arg : args) {
            if (arg.isString()) {
                std::cout << " " << arg.get<std::string>();
            } else if (arg.isNumber()) {
                std::cout << " " << arg.get<double>();
            }
        }
        std::cout << std::endl;
    });

    // Execute script that handles events
    std::string eventScript = 
        "function onEvent(eventType, action, value)\n"
        "    print(\"Lua handler - Event:\", eventType, \"Action:\", action, \"Value:\", value)\n"
        "end";

    scriptEngine->executeScript(eventScript);

    // Trigger events
    std::vector<Scripting::ScriptValue> eventArgs = { 
        Scripting::ScriptValue("jump"), 
        Scripting::ScriptValue(10.0) 
    };
    scriptEngine->triggerEvent("player_action", eventArgs);

    scriptEngine->shutdown();
}

void testScriptContext() {
    std::cout << "\n=== Testing Script Context ===" << std::endl;
    
    auto scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
    if (!scriptEngine->initialize()) {
        std::cerr << "Failed to initialize script engine" << std::endl;
        return;
    }

    // Test global variables instead of context for now
    scriptEngine->registerGlobal("playerName", Scripting::ScriptValue("TestPlayer"));
    scriptEngine->registerGlobal("playerLevel", Scripting::ScriptValue(5.0));

    // Execute script
    std::string contextScript = 
        "function getPlayerInfo()\n"
        "    return \"Player: \" .. playerName .. \" (Level \" .. playerLevel .. \")\"\n"
        "end\n"
        "print(getPlayerInfo())";

    bool contextResult = scriptEngine->executeScript(contextScript);
    std::cout << "Context script execution: " << (contextResult ? "SUCCESS" : "FAILED") << std::endl;

    scriptEngine->shutdown();
}

void testCustomFunctions() {
    std::cout << "\n=== Testing Custom Functions ===" << std::endl;
    
    auto scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
    if (!scriptEngine->initialize()) {
        std::cerr << "Failed to initialize script engine" << std::endl;
        return;
    }

    // Register custom C++ function
    scriptEngine->registerFunction("multiply", [](const std::vector<Scripting::ScriptValue>& args) -> Scripting::ScriptValue {
        if (args.size() >= 2 && args[0].isNumber() && args[1].isNumber()) {
            double result = args[0].get<double>() * args[1].get<double>();
            return Scripting::ScriptValue(result);
        }
        return Scripting::ScriptValue(0.0);
    });

    // Register global variable
    scriptEngine->registerGlobal("PI", Scripting::ScriptValue(3.14159));

    // Test the custom function
    std::string testScript = 
        "function calculateArea(radius)\n"
        "    return multiply(PI, multiply(radius, radius))\n"
        "end\n"
        "\n"
        "result = calculateArea(5)\n"
        "print(\"Area of circle with radius 5:\", result)";

    bool result = scriptEngine->executeScript(testScript);
    std::cout << "Custom function test: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << scriptEngine->getLastError() << std::endl;
    }

    scriptEngine->shutdown();
}

int main() {
    std::cout << "RPG Engine Scripting System Minimal Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testBasicScriptExecution();
    testEventHandling();
    testScriptContext();
    testCustomFunctions();

    std::cout << "\nAll scripting tests completed!" << std::endl;
    return 0;
}