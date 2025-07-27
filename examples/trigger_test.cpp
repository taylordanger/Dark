#include "../src/core/EngineCore.h"
#include "../src/physics/MovementSystem.h"
#include "../src/physics/CollisionSystem.h"
#include "../src/physics/TriggerSystem.h"
#include "../src/physics/PhysicsComponent.h"
#include "../src/physics/TriggerComponent.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/input/InputManager.h"
#include <iostream>
#include <memory>
#include <string>

using namespace RPGEngine;
using namespace RPGEngine::Physics;

// Custom renderer for physics objects and triggers
class PhysicsRenderer : public System {
public:
    PhysicsRenderer(std::shared_ptr<IGraphicsAPI> graphics, std::shared_ptr<ComponentManager> componentManager,
                   std::shared_ptr<TriggerSystem> triggerSystem)
        : System("PhysicsRenderer"), m_graphics(graphics), m_componentManager(componentManager),
          m_triggerSystem(triggerSystem) {}
    
    bool onInitialize() override {
        return true;
    }
    
    void onUpdate(float deltaTime) override {
        // Get all entities with physics components
        auto entities = m_componentManager->getEntitiesWithComponent(PhysicsComponent::getStaticType());
        
        for (auto entity : entities) {
            auto physicsComponent = m_componentManager->getComponent<PhysicsComponent>(entity);
            if (physicsComponent && physicsComponent->getCollisionShape()) {
                // Draw shape
                auto shape = physicsComponent->getCollisionShape();
                
                // Set color based on whether the body is static, a trigger, or in a trigger
                uint32_t color = 0x00FF00FF; // Green for normal bodies
                
                if (physicsComponent->isStatic()) {
                    color = 0x0000FFFF; // Blue for static bodies
                }
                
                if (physicsComponent->isTrigger()) {
                    // Check if this is a trigger entity
                    auto triggerComponent = m_componentManager->getComponent<TriggerComponent>(entity);
                    if (triggerComponent) {
                        if (triggerComponent->isActive()) {
                            if (triggerComponent->isTriggered()) {
                                color = 0xFF00FFFF; // Purple for active triggered triggers
                            } else {
                                color = 0xFF0080FF; // Pink for active triggers
                            }
                        } else {
                            color = 0x800080FF; // Dark purple for inactive triggers
                        }
                    } else {
                        color = 0xFF00FFFF; // Purple for triggers without TriggerComponent
                    }
                } else {
                    // Check if this entity is in any trigger
                    auto triggers = m_triggerSystem->getTriggersContainingEntity(entity);
                    if (!triggers.empty()) {
                        color = 0xFFFF00FF; // Yellow for entities in triggers
                    }
                }
                
                // Draw based on shape type
                if (shape->getType() == ShapeType::Circle) {
                    auto circle = std::static_pointer_cast<CircleShape>(shape);
                    m_graphics->drawCircle(circle->getPosition().x, circle->getPosition().y, 
                                         circle->getRadius(), color);
                } else if (shape->getType() == ShapeType::Rectangle) {
                    auto rect = std::static_pointer_cast<RectangleShape>(shape);
                    m_graphics->drawRectangle(rect->getPosition().x - rect->getWidth() * 0.5f, 
                                            rect->getPosition().y - rect->getHeight() * 0.5f,
                                            rect->getWidth(), rect->getHeight(), 
                                            rect->getRotation(), color);
                }
                
                // Draw velocity vector
                Vector2 pos = physicsComponent->getPosition();
                Vector2 vel = physicsComponent->getVelocity();
                if (vel.lengthSquared() > 0.1f) {
                    Vector2 endPoint = pos + vel * 0.1f; // Scale for visualization
                    m_graphics->drawLine(pos.x, pos.y, endPoint.x, endPoint.y, 0xFF0000FF);
                }
                
                // Draw entity ID
                std::string idText = std::to_string(entity.getID());
                m_graphics->drawText(pos.x, pos.y, idText.c_str(), 0xFFFFFFFF);
            }
        }
    }
    
    void onShutdown() override {}
    
private:
    std::shared_ptr<IGraphicsAPI> m_graphics;
    std::shared_ptr<ComponentManager> m_componentManager;
    std::shared_ptr<TriggerSystem> m_triggerSystem;
};

// Custom system for controlling the player
class PlayerControlSystem : public System {
public:
    PlayerControlSystem(std::shared_ptr<ComponentManager> componentManager)
        : System("PlayerControlSystem"), m_componentManager(componentManager), m_playerEntity() {}
    
    bool onInitialize() override {
        return true;
    }
    
    void setPlayerEntity(Entity entity) {
        m_playerEntity = entity;
    }
    
    void onUpdate(float deltaTime) override {
        if (!m_playerEntity.isValid()) {
            return;
        }
        
        auto physicsComponent = m_componentManager->getComponent<PhysicsComponent>(m_playerEntity);
        if (!physicsComponent) {
            return;
        }
        
        // Get input
        auto& inputManager = InputManager::getInstance();
        
        // Movement force
        Vector2 moveForce(0.0f, 0.0f);
        float moveSpeed = 500.0f;
        
        if (inputManager.isKeyPressed(KeyCode::W)) {
            moveForce.y -= moveSpeed;
        }
        if (inputManager.isKeyPressed(KeyCode::S)) {
            moveForce.y += moveSpeed;
        }
        if (inputManager.isKeyPressed(KeyCode::A)) {
            moveForce.x -= moveSpeed;
        }
        if (inputManager.isKeyPressed(KeyCode::D)) {
            moveForce.x += moveSpeed;
        }
        
        // Apply force
        physicsComponent->applyForce(moveForce);
        
        // Jump
        if (inputManager.isKeyJustPressed(KeyCode::Space)) {
            physicsComponent->applyImpulse(Vector2(0.0f, -300.0f));
        }
    }
    
    void onShutdown() override {}
    
private:
    std::shared_ptr<ComponentManager> m_componentManager;
    Entity m_playerEntity;
};

int main() {
    // Create engine
    EngineCore engine;
    
    // Initialize engine
    EngineConfig config;
    config.windowTitle = "Trigger Test";
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
    
    // Create collision system
    auto collisionSystem = std::make_shared<CollisionSystem>(800.0f, 600.0f, 100.0f);
    systemManager->addSystem(collisionSystem);
    
    // Create movement system
    auto movementSystem = std::make_shared<MovementSystem>(componentManager, collisionSystem);
    movementSystem->setGravity(Vector2(0.0f, 500.0f)); // Set gravity
    systemManager->addSystem(movementSystem);
    
    // Create trigger system
    auto triggerSystem = std::make_shared<TriggerSystem>(componentManager, collisionSystem);
    systemManager->addSystem(triggerSystem);
    
    // Create player control system
    auto playerControlSystem = std::make_shared<PlayerControlSystem>(componentManager);
    systemManager->addSystem(playerControlSystem);
    
    // Create renderer
    auto graphics = engine.getGraphicsAPI();
    auto physicsRenderer = std::make_shared<PhysicsRenderer>(graphics, componentManager, triggerSystem);
    systemManager->addSystem(physicsRenderer);
    
    // Create player entity
    Entity playerEntity = entityManager->createEntity();
    auto playerPhysics = std::make_shared<PhysicsComponent>();
    playerPhysics->setPosition(400.0f, 300.0f);
    playerPhysics->setMass(1.0f);
    playerPhysics->setFriction(0.2f);
    playerPhysics->setRestitution(0.5f);
    playerPhysics->setCollisionShape(std::make_shared<CircleShape>(20.0f));
    componentManager->addComponent(playerEntity, playerPhysics);
    playerControlSystem->setPlayerEntity(playerEntity);
    
    // Create ground
    Entity groundEntity = entityManager->createEntity();
    auto groundPhysics = std::make_shared<PhysicsComponent>();
    groundPhysics->setPosition(400.0f, 550.0f);
    groundPhysics->setStatic(true);
    groundPhysics->setCollisionShape(std::make_shared<RectangleShape>(700.0f, 20.0f));
    componentManager->addComponent(groundEntity, groundPhysics);
    
    // Create platforms
    Entity platform1Entity = entityManager->createEntity();
    auto platform1Physics = std::make_shared<PhysicsComponent>();
    platform1Physics->setPosition(200.0f, 400.0f);
    platform1Physics->setStatic(true);
    platform1Physics->setCollisionShape(std::make_shared<RectangleShape>(200.0f, 20.0f));
    componentManager->addComponent(platform1Entity, platform1Physics);
    
    Entity platform2Entity = entityManager->createEntity();
    auto platform2Physics = std::make_shared<PhysicsComponent>();
    platform2Physics->setPosition(600.0f, 300.0f);
    platform2Physics->setStatic(true);
    platform2Physics->setCollisionShape(std::make_shared<RectangleShape>(200.0f, 20.0f));
    componentManager->addComponent(platform2Entity, platform2Physics);
    
    // Create some dynamic objects
    for (int i = 0; i < 5; ++i) {
        Entity boxEntity = entityManager->createEntity();
        auto boxPhysics = std::make_shared<PhysicsComponent>();
        boxPhysics->setPosition(300.0f + i * 50.0f, 100.0f + i * 30.0f);
        boxPhysics->setMass(1.0f);
        boxPhysics->setFriction(0.2f);
        boxPhysics->setRestitution(0.3f);
        boxPhysics->setCollisionShape(std::make_shared<RectangleShape>(30.0f, 30.0f));
        componentManager->addComponent(boxEntity, boxPhysics);
    }
    
    // Create trigger zones
    
    // Create a simple trigger zone
    Entity simpleTriggerEntity = entityManager->createEntity();
    auto simpleTriggerPhysics = std::make_shared<PhysicsComponent>();
    simpleTriggerPhysics->setPosition(400.0f, 200.0f);
    simpleTriggerPhysics->setTrigger(true);
    simpleTriggerPhysics->setCollisionShape(std::make_shared<CircleShape>(50.0f));
    componentManager->addComponent(simpleTriggerEntity, simpleTriggerPhysics);
    
    auto simpleTrigger = std::make_shared<TriggerComponent>();
    simpleTrigger->setTag("simple_trigger");
    simpleTrigger->addCallback(TriggerEventType::Enter, [](const TriggerEvent& event) {
        std::cout << "Entity " << event.otherEntity.getID() << " entered simple trigger " 
                  << event.triggerEntity.getID() << std::endl;
    });
    simpleTrigger->addCallback(TriggerEventType::Exit, [](const TriggerEvent& event) {
        std::cout << "Entity " << event.otherEntity.getID() << " exited simple trigger " 
                  << event.triggerEntity.getID() << std::endl;
    });
    componentManager->addComponent(simpleTriggerEntity, simpleTrigger);
    
    // Create a one-shot trigger
    Entity oneShotTriggerEntity = entityManager->createEntity();
    auto oneShotTriggerPhysics = std::make_shared<PhysicsComponent>();
    oneShotTriggerPhysics->setPosition(200.0f, 200.0f);
    oneShotTriggerPhysics->setTrigger(true);
    oneShotTriggerPhysics->setCollisionShape(std::make_shared<CircleShape>(40.0f));
    componentManager->addComponent(oneShotTriggerEntity, oneShotTriggerPhysics);
    
    auto oneShotTrigger = std::make_shared<TriggerComponent>();
    oneShotTrigger->setTag("one_shot_trigger");
    oneShotTrigger->setOneShot(true);
    oneShotTrigger->addCallback(TriggerEventType::Enter, [](const TriggerEvent& event) {
        std::cout << "Entity " << event.otherEntity.getID() << " entered one-shot trigger " 
                  << event.triggerEntity.getID() << " (will only trigger once)" << std::endl;
    });
    componentManager->addComponent(oneShotTriggerEntity, oneShotTrigger);
    
    // Create a cooldown trigger
    Entity cooldownTriggerEntity = entityManager->createEntity();
    auto cooldownTriggerPhysics = std::make_shared<PhysicsComponent>();
    cooldownTriggerPhysics->setPosition(600.0f, 200.0f);
    cooldownTriggerPhysics->setTrigger(true);
    cooldownTriggerPhysics->setCollisionShape(std::make_shared<CircleShape>(40.0f));
    componentManager->addComponent(cooldownTriggerEntity, cooldownTriggerPhysics);
    
    auto cooldownTrigger = std::make_shared<TriggerComponent>();
    cooldownTrigger->setTag("cooldown_trigger");
    cooldownTrigger->setCooldown(3.0f); // 3 second cooldown
    cooldownTrigger->addCallback(TriggerEventType::Enter, [](const TriggerEvent& event) {
        std::cout << "Entity " << event.otherEntity.getID() << " entered cooldown trigger " 
                  << event.triggerEntity.getID() << " (has 3 second cooldown)" << std::endl;
    });
    componentManager->addComponent(cooldownTriggerEntity, cooldownTrigger);
    
    // Create a filtered trigger (only responds to player)
    Entity filteredTriggerEntity = entityManager->createEntity();
    auto filteredTriggerPhysics = std::make_shared<PhysicsComponent>();
    filteredTriggerPhysics->setPosition(400.0f, 400.0f);
    filteredTriggerPhysics->setTrigger(true);
    filteredTriggerPhysics->setCollisionShape(std::make_shared<RectangleShape>(100.0f, 40.0f));
    componentManager->addComponent(filteredTriggerEntity, filteredTriggerPhysics);
    
    auto filteredTrigger = std::make_shared<TriggerComponent>();
    filteredTrigger->setTag("filtered_trigger");
    filteredTrigger->addFilterEntity(playerEntity);
    filteredTrigger->addCallback(TriggerEventType::Enter, [](const TriggerEvent& event) {
        std::cout << "Player entered filtered trigger " << event.triggerEntity.getID() << std::endl;
    });
    filteredTrigger->addCallback(TriggerEventType::Exit, [](const TriggerEvent& event) {
        std::cout << "Player exited filtered trigger " << event.triggerEntity.getID() << std::endl;
    });
    componentManager->addComponent(filteredTriggerEntity, filteredTrigger);
    
    // Register global trigger callback
    triggerSystem->registerGlobalCallback([](const TriggerEvent& event) {
        if (event.type == TriggerEventType::Enter) {
            std::cout << "Global: Entity " << event.otherEntity.getID() << " entered trigger " 
                      << event.triggerEntity.getID() << std::endl;
        } else if (event.type == TriggerEventType::Exit) {
            std::cout << "Global: Entity " << event.otherEntity.getID() << " exited trigger " 
                      << event.triggerEntity.getID() << std::endl;
        }
    });
    
    // Print instructions
    std::cout << "Trigger Test" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/A/S/D - Move player" << std::endl;
    std::cout << "  Space - Jump" << std::endl;
    std::cout << std::endl;
    std::cout << "Trigger Types:" << std::endl;
    std::cout << "  Pink Circle - Simple trigger (responds to all entities)" << std::endl;
    std::cout << "  Pink Circle (left) - One-shot trigger (only triggers once)" << std::endl;
    std::cout << "  Pink Circle (right) - Cooldown trigger (3 second cooldown)" << std::endl;
    std::cout << "  Pink Rectangle - Filtered trigger (only responds to player)" << std::endl;
    
    // Run engine
    engine.run();
    
    // Shutdown engine
    engine.shutdown();
    
    return 0;
}