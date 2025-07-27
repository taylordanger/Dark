#include <iostream>
#include <memory>
#include <string>
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/components/TransformComponent.h"
#include "../src/components/SpriteComponent.h"
#include "../src/components/PhysicsComponent.h"

using namespace RPGEngine;
using namespace RPGEngine::Components;

int main() {
    std::cout << "=== Component System Test ===\n" << std::endl;
    
    // Create entity manager
    EntityManager entityManager;
    if (!entityManager.initialize()) {
        std::cerr << "Failed to initialize entity manager" << std::endl;
        return -1;
    }
    
    // Create component manager
    ComponentManager componentManager;
    if (!componentManager.initialize()) {
        std::cerr << "Failed to initialize component manager" << std::endl;
        return -1;
    }
    
    std::cout << "\n=== Creating Entities and Components ===\n" << std::endl;
    
    // Create player entity
    Entity player = entityManager.createEntity("Player");
    std::cout << "Created player entity: ID=" << player.getID() << ", Name=" << player.getName() << std::endl;
    
    // Add components to player
    auto playerTransform = componentManager.createComponent<TransformComponent>(player, 100.0f, 200.0f, 0.0f, 1.0f, 1.0f);
    auto playerSprite = componentManager.createComponent<SpriteComponent>(player, "assets/player.png", 32, 32, 1, true);
    auto playerPhysics = componentManager.createComponent<PhysicsComponent>(player, 0.0f, 0.0f, 1.0f, true);
    
    std::cout << "Added components to player:" << std::endl;
    std::cout << "- Transform: Position=(" << playerTransform->getX() << ", " << playerTransform->getY() << ")" << std::endl;
    std::cout << "- Sprite: Texture=" << playerSprite->getTexturePath() << ", Size=(" << playerSprite->getWidth() << ", " << playerSprite->getHeight() << ")" << std::endl;
    std::cout << "- Physics: Mass=" << playerPhysics->getMass() << ", Solid=" << playerPhysics->isSolid() << std::endl;
    
    // Create enemy entity
    Entity enemy = entityManager.createEntity("Enemy");
    std::cout << "\nCreated enemy entity: ID=" << enemy.getID() << ", Name=" << enemy.getName() << std::endl;
    
    // Add components to enemy
    auto enemyTransform = componentManager.createComponent<TransformComponent>(enemy, 300.0f, 150.0f, 180.0f, 1.2f, 1.2f);
    auto enemySprite = componentManager.createComponent<SpriteComponent>(enemy, "assets/enemy.png", 32, 32, 1, true);
    
    std::cout << "Added components to enemy:" << std::endl;
    std::cout << "- Transform: Position=(" << enemyTransform->getX() << ", " << enemyTransform->getY() << ")" << std::endl;
    std::cout << "- Sprite: Texture=" << enemySprite->getTexturePath() << ", Size=(" << enemySprite->getWidth() << ", " << enemySprite->getHeight() << ")" << std::endl;
    
    // Create item entity
    Entity item = entityManager.createEntity("Item");
    std::cout << "\nCreated item entity: ID=" << item.getID() << ", Name=" << item.getName() << std::endl;
    
    // Add components to item
    auto itemTransform = componentManager.createComponent<TransformComponent>(item, 200.0f, 250.0f);
    auto itemSprite = componentManager.createComponent<SpriteComponent>(item, "assets/item.png", 16, 16, 0, true);
    
    std::cout << "Added components to item:" << std::endl;
    std::cout << "- Transform: Position=(" << itemTransform->getX() << ", " << itemTransform->getY() << ")" << std::endl;
    std::cout << "- Sprite: Texture=" << itemSprite->getTexturePath() << ", Size=(" << itemSprite->getWidth() << ", " << itemSprite->getHeight() << ")" << std::endl;
    
    std::cout << "\n=== Component Queries ===\n" << std::endl;
    
    // Check if entities have components
    std::cout << "Player has Transform: " << componentManager.hasComponent<TransformComponent>(player) << std::endl;
    std::cout << "Player has Physics: " << componentManager.hasComponent<PhysicsComponent>(player) << std::endl;
    std::cout << "Enemy has Physics: " << componentManager.hasComponent<PhysicsComponent>(enemy) << std::endl;
    
    // Get component counts
    std::cout << "\nComponent counts:" << std::endl;
    std::cout << "Transform components: " << componentManager.getComponentCount<TransformComponent>() << std::endl;
    std::cout << "Sprite components: " << componentManager.getComponentCount<SpriteComponent>() << std::endl;
    std::cout << "Physics components: " << componentManager.getComponentCount<PhysicsComponent>() << std::endl;
    
    // Get entities with specific components
    std::cout << "\nEntities with Transform component: " << componentManager.getEntitiesWithComponent<TransformComponent>().size() << std::endl;
    std::cout << "Entities with Physics component: " << componentManager.getEntitiesWithComponent<PhysicsComponent>().size() << std::endl;
    
    std::cout << "\n=== Component Modification ===\n" << std::endl;
    
    // Modify player transform
    playerTransform->translate(50.0f, -30.0f);
    playerTransform->rotate(45.0f);
    
    std::cout << "Modified player transform:" << std::endl;
    std::cout << "- New position: (" << playerTransform->getX() << ", " << playerTransform->getY() << ")" << std::endl;
    std::cout << "- New rotation: " << playerTransform->getRotation() << " degrees" << std::endl;
    
    // Modify player physics
    playerPhysics->setVelocity(5.0f, 2.0f);
    playerPhysics->applyForce(10.0f, 0.0f);
    
    std::cout << "Modified player physics:" << std::endl;
    std::cout << "- New velocity: (" << playerPhysics->getVelocityX() << ", " << playerPhysics->getVelocityY() << ")" << std::endl;
    
    std::cout << "\n=== Component Iteration ===\n" << std::endl;
    
    // Iterate over all transform components
    std::cout << "All transform components:" << std::endl;
    componentManager.forEachComponent<TransformComponent>([](Entity entity, std::shared_ptr<TransformComponent> transform) {
        std::cout << "Entity " << entity.getID() << " (" << entity.getName() << "): Position=(" 
                  << transform->getX() << ", " << transform->getY() << "), Rotation=" << transform->getRotation() << std::endl;
    });
    
    std::cout << "\n=== Component Removal ===\n" << std::endl;
    
    // Remove a component
    std::cout << "Removing physics component from player" << std::endl;
    componentManager.removeComponent<PhysicsComponent>(player);
    
    std::cout << "Player has Physics: " << componentManager.hasComponent<PhysicsComponent>(player) << std::endl;
    std::cout << "Physics components: " << componentManager.getComponentCount<PhysicsComponent>() << std::endl;
    
    // Remove all components from an entity
    std::cout << "\nRemoving all components from item" << std::endl;
    componentManager.removeAllComponents(item);
    
    std::cout << "Item has Transform: " << componentManager.hasComponent<TransformComponent>(item) << std::endl;
    std::cout << "Item has Sprite: " << componentManager.hasComponent<SpriteComponent>(item) << std::endl;
    
    std::cout << "\n=== Entity Destruction ===\n" << std::endl;
    
    // Destroy an entity
    std::cout << "Destroying enemy entity" << std::endl;
    entityManager.destroyEntity(enemy);
    
    // Check if components were automatically removed
    std::cout << "Enemy Transform component still exists: " << (componentManager.getComponent<TransformComponent>(enemy) != nullptr) << std::endl;
    
    std::cout << "\n=== Cleanup ===\n" << std::endl;
    
    // Clear all components
    componentManager.clearAllComponents();
    std::cout << "All components cleared" << std::endl;
    
    // Shutdown managers
    componentManager.shutdown();
    entityManager.shutdown();
    
    std::cout << "\nComponent System Test completed successfully!" << std::endl;
    return 0;
}