#include "../src/core/EngineCore.h"
#include "../src/world/WorldManager.h"
#include "../src/tilemap/TilemapRenderer.h"
#include "../src/resources/ResourceManager.h"
#include "../src/graphics/Camera.h"
#include "../src/input/InputManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::World;
using namespace RPGEngine::Resources;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Tilemap;

int main() {
    // Create engine
    EngineCore engine;
    
    // Initialize engine
    EngineConfig config;
    config.windowTitle = "World Management Test";
    config.windowWidth = 800;
    config.windowHeight = 600;
    
    if (!engine.initialize(config)) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    // Get managers
    auto systemManager = engine.getSystemManager();
    auto entityManager = engine.getEntityManager();
    auto componentManager = engine.getComponentManager();
    auto resourceManager = std::make_shared<ResourceManager>();
    
    // Initialize resource manager
    if (!resourceManager->initialize()) {
        std::cerr << "Failed to initialize resource manager" << std::endl;
        return 1;
    }
    
    // Create camera
    auto camera = std::make_shared<Camera>();
    camera->setViewport(Viewport(0, 0, config.windowWidth, config.windowHeight));
    
    // Create world manager
    auto worldManager = std::make_shared<WorldManager>(resourceManager, entityManager, componentManager);
    worldManager->setCamera(camera);
    worldManager->setMapDirectory("assets/maps/");
    
    // Create tilemap renderer
    auto tilemapRenderer = std::make_shared<TilemapRenderer>(engine.getGraphicsAPI());
    tilemapRenderer->setCamera(camera);
    
    // Add systems to system manager
    systemManager->addSystem(worldManager);
    systemManager->addSystem(tilemapRenderer);
    
    // Initialize systems
    if (!worldManager->initialize()) {
        std::cerr << "Failed to initialize world manager" << std::endl;
        return 1;
    }
    
    if (!tilemapRenderer->initialize()) {
        std::cerr << "Failed to initialize tilemap renderer" << std::endl;
        return 1;
    }
    
    // Register map loaded callback
    worldManager->registerMapLoadedCallback([&tilemapRenderer, &worldManager](const MapLoadedEvent& event) {
        std::cout << "Map loaded: " << event.mapName << " (ID: " << event.mapId << ")" << std::endl;
        
        // Set tilemap for renderer
        auto map = worldManager->getMap(event.mapId);
        if (map) {
            tilemapRenderer->setTilemap(map->getTilemap());
        }
    });
    
    // Register map transition callback
    worldManager->registerTransitionCallback([](const MapTransitionEvent& event) {
        std::cout << "Map transition: " << event.fromMapId << " -> " << event.toMapId;
        if (!event.portalName.empty()) {
            std::cout << " (portal: " << event.portalName << ")";
        }
        std::cout << std::endl;
    });
    
    // Load maps
    auto map1 = worldManager->loadMap("test_map.tmx", 1);
    auto map2 = worldManager->loadMap("test_map.tmx", 2); // Using the same map file for simplicity
    
    if (!map1 || !map2) {
        std::cerr << "Failed to load maps" << std::endl;
        return 1;
    }
    
    // Set active map
    worldManager->setActiveMap(1);
    
    // Print instructions
    std::cout << "World Management Test" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Q/E - Zoom in/out" << std::endl;
    std::cout << "  1/2 - Switch between maps" << std::endl;
    std::cout << "  T - Transition between maps" << std::endl;
    std::cout << "  C - Toggle collider rendering" << std::endl;
    
    // Main loop
    bool running = true;
    while (running) {
        // Process input
        auto& inputManager = InputManager::getInstance();
        
        // Camera movement
        float cameraSpeed = 200.0f * engine.getDeltaTime();
        if (inputManager.isKeyPressed(KeyCode::W)) {
            camera->move(0.0f, -cameraSpeed);
        }
        if (inputManager.isKeyPressed(KeyCode::S)) {
            camera->move(0.0f, cameraSpeed);
        }
        if (inputManager.isKeyPressed(KeyCode::A)) {
            camera->move(-cameraSpeed, 0.0f);
        }
        if (inputManager.isKeyPressed(KeyCode::D)) {
            camera->move(cameraSpeed, 0.0f);
        }
        
        // Camera zoom
        if (inputManager.isKeyPressed(KeyCode::Q)) {
            camera->setZoom(camera->getZoom() * 1.01f);
        }
        if (inputManager.isKeyPressed(KeyCode::E)) {
            camera->setZoom(camera->getZoom() * 0.99f);
        }
        
        // Switch maps
        if (inputManager.isKeyJustPressed(KeyCode::_1)) {
            worldManager->setActiveMap(1);
        }
        if (inputManager.isKeyJustPressed(KeyCode::_2)) {
            worldManager->setActiveMap(2);
        }
        
        // Transition between maps
        if (inputManager.isKeyJustPressed(KeyCode::T)) {
            auto activeMap = worldManager->getActiveMap();
            if (activeMap) {
                uint32_t currentMapId = activeMap->getId();
                uint32_t nextMapId = (currentMapId == 1) ? 2 : 1;
                worldManager->transitionToMap(nextMapId, "spawn", 1.0f);
            }
        }
        
        // Toggle collider rendering
        if (inputManager.isKeyJustPressed(KeyCode::C)) {
            tilemapRenderer->setRenderColliders(!tilemapRenderer->isRenderingColliders());
            std::cout << "Collider rendering: " << (tilemapRenderer->isRenderingColliders() ? "ON" : "OFF") << std::endl;
        }
        
        // Update engine
        if (!engine.update()) {
            running = false;
        }
    }
    
    // Shutdown systems
    worldManager->shutdown();
    tilemapRenderer->shutdown();
    
    // Shutdown resource manager
    resourceManager->shutdown();
    
    // Shutdown engine
    engine.shutdown();
    
    return 0;
}