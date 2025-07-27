#include "../src/core/EngineCore.h"
#include "../src/tilemap/Tilemap.h"
#include "../src/tilemap/TilemapRenderer.h"
#include "../src/resources/ResourceManager.h"
#include "../src/resources/TextureResource.h"
#include "../src/graphics/Camera.h"
#include "../src/input/InputManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Tilemap;
using namespace RPGEngine::Resources;
using namespace RPGEngine::Graphics;

int main() {
    // Create engine
    EngineCore engine;
    
    // Initialize engine
    EngineConfig config;
    config.windowTitle = "Tilemap Test";
    config.windowWidth = 800;
    config.windowHeight = 600;
    
    if (!engine.initialize(config)) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    // Get managers
    auto systemManager = engine.getSystemManager();
    auto resourceManager = std::make_shared<ResourceManager>();
    
    // Initialize resource manager
    if (!resourceManager->initialize()) {
        std::cerr << "Failed to initialize resource manager" << std::endl;
        return 1;
    }
    
    // Create tilemap
    MapProperties mapProps;
    mapProps.name = "Test Map";
    mapProps.orientation = MapOrientation::Orthogonal;
    mapProps.width = 20;
    mapProps.height = 15;
    mapProps.tileWidth = 32;
    mapProps.tileHeight = 32;
    
    auto tilemap = std::make_shared<Tilemap>(mapProps);
    
    // Create tileset
    auto tileset = std::make_shared<Tileset>("Test Tileset", 32, 32);
    
    // Create texture resource
    auto texture = std::make_shared<TextureResource>("tileset", "assets/textures/tileset.png");
    resourceManager->addResource(texture);
    resourceManager->loadResource("tileset");
    
    // Set tileset texture
    tileset->setTexture(texture);
    
    // Add tileset to tilemap
    tilemap->addTileset(tileset);
    
    // Create tile layer
    auto layer = std::make_shared<TileLayer>(mapProps.width, mapProps.height);
    LayerProperties layerProps;
    layerProps.name = "Ground";
    layer->setProperties(layerProps);
    
    // Fill layer with tiles
    for (int y = 0; y < layer->getHeight(); ++y) {
        for (int x = 0; x < layer->getWidth(); ++x) {
            // Create checkerboard pattern
            uint32_t tileId = ((x + y) % 2 == 0) ? 1 : 2;
            
            // Add solid flag to some tiles
            uint32_t flags = TileFlags::None;
            if (x == 0 || y == 0 || x == layer->getWidth() - 1 || y == layer->getHeight() - 1) {
                flags |= TileFlags::Solid;
            }
            
            layer->setTile(x, y, Tile(tileId, flags));
        }
    }
    
    // Add layer to tilemap
    tilemap->addLayer(layer);
    
    // Create camera
    auto camera = std::make_shared<Camera>();
    camera->setViewport(Viewport(0, 0, config.windowWidth, config.windowHeight));
    camera->setPosition(mapProps.width * mapProps.tileWidth / 2.0f, mapProps.height * mapProps.tileHeight / 2.0f);
    
    // Create tilemap renderer
    auto tilemapRenderer = std::make_shared<TilemapRenderer>(engine.getGraphicsAPI());
    tilemapRenderer->setTilemap(tilemap);
    tilemapRenderer->setCamera(camera);
    tilemapRenderer->setRenderColliders(true);
    
    // Add tilemap renderer to system manager
    systemManager->addSystem(tilemapRenderer);
    
    // Print instructions
    std::cout << "Tilemap Test" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Q/E - Zoom in/out" << std::endl;
    std::cout << "  C - Toggle collider rendering" << std::endl;
    std::cout << "  F - Toggle frustum culling" << std::endl;
    
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
        
        // Toggle collider rendering
        if (inputManager.isKeyJustPressed(KeyCode::C)) {
            tilemapRenderer->setRenderColliders(!tilemapRenderer->isRenderingColliders());
            std::cout << "Collider rendering: " << (tilemapRenderer->isRenderingColliders() ? "ON" : "OFF") << std::endl;
        }
        
        // Toggle frustum culling
        if (inputManager.isKeyJustPressed(KeyCode::F)) {
            tilemapRenderer->setUseFrustumCulling(!tilemapRenderer->isUsingFrustumCulling());
            std::cout << "Frustum culling: " << (tilemapRenderer->isUsingFrustumCulling() ? "ON" : "OFF") << std::endl;
        }
        
        // Update engine
        if (!engine.update()) {
            running = false;
        }
    }
    
    // Shutdown resource manager
    resourceManager->shutdown();
    
    // Shutdown engine
    engine.shutdown();
    
    return 0;
}