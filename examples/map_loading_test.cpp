#include "../src/core/EngineCore.h"
#include "../src/tilemap/MapLoader.h"
#include "../src/tilemap/TilemapRenderer.h"
#include "../src/resources/ResourceManager.h"
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
    config.windowTitle = "Map Loading Test";
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
    
    // Create map loader
    MapLoader mapLoader(resourceManager);
    
    // Load map
    std::string mapPath = "assets/maps/test_map.tmx";
    auto tilemap = mapLoader.loadMap(mapPath);
    
    if (!tilemap) {
        std::cerr << "Failed to load map: " << mapPath << std::endl;
        return 1;
    }
    
    // Print map information
    const auto& mapProps = tilemap->getProperties();
    std::cout << "Loaded map: " << mapProps.name << std::endl;
    std::cout << "Dimensions: " << mapProps.width << "x" << mapProps.height << " tiles" << std::endl;
    std::cout << "Tile size: " << mapProps.tileWidth << "x" << mapProps.tileHeight << " pixels" << std::endl;
    std::cout << "Layers: " << tilemap->getLayerCount() << std::endl;
    std::cout << "Tilesets: " << tilemap->getTilesetCount() << std::endl;
    
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
    std::cout << "Map Loading Test" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Q/E - Zoom in/out" << std::endl;
    std::cout << "  C - Toggle collider rendering" << std::endl;
    std::cout << "  F - Toggle frustum culling" << std::endl;
    std::cout << "  1-9 - Toggle layer visibility" << std::endl;
    
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
        
        // Toggle layer visibility
        for (int i = 0; i < 9; ++i) {
            if (inputManager.isKeyJustPressed(static_cast<KeyCode>(KeyCode::_1 + i))) {
                if (i < tilemap->getLayerCount()) {
                    auto layer = tilemap->getLayer(i);
                    if (layer) {
                        LayerProperties props = layer->getProperties();
                        props.visible = !props.visible;
                        layer->setProperties(props);
                        std::cout << "Layer " << (i + 1) << " (" << props.name << ") visibility: " << (props.visible ? "ON" : "OFF") << std::endl;
                    }
                }
            }
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