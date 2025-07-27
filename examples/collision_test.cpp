#include "../src/core/EngineCore.h"
#include "../src/physics/CollisionSystem.h"
#include "../src/physics/CollisionShape.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/input/InputManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Physics;

// Simple collidable implementation for testing
class TestCollidable : public ICollidable {
public:
    TestCollidable(uint32_t id, std::shared_ptr<CollisionShape> shape, uint32_t layer = 1, uint32_t mask = 0xFFFFFFFF)
        : m_id(id), m_shape(shape), m_layer(layer), m_mask(mask) {}
    
    const CollisionShape& getCollisionShape() const override { return *m_shape; }
    uint32_t getCollidableID() const override { return m_id; }
    uint32_t getCollisionLayer() const override { return m_layer; }
    uint32_t getCollisionMask() const override { return m_mask; }
    
    void setPosition(const Vector2& position) { m_shape->setPosition(position); }
    void setRotation(float rotation) { m_shape->setRotation(rotation); }
    
private:
    uint32_t m_id;
    std::shared_ptr<CollisionShape> m_shape;
    uint32_t m_layer;
    uint32_t m_mask;
};

// Custom system for collision testing
class CollisionTestSystem : public System {
public:
    CollisionTestSystem(std::shared_ptr<CollisionSystem> collisionSystem)
        : System("CollisionTestSystem"), m_collisionSystem(collisionSystem), m_circle(nullptr), m_rectangle(nullptr) {}
    
    bool onInitialize() override {
        // Create test shapes
        auto circleShape = std::make_shared<CircleShape>(50.0f);
        auto rectangleShape = std::make_shared<RectangleShape>(100.0f, 60.0f);
        
        // Set initial positions
        circleShape->setPosition(200.0f, 200.0f);
        rectangleShape->setPosition(400.0f, 300.0f);
        
        // Create collidables
        m_circle = std::make_shared<TestCollidable>(1, circleShape);
        m_rectangle = std::make_shared<TestCollidable>(2, rectangleShape);
        
        // Register with collision system
        m_collisionSystem->registerCollidable(m_circle);
        m_collisionSystem->registerCollidable(m_rectangle);
        
        // Register collision callback
        m_callbackId = m_collisionSystem->registerCollisionCallback(
            [](const CollisionEvent& event) {
                std::cout << "Collision detected between " << event.collidable1->getCollidableID() 
                          << " and " << event.collidable2->getCollidableID() << std::endl;
                std::cout << "  Contact point: (" << event.result.contactPoint.x << ", " 
                          << event.result.contactPoint.y << ")" << std::endl;
                std::cout << "  Normal: (" << event.result.normal.x << ", " 
                          << event.result.normal.y << ")" << std::endl;
                std::cout << "  Penetration: " << event.result.penetration << std::endl;
            }
        );
        
        return true;
    }
    
    void onUpdate(float deltaTime) override {
        // Update circle position based on input
        if (InputManager::getInstance().isKeyPressed(KeyCode::W)) {
            m_circle->setPosition(Vector2(m_circle->getCollisionShape().getPosition().x, 
                                        m_circle->getCollisionShape().getPosition().y - 5.0f));
        }
        if (InputManager::getInstance().isKeyPressed(KeyCode::S)) {
            m_circle->setPosition(Vector2(m_circle->getCollisionShape().getPosition().x, 
                                        m_circle->getCollisionShape().getPosition().y + 5.0f));
        }
        if (InputManager::getInstance().isKeyPressed(KeyCode::A)) {
            m_circle->setPosition(Vector2(m_circle->getCollisionShape().getPosition().x - 5.0f, 
                                        m_circle->getCollisionShape().getPosition().y));
        }
        if (InputManager::getInstance().isKeyPressed(KeyCode::D)) {
            m_circle->setPosition(Vector2(m_circle->getCollisionShape().getPosition().x + 5.0f, 
                                        m_circle->getCollisionShape().getPosition().y));
        }
        
        // Update rectangle rotation
        if (InputManager::getInstance().isKeyPressed(KeyCode::Q)) {
            m_rectangle->setRotation(m_rectangle->getCollisionShape().getRotation() - 0.05f);
        }
        if (InputManager::getInstance().isKeyPressed(KeyCode::E)) {
            m_rectangle->setRotation(m_rectangle->getCollisionShape().getRotation() + 0.05f);
        }
        
        // Update collision shapes in the collision system
        m_collisionSystem->updateCollidable(m_circle);
        m_collisionSystem->updateCollidable(m_rectangle);
    }
    
    void onShutdown() override {
        // Unregister collision callback
        m_collisionSystem->unregisterCollisionCallback(m_callbackId);
        
        // Unregister collidables
        m_collisionSystem->unregisterCollidable(m_circle);
        m_collisionSystem->unregisterCollidable(m_rectangle);
    }
    
private:
    std::shared_ptr<CollisionSystem> m_collisionSystem;
    std::shared_ptr<TestCollidable> m_circle;
    std::shared_ptr<TestCollidable> m_rectangle;
    int m_callbackId;
};

// Custom renderer for collision shapes
class CollisionShapeRenderer : public System {
public:
    CollisionShapeRenderer(std::shared_ptr<IGraphicsAPI> graphics, std::shared_ptr<CollisionSystem> collisionSystem)
        : System("CollisionShapeRenderer"), m_graphics(graphics), m_collisionSystem(collisionSystem) {}
    
    bool onInitialize() override {
        return true;
    }
    
    void onUpdate(float deltaTime) override {
        // Query all collidables
        m_collisionSystem->queryRegion(CircleShape(1000.0f), [this](std::shared_ptr<ICollidable> collidable) {
            const CollisionShape& shape = collidable->getCollisionShape();
            
            // Set color based on collision state
            bool isColliding = false;
            m_collisionSystem->queryRegion(shape, [&](std::shared_ptr<ICollidable> other) {
                if (other->getCollidableID() != collidable->getCollidableID()) {
                    CollisionResult result;
                    if (m_collisionSystem->checkCollision(shape, other->getCollisionShape(), &result)) {
                        isColliding = true;
                    }
                }
            });
            
            // Draw shape
            if (shape.getType() == ShapeType::Circle) {
                const CircleShape& circle = static_cast<const CircleShape&>(shape);
                m_graphics->drawCircle(circle.getPosition().x, circle.getPosition().y, circle.getRadius(), 
                                     isColliding ? 0xFF0000FF : 0x00FF00FF);
            } else if (shape.getType() == ShapeType::Rectangle) {
                const RectangleShape& rect = static_cast<const RectangleShape&>(shape);
                m_graphics->drawRectangle(rect.getPosition().x - rect.getWidth() * 0.5f, 
                                        rect.getPosition().y - rect.getHeight() * 0.5f,
                                        rect.getWidth(), rect.getHeight(), rect.getRotation(),
                                        isColliding ? 0xFF0000FF : 0x00FF00FF);
            } else if (shape.getType() == ShapeType::Polygon) {
                const PolygonShape& poly = static_cast<const PolygonShape&>(shape);
                std::vector<Vector2> vertices = poly.getTransformedVertices();
                
                for (size_t i = 0; i < vertices.size(); ++i) {
                    size_t j = (i + 1) % vertices.size();
                    m_graphics->drawLine(vertices[i].x, vertices[i].y, vertices[j].x, vertices[j].y,
                                       isColliding ? 0xFF0000FF : 0x00FF00FF);
                }
            }
        });
    }
    
    void onShutdown() override {
    }
    
private:
    std::shared_ptr<IGraphicsAPI> m_graphics;
    std::shared_ptr<CollisionSystem> m_collisionSystem;
};

int main() {
    // Create engine
    EngineCore engine;
    
    // Initialize engine
    EngineConfig config;
    config.windowTitle = "Collision Test";
    config.windowWidth = 800;
    config.windowHeight = 600;
    
    if (!engine.initialize(config)) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    // Get system manager
    auto systemManager = engine.getSystemManager();
    
    // Create collision system
    auto collisionSystem = std::make_shared<CollisionSystem>(800.0f, 600.0f, 100.0f);
    systemManager->addSystem(collisionSystem);
    
    // Create test system
    auto testSystem = std::make_shared<CollisionTestSystem>(collisionSystem);
    systemManager->addSystem(testSystem);
    
    // Create renderer
    auto graphics = engine.getGraphicsAPI();
    auto shapeRenderer = std::make_shared<CollisionShapeRenderer>(graphics, collisionSystem);
    systemManager->addSystem(shapeRenderer);
    
    // Run engine
    engine.run();
    
    // Shutdown engine
    engine.shutdown();
    
    return 0;
}