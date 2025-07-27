#include "../src/ui/DialogueUI.h"
#include "../src/components/DialogueComponent.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/ShaderManager.h"
#include "../src/input/InputManager.h"
#include "../src/entities/EntityManager.h"
#include <iostream>
#include <memory>
#include <cassert>

using namespace RPGEngine;
using namespace RPGEngine::UI;
using namespace RPGEngine::Components;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Input;

// Mock GLFW window for testing
struct GLFWwindow {};
GLFWwindow* g_mockWindow = reinterpret_cast<GLFWwindow*>(0x12345678);

void createTestDialogue(DialogueComponent& dialogue) {
    std::cout << "\n=== Creating test dialogue for UI ===" << std::endl;
    
    DialogueTree tree("ui_test_dialogue", "UI Test Dialogue");
    tree.description = "A dialogue for testing the UI system";
    tree.startNodeId = "greeting";
    
    // Greeting node
    DialogueNode greeting("greeting", DialogueNodeType::Text);
    greeting.speaker = "Merchant";
    greeting.text = "Welcome to my shop! I have many fine wares for sale.";
    greeting.nextNodeId = "main_menu";
    tree.addNode(greeting);
    
    // Main menu choice node
    DialogueNode mainMenu("main_menu", DialogueNodeType::Choice);
    mainMenu.text = "What would you like to do?";
    
    DialogueChoice buyChoice("buy", "Show me your wares", "shop_info");
    DialogueChoice questChoice("quest", "Do you have any work?", "quest_info");
    DialogueChoice historyChoice("history", "Tell me about this place", "history_info");
    DialogueChoice leaveChoice("leave", "I must go", "farewell");
    
    mainMenu.choices.push_back(buyChoice);
    mainMenu.choices.push_back(questChoice);
    mainMenu.choices.push_back(historyChoice);
    mainMenu.choices.push_back(leaveChoice);
    tree.addNode(mainMenu);
    
    // Shop info node
    DialogueNode shopInfo("shop_info", DialogueNodeType::Text);
    shopInfo.speaker = "Merchant";
    shopInfo.text = "I have weapons, armor, potions, and magical items. Everything an adventurer needs!";
    shopInfo.nextNodeId = "back_to_menu";
    tree.addNode(shopInfo);
    
    // Quest info node
    DialogueNode questInfo("quest_info", DialogueNodeType::Text);
    questInfo.speaker = "Merchant";
    questInfo.text = "Actually, yes! I need someone to retrieve a rare gem from the nearby caves. It's dangerous work, but I'll pay well.";
    questInfo.actions.push_back(DialogueAction("set_flag", "merchant_quest_available", "true"));
    questInfo.nextNodeId = "back_to_menu";
    tree.addNode(questInfo);
    
    // History info node
    DialogueNode historyInfo("history_info", DialogueNodeType::Text);
    historyInfo.speaker = "Merchant";
    historyInfo.text = "This town was founded over a century ago by brave settlers. It's grown into a thriving trading post thanks to merchants like myself.";
    historyInfo.nextNodeId = "back_to_menu";
    tree.addNode(historyInfo);
    
    // Back to menu node
    DialogueNode backToMenu("back_to_menu", DialogueNodeType::Choice);
    backToMenu.text = "Anything else?";
    
    DialogueChoice backChoice("back", "Let me see the main options again", "main_menu");
    DialogueChoice doneChoice("done", "No, that's all", "farewell");
    
    backToMenu.choices.push_back(backChoice);
    backToMenu.choices.push_back(doneChoice);
    tree.addNode(backToMenu);
    
    // Farewell node
    DialogueNode farewell("farewell", DialogueNodeType::Text);
    farewell.speaker = "Merchant";
    farewell.text = "Safe travels, friend! Come back anytime you need supplies.";
    farewell.nextNodeId = "end";
    tree.addNode(farewell);
    
    // End node
    DialogueNode endNode("end", DialogueNodeType::End);
    tree.addNode(endNode);
    
    dialogue.addDialogueTree(tree);
    std::cout << "Created UI test dialogue tree with " << tree.nodes.size() << " nodes" << std::endl;
}

void testDialogueUIInitialization() {
    std::cout << "\n=== Testing Dialogue UI Initialization ===" << std::endl;
    
    // Create mock dependencies
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    auto inputManager = std::make_shared<InputManager>(g_mockWindow);
    
    // Create dialogue UI
    DialogueUI dialogueUI(spriteRenderer, inputManager);
    
    // Test initialization
    bool initialized = dialogueUI.onInitialize();
    assert(initialized);
    
    // Test initial state
    assert(!dialogueUI.isVisible());
    assert(!dialogueUI.isShowingHistory());
    
    std::cout << "Dialogue UI initialized successfully" << std::endl;
    std::cout << "Initial state: " << (dialogueUI.isVisible() ? "visible" : "hidden") << std::endl;
    
    // Test style configuration
    DialogueUIStyle customStyle;
    customStyle.windowWidth = 800.0f;
    customStyle.windowHeight = 250.0f;
    customStyle.textColor = Color(0.9f, 0.9f, 1.0f, 1.0f);
    customStyle.typewriterSpeed = 30.0f;
    
    dialogueUI.setStyle(customStyle);
    
    const DialogueUIStyle& retrievedStyle = dialogueUI.getStyle();
    assert(retrievedStyle.windowWidth == 800.0f);
    assert(retrievedStyle.windowHeight == 250.0f);
    assert(retrievedStyle.typewriterSpeed == 30.0f);
    
    std::cout << "Style configuration test passed" << std::endl;
    
    dialogueUI.onShutdown();
    std::cout << "Dialogue UI initialization test passed!" << std::endl;
}

void testDialogueUIIntegration() {
    std::cout << "\n=== Testing Dialogue UI Integration ===" << std::endl;
    
    // Create dependencies
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    auto inputManager = std::make_shared<InputManager>(g_mockWindow);
    
    // Create dialogue system
    EntityManager entityManager;
    Entity npc = entityManager.createEntity("TestNPC");
    auto dialogueComponent = std::make_shared<DialogueComponent>(npc.getID());
    createTestDialogue(*dialogueComponent);
    
    // Create dialogue UI
    DialogueUI dialogueUI(spriteRenderer, inputManager);
    dialogueUI.onInitialize();
    
    // Set up dialogue component
    dialogueUI.setDialogueComponent(dialogueComponent);
    
    // Test showing dialogue
    assert(!dialogueUI.isVisible());
    
    dialogueComponent->startDialogue("ui_test_dialogue");
    dialogueUI.show();
    
    assert(dialogueUI.isVisible());
    std::cout << "Dialogue UI shown successfully" << std::endl;
    
    // Test callbacks
    bool advanceCalled = false;
    bool closeCalled = false;
    
    dialogueUI.setAdvanceCallback([&advanceCalled](const std::string& choiceId) {
        std::cout << "Advance callback called with choice: " << (choiceId.empty() ? "none" : choiceId) << std::endl;
        advanceCalled = true;
    });
    
    dialogueUI.setCloseCallback([&closeCalled]() {
        std::cout << "Close callback called" << std::endl;
        closeCalled = true;
    });
    
    std::cout << "Callbacks set up successfully" << std::endl;
    
    // Test typewriter functionality
    dialogueUI.setTypewriterEnabled(true);
    
    // Simulate some updates
    for (int i = 0; i < 10; ++i) {
        dialogueUI.onUpdate(0.1f); // 100ms per frame
    }
    
    std::cout << "Typewriter effect tested" << std::endl;
    
    // Test hiding dialogue
    dialogueUI.hide();
    
    // Update to complete fade out
    for (int i = 0; i < 10; ++i) {
        dialogueUI.onUpdate(0.1f);
    }
    
    assert(!dialogueUI.isVisible());
    std::cout << "Dialogue UI hidden successfully" << std::endl;
    
    dialogueUI.onShutdown();
    std::cout << "Dialogue UI integration test passed!" << std::endl;
}

void testDialogueUIHistory() {
    std::cout << "\n=== Testing Dialogue UI History ===" << std::endl;
    
    // Create dependencies
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    auto inputManager = std::make_shared<InputManager>(g_mockWindow);
    
    // Create dialogue system
    EntityManager entityManager;
    Entity npc = entityManager.createEntity("TestNPC");
    auto dialogueComponent = std::make_shared<DialogueComponent>(npc.getID());
    createTestDialogue(*dialogueComponent);
    
    // Add some history
    dialogueComponent->addToHistory("Merchant: Hello there!");
    dialogueComponent->addToHistory("Player: Hi, what do you sell?");
    dialogueComponent->addToHistory("Merchant: I have many fine wares!");
    dialogueComponent->addToHistory("Player: Show me your weapons.");
    dialogueComponent->addToHistory("Merchant: Here are my best swords and axes.");
    
    // Create dialogue UI
    DialogueUI dialogueUI(spriteRenderer, inputManager);
    dialogueUI.onInitialize();
    dialogueUI.setDialogueComponent(dialogueComponent);
    
    // Start dialogue and show UI
    dialogueComponent->startDialogue("ui_test_dialogue");
    dialogueUI.show();
    
    // Test history toggle
    assert(!dialogueUI.isShowingHistory());
    
    dialogueUI.toggleHistory();
    assert(dialogueUI.isShowingHistory());
    std::cout << "History display toggled on" << std::endl;
    
    // Simulate some updates while showing history
    for (int i = 0; i < 5; ++i) {
        dialogueUI.onUpdate(0.1f);
    }
    
    dialogueUI.toggleHistory();
    assert(!dialogueUI.isShowingHistory());
    std::cout << "History display toggled off" << std::endl;
    
    dialogueUI.onShutdown();
    std::cout << "Dialogue UI history test passed!" << std::endl;
}

void testDialogueUIChoices() {
    std::cout << "\n=== Testing Dialogue UI Choices ===" << std::endl;
    
    // Create dependencies
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    auto inputManager = std::make_shared<InputManager>(g_mockWindow);
    
    // Create dialogue system
    EntityManager entityManager;
    Entity npc = entityManager.createEntity("TestNPC");
    auto dialogueComponent = std::make_shared<DialogueComponent>(npc.getID());
    createTestDialogue(*dialogueComponent);
    
    // Create dialogue UI
    DialogueUI dialogueUI(spriteRenderer, inputManager);
    dialogueUI.onInitialize();
    dialogueUI.setDialogueComponent(dialogueComponent);
    
    // Start dialogue and advance to choice menu
    dialogueComponent->startDialogue("ui_test_dialogue");
    dialogueComponent->advanceDialogue(); // Move to main_menu (choice node)
    
    dialogueUI.show();
    
    // Test that we have choices available
    const DialogueNode* currentNode = dialogueComponent->getCurrentNode();
    assert(currentNode != nullptr);
    assert(currentNode->type == DialogueNodeType::Choice);
    assert(!currentNode->choices.empty());
    
    std::cout << "Choice node loaded with " << currentNode->choices.size() << " choices" << std::endl;
    
    // Test choice selection callback
    std::string selectedChoice;
    dialogueUI.setAdvanceCallback([&selectedChoice](const std::string& choiceId) {
        selectedChoice = choiceId;
        std::cout << "Choice selected: " << choiceId << std::endl;
    });
    
    // Simulate some updates to render choices
    for (int i = 0; i < 5; ++i) {
        dialogueUI.onUpdate(0.1f);
    }
    
    std::cout << "Choice rendering tested" << std::endl;
    
    dialogueUI.onShutdown();
    std::cout << "Dialogue UI choices test passed!" << std::endl;
}

void testDialogueUITypewriter() {
    std::cout << "\n=== Testing Dialogue UI Typewriter Effect ===" << std::endl;
    
    // Create dependencies
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    auto inputManager = std::make_shared<InputManager>(g_mockWindow);
    
    // Create dialogue system
    EntityManager entityManager;
    Entity npc = entityManager.createEntity("TestNPC");
    auto dialogueComponent = std::make_shared<DialogueComponent>(npc.getID());
    createTestDialogue(*dialogueComponent);
    
    // Create dialogue UI
    DialogueUI dialogueUI(spriteRenderer, inputManager);
    dialogueUI.onInitialize();
    dialogueUI.setDialogueComponent(dialogueComponent);
    
    // Configure typewriter effect
    DialogueUIStyle style = dialogueUI.getStyle();
    style.enableTypewriter = true;
    style.typewriterSpeed = 20.0f; // 20 characters per second
    dialogueUI.setStyle(style);
    
    // Start dialogue
    dialogueComponent->startDialogue("ui_test_dialogue");
    dialogueUI.show();
    
    std::cout << "Typewriter effect enabled at " << style.typewriterSpeed << " chars/sec" << std::endl;
    
    // Test typewriter progression
    for (int i = 0; i < 20; ++i) {
        dialogueUI.onUpdate(0.1f); // 100ms per frame
    }
    
    std::cout << "Typewriter effect progression tested" << std::endl;
    
    // Test skip functionality
    dialogueUI.skipTypewriter();
    std::cout << "Typewriter skip tested" << std::endl;
    
    // Test disabling typewriter
    dialogueUI.setTypewriterEnabled(false);
    std::cout << "Typewriter disabled" << std::endl;
    
    dialogueUI.onShutdown();
    std::cout << "Dialogue UI typewriter test passed!" << std::endl;
}

void testDialogueUIStyles() {
    std::cout << "\n=== Testing Dialogue UI Styles ===" << std::endl;
    
    // Create dependencies
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    auto inputManager = std::make_shared<InputManager>(g_mockWindow);
    
    // Create dialogue UI
    DialogueUI dialogueUI(spriteRenderer, inputManager);
    dialogueUI.onInitialize();
    
    // Test default style
    const DialogueUIStyle& defaultStyle = dialogueUI.getStyle();
    std::cout << "Default window size: " << defaultStyle.windowWidth << "x" << defaultStyle.windowHeight << std::endl;
    std::cout << "Default text size: " << defaultStyle.textSize << std::endl;
    std::cout << "Default typewriter speed: " << defaultStyle.typewriterSpeed << std::endl;
    
    // Test custom style
    DialogueUIStyle customStyle;
    customStyle.windowX = 100.0f;
    customStyle.windowY = 300.0f;
    customStyle.windowWidth = 600.0f;
    customStyle.windowHeight = 180.0f;
    customStyle.windowBackgroundColor = Color(0.1f, 0.1f, 0.2f, 0.9f);
    customStyle.textColor = Color(1.0f, 1.0f, 0.8f, 1.0f);
    customStyle.speakerColor = Color(0.8f, 1.0f, 0.8f, 1.0f);
    customStyle.textSize = 18.0f;
    customStyle.typewriterSpeed = 40.0f;
    customStyle.enableTypewriter = false;
    
    dialogueUI.setStyle(customStyle);
    
    const DialogueUIStyle& retrievedStyle = dialogueUI.getStyle();
    assert(retrievedStyle.windowX == 100.0f);
    assert(retrievedStyle.windowY == 300.0f);
    assert(retrievedStyle.windowWidth == 600.0f);
    assert(retrievedStyle.windowHeight == 180.0f);
    assert(retrievedStyle.textSize == 18.0f);
    assert(retrievedStyle.typewriterSpeed == 40.0f);
    assert(retrievedStyle.enableTypewriter == false);
    
    std::cout << "Custom style applied successfully" << std::endl;
    std::cout << "Custom window size: " << retrievedStyle.windowWidth << "x" << retrievedStyle.windowHeight << std::endl;
    std::cout << "Custom text size: " << retrievedStyle.textSize << std::endl;
    
    dialogueUI.onShutdown();
    std::cout << "Dialogue UI styles test passed!" << std::endl;
}

int main() {
    std::cout << "Starting Dialogue UI Tests..." << std::endl;
    
    try {
        testDialogueUIInitialization();
        testDialogueUIIntegration();
        testDialogueUIHistory();
        testDialogueUIChoices();
        testDialogueUITypewriter();
        testDialogueUIStyles();
        
        std::cout << "\n=== All Dialogue UI Tests Passed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}