/**
 * Dialogue UI Integration Example
 * 
 * This example demonstrates how to integrate the DialogueUI system
 * with the existing dialogue component and game systems.
 * 
 * Note: This is a conceptual example showing the integration pattern.
 * In a real implementation, you would need to link against the actual
 * compiled libraries and initialize the graphics/input systems properly.
 */

#include "../src/ui/DialogueUI.h"
#include "../src/components/DialogueComponent.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/input/InputManager.h"
#include "../src/entities/EntityManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::UI;
using namespace RPGEngine::Components;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Input;

/**
 * Example game class showing dialogue UI integration
 */
class DialogueUIExample {
public:
    DialogueUIExample() = default;
    
    /**
     * Initialize the dialogue UI system
     */
    bool initialize() {
        std::cout << "=== Dialogue UI Integration Example ===" << std::endl;
        
        // In a real implementation, you would initialize these properly:
        // m_graphicsAPI = std::make_shared<OpenGLAPI>();
        // m_graphicsAPI->initialize(800, 600, "Dialogue UI Example", false);
        // m_shaderManager = std::make_shared<ShaderManager>(m_graphicsAPI);
        // m_spriteRenderer = std::make_shared<SpriteRenderer>(m_graphicsAPI, m_shaderManager);
        // m_inputManager = std::make_shared<InputManager>(window);
        
        std::cout << "1. Graphics and input systems would be initialized here" << std::endl;
        
        // Create dialogue UI with custom styling
        // m_dialogueUI = std::make_unique<DialogueUI>(m_spriteRenderer, m_inputManager);
        
        DialogueUIStyle customStyle;
        customStyle.windowX = 50.0f;
        customStyle.windowY = 400.0f;
        customStyle.windowWidth = 700.0f;
        customStyle.windowHeight = 200.0f;
        customStyle.windowBackgroundColor = Color(0.0f, 0.0f, 0.0f, 0.85f);
        customStyle.windowBorderColor = Color(0.8f, 0.8f, 1.0f, 1.0f);
        customStyle.textColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
        customStyle.speakerColor = Color(1.0f, 1.0f, 0.6f, 1.0f);
        customStyle.choiceSelectedColor = Color(1.0f, 1.0f, 0.8f, 1.0f);
        customStyle.enableTypewriter = true;
        customStyle.typewriterSpeed = 40.0f;
        
        std::cout << "2. Dialogue UI created with custom styling:" << std::endl;
        std::cout << "   - Window size: " << customStyle.windowWidth << "x" << customStyle.windowHeight << std::endl;
        std::cout << "   - Typewriter speed: " << customStyle.typewriterSpeed << " chars/sec" << std::endl;
        
        // m_dialogueUI->setStyle(customStyle);
        // m_dialogueUI->onInitialize();
        
        // Set up dialogue UI callbacks
        setupDialogueCallbacks();
        
        // Create test NPC with dialogue
        createTestNPC();
        
        std::cout << "3. Dialogue UI system initialized successfully" << std::endl;
        return true;
    }
    
    /**
     * Set up dialogue UI callbacks
     */
    void setupDialogueCallbacks() {
        std::cout << "Setting up dialogue UI callbacks..." << std::endl;
        
        // In a real implementation:
        /*
        m_dialogueUI->setAdvanceCallback([this](const std::string& choiceId) {
            handleDialogueAdvance(choiceId);
        });
        
        m_dialogueUI->setCloseCallback([this]() {
            handleDialogueClose();
        });
        */
        
        std::cout << "   - Advance callback: handles dialogue progression and choice selection" << std::endl;
        std::cout << "   - Close callback: handles dialogue termination" << std::endl;
    }
    
    /**
     * Create a test NPC with dialogue
     */
    void createTestNPC() {
        std::cout << "Creating test NPC with dialogue..." << std::endl;
        
        // In a real implementation:
        /*
        Entity npc = m_entityManager.createEntity("ShopKeeper");
        m_npcDialogue = std::make_shared<DialogueComponent>(npc.getID());
        
        // Load dialogue from file or create programmatically
        createShopKeeperDialogue(*m_npcDialogue);
        
        // Connect dialogue component to UI
        m_dialogueUI->setDialogueComponent(m_npcDialogue);
        */
        
        std::cout << "   - NPC entity created with DialogueComponent" << std::endl;
        std::cout << "   - Dialogue tree loaded with multiple conversation paths" << std::endl;
        std::cout << "   - DialogueComponent connected to DialogueUI" << std::endl;
    }
    
    /**
     * Start dialogue with NPC
     */
    void startDialogue(const std::string& dialogueTreeId) {
        std::cout << "\nStarting dialogue: " << dialogueTreeId << std::endl;
        
        // In a real implementation:
        /*
        if (m_npcDialogue && m_dialogueUI) {
            // Start the dialogue in the component
            if (m_npcDialogue->startDialogue(dialogueTreeId)) {
                // Show the UI
                m_dialogueUI->show();
                m_dialogueActive = true;
                
                std::cout << "Dialogue UI shown, player can now interact" << std::endl;
            }
        }
        */
        
        std::cout << "   - Dialogue component started with tree: " << dialogueTreeId << std::endl;
        std::cout << "   - Dialogue UI fades in and becomes interactive" << std::endl;
        std::cout << "   - Player can now read text and make choices" << std::endl;
    }
    
    /**
     * Handle dialogue advance (called by UI callback)
     */
    void handleDialogueAdvance(const std::string& choiceId) {
        std::cout << "Handling dialogue advance..." << std::endl;
        
        if (choiceId.empty()) {
            std::cout << "   - Advancing to next dialogue node" << std::endl;
        } else {
            std::cout << "   - Player selected choice: " << choiceId << std::endl;
        }
        
        // In a real implementation:
        /*
        if (m_npcDialogue) {
            bool continued = m_npcDialogue->advanceDialogue(choiceId);
            
            if (!continued || !m_npcDialogue->isDialogueActive()) {
                // Dialogue ended
                handleDialogueClose();
            }
        }
        */
    }
    
    /**
     * Handle dialogue close (called by UI callback)
     */
    void handleDialogueClose() {
        std::cout << "Closing dialogue..." << std::endl;
        
        // In a real implementation:
        /*
        if (m_dialogueUI) {
            m_dialogueUI->hide();
        }
        
        if (m_npcDialogue) {
            m_npcDialogue->endDialogue();
        }
        
        m_dialogueActive = false;
        */
        
        std::cout << "   - Dialogue UI fades out" << std::endl;
        std::cout << "   - Player regains control" << std::endl;
        std::cout << "   - Dialogue state saved" << std::endl;
    }
    
    /**
     * Update the dialogue system
     */
    void update(float deltaTime) {
        // In a real implementation:
        /*
        if (m_dialogueUI && m_dialogueActive) {
            m_dialogueUI->onUpdate(deltaTime);
        }
        */
        
        // Handle input for starting dialogue (example)
        // if (inputManager->isActionJustActivated("interact") && nearNPC) {
        //     startDialogue("shopkeeper_main");
        // }
    }
    
    /**
     * Demonstrate dialogue features
     */
    void demonstrateFeatures() {
        std::cout << "\n=== Dialogue UI Features Demonstration ===" << std::endl;
        
        std::cout << "\n1. Text Rendering:" << std::endl;
        std::cout << "   - Supports speaker names with custom colors" << std::endl;
        std::cout << "   - Automatic text wrapping for long dialogue" << std::endl;
        std::cout << "   - Typewriter effect with configurable speed" << std::endl;
        std::cout << "   - Skip functionality for fast readers" << std::endl;
        
        std::cout << "\n2. Choice Selection:" << std::endl;
        std::cout << "   - Visual highlighting of selected choice" << std::endl;
        std::cout << "   - Keyboard navigation (Up/Down arrows)" << std::endl;
        std::cout << "   - Enter key to select choice" << std::endl;
        std::cout << "   - Conditional choices based on game state" << std::endl;
        
        std::cout << "\n3. History System:" << std::endl;
        std::cout << "   - Press H to view dialogue history" << std::endl;
        std::cout << "   - Scrollable history with Up/Down arrows" << std::endl;
        std::cout << "   - Shows previous dialogue and player choices" << std::endl;
        std::cout << "   - Automatic history management" << std::endl;
        
        std::cout << "\n4. Input Controls:" << std::endl;
        std::cout << "   - Space/Enter: Advance dialogue or skip typewriter" << std::endl;
        std::cout << "   - Up/Down: Navigate choices or scroll history" << std::endl;
        std::cout << "   - H: Toggle dialogue history" << std::endl;
        std::cout << "   - S: Skip typewriter animation" << std::endl;
        std::cout << "   - Escape: Close dialogue" << std::endl;
        
        std::cout << "\n5. Visual Features:" << std::endl;
        std::cout << "   - Smooth fade in/out animations" << std::endl;
        std::cout << "   - Customizable window styling and colors" << std::endl;
        std::cout << "   - Visual indicators for continue and choices" << std::endl;
        std::cout << "   - Responsive layout for different screen sizes" << std::endl;
        
        std::cout << "\n6. Integration Features:" << std::endl;
        std::cout << "   - Seamless integration with DialogueComponent" << std::endl;
        std::cout << "   - Callback system for game logic integration" << std::endl;
        std::cout << "   - Support for dialogue flags and variables" << std::endl;
        std::cout << "   - Automatic UI updates when dialogue changes" << std::endl;
    }
    
    /**
     * Show example dialogue tree structure
     */
    void showExampleDialogueTree() {
        std::cout << "\n=== Example Dialogue Tree Structure ===" << std::endl;
        
        std::cout << R"(
ShopKeeper Dialogue Tree:
├── greeting (Text)
│   Speaker: "ShopKeeper"
│   Text: "Welcome to my shop! How can I help you?"
│   Next: main_menu
│
├── main_menu (Choice)
│   Text: "What would you like to do?"
│   Choices:
│   ├── buy → "I'd like to buy something" → shop_menu
│   ├── sell → "I want to sell items" → sell_menu
│   ├── quest → "Do you have any work?" → quest_check
│   └── leave → "I must be going" → farewell
│
├── shop_menu (Text)
│   Speaker: "ShopKeeper"
│   Text: "Here are my finest wares!"
│   Actions: [play_sound: "shop_open"]
│   Next: back_to_menu
│
├── quest_check (Condition)
│   Conditions: [flag: "shop_quest_given" == false]
│   Next: give_quest (if true) / no_quest (if false)
│
├── give_quest (Text)
│   Speaker: "ShopKeeper"
│   Text: "I need someone to retrieve rare herbs..."
│   Actions: [set_flag: "shop_quest_given" = true,
│            start_quest: "herb_gathering"]
│   Next: back_to_menu
│
└── farewell (Text)
    Speaker: "ShopKeeper"
    Text: "Safe travels, friend!"
    Next: end
        )" << std::endl;
    }
    
    /**
     * Cleanup
     */
    void shutdown() {
        std::cout << "\nShutting down dialogue UI system..." << std::endl;
        
        // In a real implementation:
        /*
        if (m_dialogueUI) {
            m_dialogueUI->onShutdown();
        }
        */
        
        std::cout << "Dialogue UI system shut down successfully" << std::endl;
    }

private:
    // In a real implementation, these would be actual system instances:
    // std::shared_ptr<IGraphicsAPI> m_graphicsAPI;
    // std::shared_ptr<ShaderManager> m_shaderManager;
    // std::shared_ptr<SpriteRenderer> m_spriteRenderer;
    // std::shared_ptr<InputManager> m_inputManager;
    // std::unique_ptr<DialogueUI> m_dialogueUI;
    // std::shared_ptr<DialogueComponent> m_npcDialogue;
    // EntityManager m_entityManager;
    // bool m_dialogueActive = false;
};

int main() {
    DialogueUIExample example;
    
    if (example.initialize()) {
        example.demonstrateFeatures();
        example.showExampleDialogueTree();
        
        std::cout << "\n=== Simulating Dialogue Interaction ===" << std::endl;
        example.startDialogue("shopkeeper_main");
        
        // Simulate some updates
        for (int i = 0; i < 5; ++i) {
            example.update(0.016f); // ~60 FPS
        }
        
        // Simulate player making a choice
        example.handleDialogueAdvance("buy");
        
        // Simulate dialogue ending
        example.handleDialogueClose();
        
        example.shutdown();
    }
    
    std::cout << "\n=== Integration Example Complete ===" << std::endl;
    std::cout << "This example shows how to integrate DialogueUI with your game." << std::endl;
    std::cout << "See the source code for detailed implementation patterns." << std::endl;
    
    return 0;
}