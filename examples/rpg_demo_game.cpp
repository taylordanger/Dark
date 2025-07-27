#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Core Engine
#include "../src/core/EngineCore.h"
#include "../src/core/ConfigurationManager.h"

// Systems
#include "../src/systems/SystemManager.h"
#include "../src/systems/MovementSystem.h"
#include "../src/systems/CombatSystem.h"
#include "../src/systems/QuestSystem.h"
#include "../src/systems/ScriptSystem.h"
#include "../src/physics/CollisionSystem.h"
#include "../src/physics/TriggerSystem.h"
#include "../src/graphics/CameraSystem.h"

// Components
#include "../src/components/ComponentManager.h"
#include "../src/components/StatsComponent.h"
#include "../src/components/InventoryComponent.h"
#include "../src/components/DialogueComponent.h"
#include "../src/components/QuestComponent.h"
#include "../src/components/CombatComponent.h"
#include "../src/components/PhysicsComponent.h"
#include "../src/components/TriggerComponent.h"

// Graphics & Rendering
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/graphics/Camera.h"
#include "../src/graphics/AnimationComponent.h"
#include "../src/tilemap/TilemapRenderer.h"
#include "../src/tilemap/MapLoader.h"

// Audio
#include "../src/audio/AudioManager.h"
#include "../src/audio/MusicManager.h"
#include "../src/audio/SoundEffectManager.h"

// Input
#include "../src/input/InputManager.h"
#include "../src/input/KeyboardDevice.h"
#include "../src/input/MouseDevice.h"
#include "../src/input/GamepadDevice.h"

// World & Maps
#include "../src/world/WorldManager.h"
#include "../src/world/Map.h"

// UI
#include "../src/ui/UIRenderer.h"
#include "../src/ui/GameHUD.h"
#include "../src/ui/DialogueUI.h"
#include "../src/ui/QuestUI.h"
#include "../src/ui/CombatUI.h"
#include "../src/ui/SaveLoadUI.h"

// Scene Management
#include "../src/scene/SceneManager.h"
#include "../src/scene/GameScene.h"
#include "../src/scene/MenuScene.h"
#include "../src/scene/BattleScene.h"

// Save System
#include "../src/save/SaveManager.h"
#include "../src/save/SaveLoadManager.h"

// Resources
#include "../src/resources/ResourceManager.h"

// Debug Tools
#include "../src/debug/DebugRenderer.h"
#include "../src/debug/PerformanceProfiler.h"
#include "../src/debug/EntityInspector.h"

// Entities
#include "../src/entities/EntityManager.h"
#include "../src/entities/EntityFactory.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Components;
using namespace RPGEngine::Systems;

/**
 * Comprehensive RPG Demo Game
 * 
 * This demo showcases ALL major engine features:
 * - Entity-Component-System architecture
 * - Graphics rendering with sprites and animations
 * - Tilemap rendering and world management
 * - Input handling (keyboard, mouse, gamepad)
 * - Audio system (music and sound effects)
 * - Physics and collision detection
 * - RPG systems (stats, inventory, dialogue, quests, combat)
 * - Scene management
 * - Save/load system
 * - UI system
 * - Scripting integration
 * - Debug tools
 * - Cross-platform support
 */
class RPGDemoGame {
private:
    // Core Engine Components
    std::shared_ptr<EngineCore> engine;
    std::shared_ptr<SystemManager> systemManager;
    std::shared_ptr<EntityManager> entityManager;
    std::shared_ptr<ComponentManager> componentManager;
    std::shared_ptr<ConfigurationManager> configManager;
    
    // Graphics & Rendering
    std::shared_ptr<OpenGLAPI> graphicsAPI;
    std::shared_ptr<SpriteRenderer> spriteRenderer;
    std::shared_ptr<TilemapRenderer> tilemapRenderer;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<CameraSystem> cameraSystem;
    
    // Input System
    std::shared_ptr<InputManager> inputManager;
    std::shared_ptr<KeyboardDevice> keyboard;
    std::shared_ptr<MouseDevice> mouse;
    std::shared_ptr<GamepadDevice> gamepad;
    
    // Audio System
    std::shared_ptr<AudioManager> audioManager;
    std::shared_ptr<MusicManager> musicManager;
    std::shared_ptr<SoundEffectManager> soundManager;
    
    // World & Maps
    std::shared_ptr<WorldManager> worldManager;
    std::shared_ptr<MapLoader> mapLoader;
    
    // Game Systems
    std::shared_ptr<MovementSystem> movementSystem;
    std::shared_ptr<CollisionSystem> collisionSystem;
    std::shared_ptr<TriggerSystem> triggerSystem;
    std::shared_ptr<CombatSystem> combatSystem;
    std::shared_ptr<QuestSystem> questSystem;
    std::shared_ptr<ScriptSystem> scriptSystem;
    
    // UI System
    std::shared_ptr<UIRenderer> uiRenderer;
    std::shared_ptr<GameHUD> gameHUD;
    std::shared_ptr<DialogueUI> dialogueUI;
    std::shared_ptr<QuestUI> questUI;
    std::shared_ptr<CombatUI> combatUI;
    std::shared_ptr<SaveLoadUI> saveLoadUI;
    
    // Scene Management
    std::shared_ptr<SceneManager> sceneManager;
    std::shared_ptr<GameScene> gameScene;
    std::shared_ptr<MenuScene> menuScene;
    std::shared_ptr<BattleScene> battleScene;
    
    // Save System
    std::shared_ptr<SaveManager> saveManager;
    std::shared_ptr<SaveLoadManager> saveLoadManager;
    
    // Resources
    std::shared_ptr<ResourceManager> resourceManager;
    
    // Debug Tools
    std::shared_ptr<DebugRenderer> debugRenderer;
    std::shared_ptr<PerformanceProfiler> profiler;
    std::shared_ptr<EntityInspector> entityInspector;
    
    // Game Entities
    Entity playerEntity;
    std::vector<Entity> npcEntities;
    std::vector<Entity> itemEntities;
    std::vector<Entity> enemyEntities;
    
    // Game State
    bool isRunning;
    bool showDebugInfo;
    bool inCombat;
    bool inDialogue;
    float gameTime;
    
public:
    RPGDemoGame() : isRunning(false), showDebugInfo(false), inCombat(false), 
                    inDialogue(false), gameTime(0.0f) {}
    
    bool initialize() {
        std::cout << "=== RPG Engine Demo Game ===" << std::endl;
        std::cout << "Initializing comprehensive RPG demo..." << std::endl;
        
        // Initialize core engine
        if (!initializeCore()) {
            std::cerr << "Failed to initialize core engine" << std::endl;
            return false;
        }
        
        // Initialize graphics
        if (!initializeGraphics()) {
            std::cerr << "Failed to initialize graphics" << std::endl;
            return false;
        }
        
        // Initialize input
        if (!initializeInput()) {
            std::cerr << "Failed to initialize input" << std::endl;
            return false;
        }
        
        // Initialize audio
        if (!initializeAudio()) {
            std::cerr << "Failed to initialize audio" << std::endl;
            return false;
        }
        
        // Initialize game systems
        if (!initializeSystems()) {
            std::cerr << "Failed to initialize game systems" << std::endl;
            return false;
        }
        
        // Initialize UI
        if (!initializeUI()) {
            std::cerr << "Failed to initialize UI" << std::endl;
            return false;
        }
        
        // Initialize scenes
        if (!initializeScenes()) {
            std::cerr << "Failed to initialize scenes" << std::endl;
            return false;
        }
        
        // Create demo world
        if (!createDemoWorld()) {
            std::cerr << "Failed to create demo world" << std::endl;
            return false;
        }
        
        std::cout << "✅ RPG Demo Game initialized successfully!" << std::endl;
        return true;
    }
    
    void run() {
        isRunning = true;
        
        std::cout << "\n🎮 Starting RPG Demo Game..." << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD - Move player" << std::endl;
        std::cout << "  SPACE - Interact/Attack" << std::endl;
        std::cout << "  E - Open inventory" << std::endl;
        std::cout << "  Q - Open quest log" << std::endl;
        std::cout << "  TAB - Toggle debug info" << std::endl;
        std::cout << "  ESC - Pause menu" << std::endl;
        std::cout << "  F5 - Quick save" << std::endl;
        std::cout << "  F9 - Quick load" << std::endl;
        
        // Start with menu scene
        sceneManager->switchToScene("menu");
        
        // Main game loop
        while (isRunning && !graphicsAPI->shouldClose()) {
            float deltaTime = calculateDeltaTime();
            gameTime += deltaTime;
            
            // Handle input
            handleInput();
            
            // Update current scene
            sceneManager->update(deltaTime);
            
            // Update game systems
            updateSystems(deltaTime);
            
            // Render frame
            render();
            
            // Update profiler
            if (profiler) {
                profiler->endFrame();
            }
            
            // Poll events
            graphicsAPI->pollEvents();
        }
        
        std::cout << "🎮 RPG Demo Game ended." << std::endl;
    }
    
    void shutdown() {
        std::cout << "Shutting down RPG Demo Game..." << std::endl;
        
        // Save final game state
        if (saveManager) {
            saveManager->quickSave();
        }
        
        // Shutdown systems in reverse order
        if (sceneManager) sceneManager.reset();
        if (audioManager) audioManager->shutdown();
        if (graphicsAPI) graphicsAPI->shutdown();
        if (engine) engine->shutdown();
        
        std::cout << "✅ RPG Demo Game shutdown complete." << std::endl;
    }

private:
    bool initializeCore() {
        // Configuration
        configManager = std::make_shared<ConfigurationManager>();
        configManager->loadFromFile("config/game_config.json");
        
        // Core engine
        engine = std::make_shared<EngineCore>();
        if (!engine->initialize()) {
            return false;
        }
        
        // Entity and component management
        entityManager = std::make_shared<EntityManager>();
        componentManager = std::make_shared<ComponentManager>();
        
        // System manager
        systemManager = std::make_shared<SystemManager>();
        
        // Resource manager
        resourceManager = std::make_shared<ResourceManager>();
        
        return true;
    }
    
    bool initializeGraphics() {
        // Graphics API
        graphicsAPI = std::make_shared<OpenGLAPI>();
        if (!graphicsAPI->initialize(1024, 768, "RPG Engine Demo", false)) {
            return false;
        }
        
        // Renderers
        spriteRenderer = std::make_shared<SpriteRenderer>();
        spriteRenderer->initialize(graphicsAPI);
        
        tilemapRenderer = std::make_shared<TilemapRenderer>();
        tilemapRenderer->initialize(graphicsAPI);
        
        // Camera
        camera = std::make_shared<Camera>();
        camera->setViewportSize(1024, 768);
        camera->setPosition(0, 0);
        
        cameraSystem = std::make_shared<CameraSystem>(camera);
        systemManager->addSystem(cameraSystem);
        
        // Debug renderer
        debugRenderer = std::make_shared<DebugRenderer>();
        debugRenderer->initialize(graphicsAPI);
        
        return true;
    }
    
    bool initializeInput() {
        inputManager = std::make_shared<InputManager>();
        
        // Input devices
        keyboard = std::make_shared<KeyboardDevice>();
        mouse = std::make_shared<MouseDevice>();
        gamepad = std::make_shared<GamepadDevice>();
        
        inputManager->addDevice(keyboard);
        inputManager->addDevice(mouse);
        inputManager->addDevice(gamepad);
        
        return inputManager->initialize();
    }
    
    bool initializeAudio() {
        audioManager = std::make_shared<AudioManager>();
        if (!audioManager->initialize()) {
            return false;
        }
        
        musicManager = std::make_shared<MusicManager>(audioManager);
        soundManager = std::make_shared<SoundEffectManager>(audioManager);
        
        // Load demo audio
        musicManager->loadMusic("background", "assets/audio/background_music.ogg");
        soundManager->loadSound("footstep", "assets/audio/footstep.wav");
        soundManager->loadSound("sword_hit", "assets/audio/sword_hit.wav");
        soundManager->loadSound("item_pickup", "assets/audio/item_pickup.wav");
        
        // Start background music
        musicManager->playMusic("background", true);
        
        return true;
    }
    
    bool initializeSystems() {
        // Physics systems
        collisionSystem = std::make_shared<CollisionSystem>(componentManager);
        triggerSystem = std::make_shared<TriggerSystem>(componentManager);
        movementSystem = std::make_shared<MovementSystem>(componentManager, collisionSystem);
        
        // Game systems
        combatSystem = std::make_shared<CombatSystem>(componentManager);
        questSystem = std::make_shared<QuestSystem>(componentManager);
        scriptSystem = std::make_shared<ScriptSystem>();
        
        // Add systems to manager
        systemManager->addSystem(movementSystem);
        systemManager->addSystem(collisionSystem);
        systemManager->addSystem(triggerSystem);
        systemManager->addSystem(combatSystem);
        systemManager->addSystem(questSystem);
        systemManager->addSystem(scriptSystem);
        
        // World management
        worldManager = std::make_shared<WorldManager>();
        mapLoader = std::make_shared<MapLoader>();
        
        // Save system
        saveManager = std::make_shared<SaveManager>();
        saveLoadManager = std::make_shared<SaveLoadManager>(saveManager);
        
        // Debug tools
        profiler = std::make_shared<PerformanceProfiler>();
        entityInspector = std::make_shared<EntityInspector>(entityManager, componentManager);
        
        return true;
    }
    
    bool initializeUI() {
        uiRenderer = std::make_shared<UIRenderer>();
        if (!uiRenderer->initialize(graphicsAPI)) {
            return false;
        }
        
        // Game UI components
        gameHUD = std::make_shared<GameHUD>(uiRenderer);
        dialogueUI = std::make_shared<DialogueUI>(uiRenderer);
        questUI = std::make_shared<QuestUI>(uiRenderer);
        combatUI = std::make_shared<CombatUI>(uiRenderer);
        saveLoadUI = std::make_shared<SaveLoadUI>(uiRenderer, saveLoadManager);
        
        return true;
    }
    
    bool initializeScenes() {
        sceneManager = std::make_shared<SceneManager>();
        
        // Create scenes
        menuScene = std::make_shared<MenuScene>(entityManager, componentManager);
        gameScene = std::make_shared<GameScene>(entityManager, componentManager);
        battleScene = std::make_shared<BattleScene>(entityManager, componentManager);
        
        // Add scenes to manager
        sceneManager->addScene("menu", menuScene);
        sceneManager->addScene("game", gameScene);
        sceneManager->addScene("battle", battleScene);
        
        return true;
    }
    
    bool createDemoWorld() {
        std::cout << "Creating demo world..." << std::endl;
        
        // Create player
        createPlayer();
        
        // Create NPCs
        createNPCs();
        
        // Create items
        createItems();
        
        // Create enemies
        createEnemies();
        
        // Load demo map
        loadDemoMap();
        
        // Setup demo quests
        setupDemoQuests();
        
        std::cout << "✅ Demo world created successfully!" << std::endl;
        return true;
    }
    
    void createPlayer() {
        playerEntity = entityManager->createEntity();
        
        // Stats component
        auto stats = std::make_shared<StatsComponent>(playerEntity);
        stats->setLevel(1);
        stats->setExperience(0);
        stats->setBaseMaxHP(100);
        stats->setCurrentHP(100);
        stats->setBaseMaxMP(50);
        stats->setCurrentMP(50);
        componentManager->addComponent(playerEntity, stats);
        
        // Inventory component
        auto inventory = std::make_shared<InventoryComponent>(playerEntity);
        inventory->setCapacity(20);
        componentManager->addComponent(playerEntity, inventory);
        
        // Physics component for movement
        auto physics = std::make_shared<PhysicsComponent>(playerEntity);
        physics->setPosition(100, 100);
        physics->setVelocity(0, 0);
        componentManager->addComponent(playerEntity, physics);
        
        // Combat component
        auto combat = std::make_shared<CombatComponent>(playerEntity);
        combat->setAttackPower(15);
        combat->setDefense(5);
        componentManager->addComponent(playerEntity, combat);
        
        // Animation component
        auto animation = std::make_shared<AnimationComponent>(playerEntity);
        animation->addAnimation("idle", {"player_idle_1.png", "player_idle_2.png"}, 0.5f);
        animation->addAnimation("walk", {"player_walk_1.png", "player_walk_2.png", "player_walk_3.png"}, 0.2f);
        animation->playAnimation("idle", true);
        componentManager->addComponent(playerEntity, animation);
        
        std::cout << "  ✅ Player created with stats, inventory, and combat abilities" << std::endl;
    }
    
    void createNPCs() {
        // Create village elder NPC
        Entity elderNPC = entityManager->createEntity();
        
        auto elderPhysics = std::make_shared<PhysicsComponent>(elderNPC);
        elderPhysics->setPosition(200, 150);
        componentManager->addComponent(elderNPC, elderPhysics);
        
        auto elderDialogue = std::make_shared<DialogueComponent>(elderNPC);
        elderDialogue->loadFromFile("assets/dialogue/village_elder.json");
        componentManager->addComponent(elderNPC, elderDialogue);
        
        // Create merchant NPC
        Entity merchantNPC = entityManager->createEntity();
        
        auto merchantPhysics = std::make_shared<PhysicsComponent>(merchantNPC);
        merchantPhysics->setPosition(300, 200);
        componentManager->addComponent(merchantNPC, merchantPhysics);
        
        auto merchantInventory = std::make_shared<InventoryComponent>(merchantNPC);
        merchantInventory->setCapacity(50);
        // Add merchant items
        componentManager->addComponent(merchantNPC, merchantInventory);
        
        npcEntities.push_back(elderNPC);
        npcEntities.push_back(merchantNPC);
        
        std::cout << "  ✅ Created " << npcEntities.size() << " NPCs with dialogue and trading" << std::endl;
    }
    
    void createItems() {
        // Create health potion
        Entity healthPotion = entityManager->createEntity();
        
        auto potionPhysics = std::make_shared<PhysicsComponent>(healthPotion);
        potionPhysics->setPosition(250, 180);
        componentManager->addComponent(healthPotion, potionPhysics);
        
        // Create trigger for pickup
        auto potionTrigger = std::make_shared<TriggerComponent>(healthPotion);
        potionTrigger->setTriggerType("item_pickup");
        potionTrigger->setTriggerData("health_potion");
        componentManager->addComponent(healthPotion, potionTrigger);
        
        // Create sword item
        Entity sword = entityManager->createEntity();
        
        auto swordPhysics = std::make_shared<PhysicsComponent>(sword);
        swordPhysics->setPosition(320, 220);
        componentManager->addComponent(sword, swordPhysics);
        
        auto swordTrigger = std::make_shared<TriggerComponent>(sword);
        swordTrigger->setTriggerType("item_pickup");
        swordTrigger->setTriggerData("iron_sword");
        componentManager->addComponent(sword, swordTrigger);
        
        itemEntities.push_back(healthPotion);
        itemEntities.push_back(sword);
        
        std::cout << "  ✅ Created " << itemEntities.size() << " interactive items" << std::endl;
    }
    
    void createEnemies() {
        // Create goblin enemy
        Entity goblin = entityManager->createEntity();
        
        auto goblinStats = std::make_shared<StatsComponent>(goblin);
        goblinStats->setLevel(1);
        goblinStats->setBaseMaxHP(30);
        goblinStats->setCurrentHP(30);
        componentManager->addComponent(goblin, goblinStats);
        
        auto goblinPhysics = std::make_shared<PhysicsComponent>(goblin);
        goblinPhysics->setPosition(400, 300);
        componentManager->addComponent(goblin, goblinPhysics);
        
        auto goblinCombat = std::make_shared<CombatComponent>(goblin);
        goblinCombat->setAttackPower(8);
        goblinCombat->setDefense(2);
        componentManager->addComponent(goblin, goblinCombat);
        
        // Create orc enemy
        Entity orc = entityManager->createEntity();
        
        auto orcStats = std::make_shared<StatsComponent>(orc);
        orcStats->setLevel(2);
        orcStats->setBaseMaxHP(50);
        orcStats->setCurrentHP(50);
        componentManager->addComponent(orc, orcStats);
        
        auto orcPhysics = std::make_shared<PhysicsComponent>(orc);
        orcPhysics->setPosition(450, 350);
        componentManager->addComponent(orc, orcPhysics);
        
        auto orcCombat = std::make_shared<CombatComponent>(orc);
        orcCombat->setAttackPower(12);
        orcCombat->setDefense(4);
        componentManager->addComponent(orc, orcCombat);
        
        enemyEntities.push_back(goblin);
        enemyEntities.push_back(orc);
        
        std::cout << "  ✅ Created " << enemyEntities.size() << " enemies with combat stats" << std::endl;
    }
    
    void loadDemoMap() {
        // Load tilemap
        if (mapLoader->loadMap("assets/maps/demo_village.tmx")) {
            auto map = mapLoader->getCurrentMap();
            worldManager->setCurrentMap(map);
            std::cout << "  ✅ Loaded demo village map" << std::endl;
        } else {
            std::cout << "  ⚠️  Using procedural demo map" << std::endl;
            // Create simple procedural map as fallback
        }
    }
    
    void setupDemoQuests() {
        // Create main quest
        Entity mainQuest = entityManager->createEntity();
        
        auto questComponent = std::make_shared<QuestComponent>(mainQuest);
        questComponent->setTitle("Defend the Village");
        questComponent->setDescription("Help the village elder defend against the goblin threat.");
        questComponent->addObjective("Talk to the village elder");
        questComponent->addObjective("Defeat 2 goblins");
        questComponent->addObjective("Return to the elder");
        questComponent->setRewardExperience(100);
        componentManager->addComponent(mainQuest, questComponent);
        
        // Activate the quest
        questSystem->activateQuest(mainQuest);
        
        std::cout << "  ✅ Setup demo quest system" << std::endl;
    }
    
    float calculateDeltaTime() {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        return std::min(deltaTime, 0.016f); // Cap at 60 FPS
    }
    
    void handleInput() {
        inputManager->update();
        
        // Toggle debug info
        if (keyboard->isKeyPressed(GLFW_KEY_TAB)) {
            showDebugInfo = !showDebugInfo;
        }
        
        // Quick save/load
        if (keyboard->isKeyPressed(GLFW_KEY_F5)) {
            saveManager->quickSave();
            std::cout << "Game saved!" << std::endl;
        }
        
        if (keyboard->isKeyPressed(GLFW_KEY_F9)) {
            saveManager->quickLoad();
            std::cout << "Game loaded!" << std::endl;
        }
        
        // Scene switching
        if (keyboard->isKeyPressed(GLFW_KEY_ESCAPE)) {
            if (sceneManager->getCurrentSceneName() == "game") {
                sceneManager->switchToScene("menu");
            } else if (sceneManager->getCurrentSceneName() == "menu") {
                sceneManager->switchToScene("game");
            }
        }
        
        // Player movement (only in game scene)
        if (sceneManager->getCurrentSceneName() == "game") {
            handlePlayerMovement();
        }
    }
    
    void handlePlayerMovement() {
        auto playerPhysics = componentManager->getComponent<PhysicsComponent>(playerEntity);
        if (!playerPhysics) return;
        
        float moveSpeed = 100.0f;
        float vx = 0, vy = 0;
        
        if (keyboard->isKeyDown(GLFW_KEY_W) || keyboard->isKeyDown(GLFW_KEY_UP)) {
            vy = -moveSpeed;
        }
        if (keyboard->isKeyDown(GLFW_KEY_S) || keyboard->isKeyDown(GLFW_KEY_DOWN)) {
            vy = moveSpeed;
        }
        if (keyboard->isKeyDown(GLFW_KEY_A) || keyboard->isKeyDown(GLFW_KEY_LEFT)) {
            vx = -moveSpeed;
        }
        if (keyboard->isKeyDown(GLFW_KEY_D) || keyboard->isKeyDown(GLFW_KEY_RIGHT)) {
            vx = moveSpeed;
        }
        
        playerPhysics->setVelocity(vx, vy);
        
        // Update player animation
        auto playerAnimation = componentManager->getComponent<AnimationComponent>(playerEntity);
        if (playerAnimation) {
            if (vx != 0 || vy != 0) {
                playerAnimation->playAnimation("walk", true);
                soundManager->playSound("footstep");
            } else {
                playerAnimation->playAnimation("idle", true);
            }
        }
        
        // Update camera to follow player
        float px, py;
        playerPhysics->getPosition(px, py);
        camera->setPosition(px, py);
        
        // Interaction
        if (keyboard->isKeyPressed(GLFW_KEY_SPACE)) {
            handlePlayerInteraction();
        }
        
        // UI toggles
        if (keyboard->isKeyPressed(GLFW_KEY_E)) {
            // Toggle inventory UI
        }
        
        if (keyboard->isKeyPressed(GLFW_KEY_Q)) {
            // Toggle quest UI
        }
    }
    
    void handlePlayerInteraction() {
        // Check for nearby NPCs, items, or enemies
        auto playerPhysics = componentManager->getComponent<PhysicsComponent>(playerEntity);
        if (!playerPhysics) return;
        
        float px, py;
        playerPhysics->getPosition(px, py);
        
        // Check for NPC interactions
        for (Entity npc : npcEntities) {
            auto npcPhysics = componentManager->getComponent<PhysicsComponent>(npc);
            if (!npcPhysics) continue;
            
            float nx, ny;
            npcPhysics->getPosition(nx, ny);
            
            float distance = sqrt((px - nx) * (px - nx) + (py - ny) * (py - ny));
            if (distance < 50.0f) {
                // Start dialogue
                auto dialogue = componentManager->getComponent<DialogueComponent>(npc);
                if (dialogue) {
                    inDialogue = true;
                    dialogueUI->startDialogue(dialogue);
                    std::cout << "Started dialogue with NPC" << std::endl;
                }
                return;
            }
        }
        
        // Check for combat
        for (Entity enemy : enemyEntities) {
            auto enemyPhysics = componentManager->getComponent<PhysicsComponent>(enemy);
            if (!enemyPhysics) continue;
            
            float ex, ey;
            enemyPhysics->getPosition(ex, ey);
            
            float distance = sqrt((px - ex) * (px - ex) + (py - ey) * (py - ey));
            if (distance < 60.0f) {
                // Start combat
                inCombat = true;
                sceneManager->switchToScene("battle");
                combatSystem->startCombat(playerEntity, enemy);
                soundManager->playSound("sword_hit");
                std::cout << "Combat started!" << std::endl;
                return;
            }
        }
    }
    
    void updateSystems(float deltaTime) {
        if (profiler) {
            profiler->beginFrame();
        }
        
        // Update all systems
        systemManager->update(deltaTime);
        
        // Update audio
        if (audioManager) {
            audioManager->update(deltaTime);
        }
        
        // Update UI based on game state
        updateUI(deltaTime);
    }
    
    void updateUI(float deltaTime) {
        // Update HUD with player stats
        auto playerStats = componentManager->getComponent<StatsComponent>(playerEntity);
        if (playerStats && gameHUD) {
            gameHUD->updatePlayerStats(
                playerStats->getCurrentHP(),
                playerStats->getMaxHP(),
                playerStats->getCurrentMP(),
                playerStats->getMaxMP(),
                playerStats->getLevel(),
                playerStats->getExperience()
            );
        }
        
        // Update dialogue UI
        if (inDialogue && dialogueUI) {
            dialogueUI->update(deltaTime);
            if (dialogueUI->isDialogueComplete()) {
                inDialogue = false;
            }
        }
        
        // Update combat UI
        if (inCombat && combatUI) {
            combatUI->update(deltaTime);
            if (combatSystem->isCombatComplete()) {
                inCombat = false;
                sceneManager->switchToScene("game");
            }
        }
        
        // Update quest UI
        if (questUI) {
            questUI->update(deltaTime);
        }
    }
    
    void render() {
        graphicsAPI->beginFrame();
        graphicsAPI->clear(0.2f, 0.3f, 0.8f, 1.0f);
        
        // Set camera for world rendering
        camera->updateMatrices();
        
        // Render tilemap
        if (tilemapRenderer && worldManager->getCurrentMap()) {
            tilemapRenderer->render(worldManager->getCurrentMap(), camera);
        }
        
        // Render entities
        renderEntities();
        
        // Render UI
        renderUI();
        
        // Render debug info
        if (showDebugInfo) {
            renderDebugInfo();
        }
        
        graphicsAPI->endFrame();
    }
    
    void renderEntities() {
        // Render player
        renderEntity(playerEntity);
        
        // Render NPCs
        for (Entity npc : npcEntities) {
            renderEntity(npc);
        }
        
        // Render items
        for (Entity item : itemEntities) {
            renderEntity(item);
        }
        
        // Render enemies
        for (Entity enemy : enemyEntities) {
            renderEntity(enemy);
        }
    }
    
    void renderEntity(Entity entity) {
        auto physics = componentManager->getComponent<PhysicsComponent>(entity);
        auto animation = componentManager->getComponent<AnimationComponent>(entity);
        
        if (!physics) return;
        
        float x, y;
        physics->getPosition(x, y);
        
        if (animation && spriteRenderer) {
            // Render animated sprite
            auto currentFrame = animation->getCurrentFrame();
            if (!currentFrame.empty()) {
                spriteRenderer->renderSprite(currentFrame, x, y, 32, 32, camera);
            }
        } else if (spriteRenderer) {
            // Render default sprite based on entity type
            std::string spriteName = "default.png";
            
            // Determine sprite based on components
            if (componentManager->hasComponent<CombatComponent>(entity)) {
                if (entity == playerEntity) {
                    spriteName = "player.png";
                } else {
                    spriteName = "enemy.png";
                }
            } else if (componentManager->hasComponent<DialogueComponent>(entity)) {
                spriteName = "npc.png";
            } else if (componentManager->hasComponent<TriggerComponent>(entity)) {
                spriteName = "item.png";
            }
            
            spriteRenderer->renderSprite(spriteName, x, y, 32, 32, camera);
        }
    }
    
    void renderUI() {
        if (!uiRenderer) return;
        
        // Render HUD
        if (gameHUD && sceneManager->getCurrentSceneName() == "game") {
            gameHUD->render();
        }
        
        // Render dialogue UI
        if (inDialogue && dialogueUI) {
            dialogueUI->render();
        }
        
        // Render combat UI
        if (inCombat && combatUI) {
            combatUI->render();
        }
        
        // Render quest UI
        if (questUI && keyboard->isKeyDown(GLFW_KEY_Q)) {
            questUI->render();
        }
        
        // Render current scene UI
        sceneManager->render();
    }
    
    void renderDebugInfo() {
        if (!debugRenderer) return;
        
        // Render collision shapes
        for (Entity entity : {playerEntity}) {
            auto physics = componentManager->getComponent<PhysicsComponent>(entity);
            if (physics) {
                float x, y;
                physics->getPosition(x, y);
                debugRenderer->drawRectangle(x - 16, y - 16, 32, 32, 0.0f, 1.0f, 0.0f, 0.5f);
            }
        }
        
        // Render performance info
        if (profiler) {
            auto stats = profiler->getFrameStats();
            debugRenderer->drawText(10, 10, "FPS: " + std::to_string(stats.fps));
            debugRenderer->drawText(10, 30, "Frame Time: " + std::to_string(stats.frameTime) + "ms");
            debugRenderer->drawText(10, 50, "Entities: " + std::to_string(entityManager->getEntityCount()));
        }
        
        // Render entity inspector
        if (entityInspector) {
            entityInspector->render(debugRenderer);
        }
    }
};

int main() {
    RPGDemoGame game;
    
    if (!game.initialize()) {
        std::cerr << "Failed to initialize RPG Demo Game" << std::endl;
        return -1;
    }
    
    game.run();
    game.shutdown();
    
    return 0;
}