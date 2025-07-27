#include <iostream>
#include <memory>
#include "../src/scripting/LuaScriptEngine.h"
#include "../src/components/ScriptComponent.h"
#include "../src/systems/ScriptSystem.h"

using namespace RPGEngine;

void testBasicScriptExecution() {
    std::cout << "\n=== Testing Basic Script Execution ===" << std::endl;
    
    auto scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
    if (!scriptEngine->initialize()) {
        std::cerr << "Failed to initialize script engine" << std::endl;
        return;
    }

    // Test basic script execution
    std::string testScript = R"(
        function greet(name)
            return "Hello, " .. name .. "!"
        end
        
        function add(a, b)
            return a + b
        end
        
        testVariable = 42
    )";

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

void testScriptComponent() {
    std::cout << "\n=== Testing Script Component ===" << std::endl;
    
    auto scriptEngine = std::make_shared<Scripting::LuaScriptEngine>();
    if (!scriptEngine->initialize()) {
        std::cerr << "Failed to initialize script engine" << std::endl;
        return;
    }

    // Create script component
    Components::EntityId testEntityId = 123;
    auto scriptComponent = std::make_shared<Components::ScriptComponent>(testEntityId);
    scriptComponent->setScriptEngine(scriptEngine);

    // Set a test script
    std::string entityScript = R"(
        health = 100
        name = "TestEntity"
        
        function onInitialize()
            log("Entity", entityId, "initialized with health:", health)
        end
        
        function onUpdate(deltaTime)
            -- Simple health regeneration
            if health < 100 then
                health = health + deltaTime * 10
                if health > 100 then
                    health = 100
                end
            end
        end
        
        function takeDamage(amount)
            health = health - amount
            log("Entity", entityId, "took", amount, "damage. Health:", health)
            return health
        end
        
        function getName()
            return name
        end
    )";

    scriptComponent->setScript(entityScript);
    scriptComponent->onInitialize();

    // Test script function calls
    std::vector<Scripting::ScriptValue> damageArgs = { Scripting::ScriptValue(25.0) };
    scriptComponent->callScriptFunction("takeDamage", damageArgs);

    // Test getting script variables
    Scripting::ScriptValue healthValue = scriptComponent->getScriptVariable("health");
    if (healthValue.isNumber()) {
        std::cout << "Current health: " << healthValue.get<double>() << std::endl;
    }

    // Test update
    scriptComponent->onUpdate(1.0f); // 1 second update

    healthValue = scriptComponent->getScriptVariable("health");
    if (healthValue.isNumber()) {
        std::cout << "Health after update: " << healthValue.get<double>() << std::endl;
    }

    scriptComponent->onDestroy();
    scriptEngine->shutdown();
}

void testScriptSystem() {
    std::cout << "\n=== Testing Script System ===" << std::endl;
    
    auto scriptSystem = std::make_shared<Systems::ScriptSystem>();
    scriptSystem->initialize();

    // Test global script execution
    std::string globalScript = R"(
        gameState = {
            score = 0,
            level = 1
        }
        
        function increaseScore(points)
            gameState.score = gameState.score + points
            log("Score increased by", points, "Total score:", gameState.score)
            return gameState.score
        end
        
        function onEvent(eventType, ...)
            log("Global event received:", eventType)
        end
    )";

    bool result = scriptSystem->executeGlobalScript(globalScript);
    std::cout << "Global script execution: " << (result ? "SUCCESS" : "FAILED") << std::endl;

    // Test global function call
    std::vector<Scripting::ScriptValue> scoreArgs = { Scripting::ScriptValue(100.0) };
    Scripting::ScriptValue newScore = scriptSystem->callGlobalFunction("increaseScore", scoreArgs);
    if (newScore.isNumber()) {
        std::cout << "New score: " << newScore.get<double>() << std::endl;
    }

    // Test event system
    std::vector<Scripting::ScriptValue> eventArgs = { Scripting::ScriptValue("player_died") };
    scriptSystem->triggerGlobalEvent("game_event", eventArgs);

    // Test component integration
    Components::EntityId testEntityId = 456;
    auto scriptComponent = std::make_shared<Components::ScriptComponent>(testEntityId);
    
    std::string componentScript = R"(
        function onInitialize()
            log("Component initialized for entity", entityId)
        end
        
        function onUpdate(deltaTime)
            -- This would be called every frame
        end
    )";
    
    scriptComponent->setScript(componentScript);
    scriptSystem->onComponentAdded(testEntityId, scriptComponent);

    // Simulate update
    scriptSystem->update(0.016f); // ~60 FPS

    scriptSystem->onComponentRemoved(testEntityId);
    scriptSystem->shutdown();
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
    std::string eventScript = R"(
        function onEvent(eventType, action, value)
            log("Lua handler - Event:", eventType, "Action:", action, "Value:", value)
        end
    )";

    scriptEngine->executeScript(eventScript);

    // Trigger events
    std::vector<Scripting::ScriptValue> eventArgs = { 
        Scripting::ScriptValue("jump"), 
        Scripting::ScriptValue(10.0) 
    };
    scriptEngine->triggerEvent("player_action", eventArgs);

    scriptEngine->shutdown();
}

int main() {
    std::cout << "RPG Engine Scripting System Test" << std::endl;
    std::cout << "=================================" << std::endl;

    testBasicScriptExecution();
    testScriptComponent();
    testScriptSystem();
    testEventHandling();

    std::cout << "\nAll scripting tests completed!" << std::endl;
    return 0;
}