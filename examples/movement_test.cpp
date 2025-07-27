#include "../src/core/EngineCore.h"
#include "../src/physics/MovementSystem.h"
#include "../src/physics/CollisionSystem.h"
#include "../src/physics/PhysicsComponent.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/input/InputManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Physics;

// Custom renderer for physics objects
class PhysicsRenderer : public System {
public:
    PhysicsRenderer(std::shared_ptr<IGraphicsAPI> graphics, std::shared_ptr<ComponentManager> componentManager)
        : System("PhysicsRenderer"), m_graphics(graphics), m_componentManager(componentManager) {}
    
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
                
                // Set color based on whether the body is static or a trigger
                uint32_t color = 0x00FF00FF; // Green for normal bodies
                
                if (physicsComponent->isStatic()) {
                    color = 0x0000FFFF; // Blue for static bodies
                }
                
                if (physicsComponent->isTrigger()) {
                    color = 0xFF00FFFF; // Purple for triggers
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
            }
        }
    }
    
    void onShutdown() override {}
    
private:
    std::shared_ptr<IGraphicsAPI> m_graphics;
    std::shared_ptr<ComponentManager> m_componentManager;
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
    config.windowTitle = "Movement Test";
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
    
    // Create player control system
    auto playerControlSystem = std::make_shared<PlayerControlSystem>(componentManager);
    systemManager->addSystem(playerControlSystem);
    
    // Create renderer
    auto graphics = engine.getGraphicsAPI();
    auto physicsRenderer = std::make_shared<PhysicsRenderer>(graphics, componentManager);
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
    
    // Create a trigger zone
    Entity triggerEntity = entityManager->createEntity();
    auto triggerPhysics = std::make_shared<PhysicsComponent>();
    triggerPhysics->setPosition(400.0f, 200.0f);
    triggerPhysics->setTrigger(true);
    triggerPhysics->setCollisionShape(std::make_shared<CircleShape>(50.0f));
    componentManager->addComponent(triggerEntity, triggerPhysics);
    
    // Register collision callback for trigger events
    collisionSystem->registerCollisionCallback([&](const CollisionEvent& event) {
        // Check if one of the collidables is a trigger
        auto collidable1 = std::dynamic_pointer_cast<PhysicsCollidable>(event.collidable1);
        auto collidable2 = std::dynamic_pointer_cast<PhysicsCollidable>(event.collidable2);
        
        if (collidable1 && collidable2) {
            auto physics1 = collidable1->getPhysicsComponent();
            auto physics2 = collidable2->getPhysicsComponent();
            
            if (physics1->isTrigger() || physics2->isTrigger()) {
                std::cout << "Trigger event: Entity " << collidable1->getEntity().getID() 
                          << " and Entity " << collidable2->getEntity().getID() << std::endl;
            }
        }
    });
    
    // Print instructions
    std::cout << "Movement Test" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/A/S/D - Move player" << std::endl;
    std::cout << "  Space - Jump" << std::endl;
    
    // Run engine
    engine.run();
    
    // Shutdown engine
    engine.shutdown();
    
    return 0;
}