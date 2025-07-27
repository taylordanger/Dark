#include <iostream>
#include <memory>
#include <string>
#include "../src/entities/EntityManager.h"

using namespace RPGEngine;

int main() {
    std::cout << "=== Entity Manager Test ===\n" << std::endl;
    
    // Create entity manager
    EntityManager entityManager;
    
    // Initialize entity manager
    if (!entityManager.initialize()) {
        std::cerr << "Failed to initialize entity manager" << std::endl;
        return -1;
    }
    
    std::cout << "\n=== Creating Entities ===\n" << std::endl;
    
    // Create some entities
    Entity player = entityManager.createEntity("Player");
    Entity npc1 = entityManager.createEntity("NPC_1");
    Entity npc2 = entityManager.createEntity("NPC_2");
    Entity item = entityManager.createEntity("Item");
    
    // Print entity information
    std::cout << "Created entities:" << std::endl;
    std::cout << "Player: ID=" << player.getID() << ", Name=" << player.getName() << ", Active=" << player.isActive() << std::endl;
    std::cout << "NPC 1: ID=" << npc1.getID() << ", Name=" << npc1.getName() << ", Active=" << npc1.isActive() << std::endl;
    std::cout << "NPC 2: ID=" << npc2.getID() << ", Name=" << npc2.getName() << ", Active=" << npc2.isActive() << std::endl;
    std::cout << "Item: ID=" << item.getID() << ", Name=" << item.getName() << ", Active=" << item.isActive() << std::endl;
    
    std::cout << "\nTotal entities: " << entityManager.getEntityCount() << std::endl;
    std::cout << "Active entities: " << entityManager.getActiveEntityCount() << std::endl;
    
    std::cout << "\n=== Entity Lookup ===\n" << std::endl;
    
    // Look up entities
    Entity foundPlayer = entityManager.getEntityByName("Player");
    Entity foundNPC = entityManager.getEntityByName("NPC_1");
    Entity notFound = entityManager.getEntityByName("DoesNotExist");
    
    std::cout << "Found Player: " << (foundPlayer.isValid() ? "Yes" : "No") << std::endl;
    std::cout << "Found NPC_1: " << (foundNPC.isValid() ? "Yes" : "No") << std::endl;
    std::cout << "Found DoesNotExist: " << (notFound.isValid() ? "Yes" : "No") << std::endl;
    
    std::cout << "\n=== Entity Activation/Deactivation ===\n" << std::endl;
    
    // Deactivate an entity
    entityManager.setEntityActive(npc1, false);
    std::cout << "Deactivated NPC_1" << std::endl;
    
    // Check active state
    std::cout << "NPC_1 active: " << entityManager.getEntity(npc1.getID()).isActive() << std::endl;
    std::cout << "Active entities: " << entityManager.getActiveEntityCount() << std::endl;
    
    std::cout << "\n=== Entity Iteration ===\n" << std::endl;
    
    // Iterate over all entities
    std::cout << "All entities:" << std::endl;
    entityManager.forEachEntity([](Entity entity) {
        std::cout << "Entity ID=" << entity.getID() << ", Name=" << entity.getName() 
                  << ", Active=" << entity.isActive() << std::endl;
    });
    
    // Iterate over active entities
    std::cout << "\nActive entities:" << std::endl;
    entityManager.forEachActiveEntity([](Entity entity) {
        std::cout << "Entity ID=" << entity.getID() << ", Name=" << entity.getName() << std::endl;
    });
    
    std::cout << "\n=== Entity Destruction ===\n" << std::endl;
    
    // Destroy an entity
    std::cout << "Destroying Item entity" << std::endl;
    entityManager.destroyEntity(item);
    
    // Mark an entity for deferred destruction
    std::cout << "Marking NPC_2 for deferred destruction" << std::endl;
    entityManager.destroyEntityDeferred(npc2);
    
    // Process deferred operations
    std::cout << "Processing deferred operations" << std::endl;
    entityManager.update();
    
    // Check entity count
    std::cout << "Total entities: " << entityManager.getEntityCount() << std::endl;
    std::cout << "Active entities: " << entityManager.getActiveEntityCount() << std::endl;
    
    // Check if entities exist
    std::cout << "Player exists: " << entityManager.entityExists(player.getID()) << std::endl;
    std::cout << "Item exists: " << entityManager.entityExists(item.getID()) << std::endl;
    std::cout << "NPC_2 exists: " << entityManager.entityExists(npc2.getID()) << std::endl;
    
    std::cout << "\n=== Entity Reuse ===\n" << std::endl;
    
    // Create new entities to reuse IDs
    Entity newEntity1 = entityManager.createEntity("NewEntity1");
    Entity newEntity2 = entityManager.createEntity("NewEntity2");
    
    std::cout << "New entity 1: ID=" << newEntity1.getID() << ", Name=" << newEntity1.getName() << std::endl;
    std::cout << "New entity 2: ID=" << newEntity2.getID() << ", Name=" << newEntity2.getName() << std::endl;
    
    std::cout << "\n=== Clearing Entities ===\n" << std::endl;
    
    // Clear all entities
    entityManager.clearAllEntities();
    std::cout << "All entities cleared" << std::endl;
    std::cout << "Total entities: " << entityManager.getEntityCount() << std::endl;
    
    // Shutdown entity manager
    entityManager.shutdown();
    
    std::cout << "\nEntity Manager Test completed successfully!" << std::endl;
    return 0;
}