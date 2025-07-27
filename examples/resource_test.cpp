#include "../src/resources/ResourceManager.h"
#include "../src/resources/TextureResource.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace RPGEngine;
using namespace RPGEngine::Resources;

int main() {
    // Create resource manager
    ResourceManager resourceManager;
    
    // Initialize resource manager
    if (!resourceManager.initialize()) {
        std::cerr << "Failed to initialize resource manager" << std::endl;
        return 1;
    }
    
    // Set base path for resources
    resourceManager.setBasePath("assets/");
    
    // Create texture resources
    auto texture1 = std::make_shared<TextureResource>("texture1", "assets/textures/texture1.png");
    auto texture2 = std::make_shared<TextureResource>("texture2", "assets/textures/texture2.png");
    auto texture3 = std::make_shared<TextureResource>("texture3", "assets/textures/texture3.png");
    
    // Add resources to manager
    resourceManager.addResource(texture1);
    resourceManager.addResource(texture2);
    resourceManager.addResource(texture3);
    
    // Load resources synchronously
    std::cout << "Loading texture1 synchronously..." << std::endl;
    resourceManager.loadResource("texture1", [](std::shared_ptr<Resource> resource) {
        std::cout << "Texture1 loaded: " << (resource->isLoaded() ? "success" : "failed") << std::endl;
    });
    
    // Load resources asynchronously
    std::cout << "Loading texture2 and texture3 asynchronously..." << std::endl;
    resourceManager.loadResourceAsync("texture2", [](std::shared_ptr<Resource> resource) {
        std::cout << "Texture2 loaded: " << (resource->isLoaded() ? "success" : "failed") << std::endl;
    });
    
    resourceManager.loadResourceAsync("texture3", [](std::shared_ptr<Resource> resource) {
        std::cout << "Texture3 loaded: " << (resource->isLoaded() ? "success" : "failed") << std::endl;
    });
    
    // Update resource manager to process async loads
    for (int i = 0; i < 10; ++i) {
        resourceManager.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Print resource statistics
    std::cout << "Resource statistics:" << std::endl;
    std::cout << "  Total resources: " << resourceManager.getResourceCount() << std::endl;
    std::cout << "  Loaded resources: " << resourceManager.getLoadedResourceCount() << std::endl;
    std::cout << "  Loading resources: " << resourceManager.getLoadingResourceCount() << std::endl;
    std::cout << "  Failed resources: " << resourceManager.getFailedResourceCount() << std::endl;
    
    // Get all texture resources
    auto textures = resourceManager.getResourcesOfType<TextureResource>();
    std::cout << "Texture resources: " << textures.size() << std::endl;
    
    for (const auto& texture : textures) {
        std::cout << "  " << texture->getId() << ": " << texture->getWidth() << "x" << texture->getHeight() << std::endl;
    }
    
    // Unload resources
    std::cout << "Unloading texture1..." << std::endl;
    resourceManager.unloadResource("texture1");
    
    // Reload resources
    std::cout << "Reloading texture1..." << std::endl;
    resourceManager.reloadResource("texture1", [](std::shared_ptr<Resource> resource) {
        std::cout << "Texture1 reloaded: " << (resource->isLoaded() ? "success" : "failed") << std::endl;
    });
    
    // Clear unused resources
    std::cout << "Clearing unused resources..." << std::endl;
    size_t cleared = resourceManager.clearUnusedResources();
    std::cout << "Cleared " << cleared << " unused resources" << std::endl;
    
    // Shutdown resource manager
    resourceManager.shutdown();
    
    return 0;
}