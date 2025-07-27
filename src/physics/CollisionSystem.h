#pragma once

#include "CollisionShape.h"
#include "CollisionDetection.h"
#include "SpatialPartitioning.h"
#include "../systems/System.h"
#include "../core/Event.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace RPGEngine {
namespace Physics {

/**
 * Collision event structure
 * Contains information about a collision between two collidable objects
 */
struct CollisionEvent : public Event<CollisionEvent> {
    std::shared_ptr<ICollidable> collidable1;
    std::shared_ptr<ICollidable> collidable2;
    CollisionResult result;
    
    CollisionEvent(std::shared_ptr<ICollidable> collidable1, std::shared_ptr<ICollidable> collidable2, const CollisionResult& result)
        : collidable1(collidable1), collidable2(collidable2), result(result) {}
};

/**
 * Collision system
 * Manages collision detection and resolution
 */
class CollisionSystem : public System {
public:
    /**
     * Constructor
     * @param worldWidth World width
     * @param worldHeight World height
     * @param cellSize Cell size for spatial partitioning
     */
    CollisionSystem(float worldWidth, float worldHeight, float cellSize = 100.0f);
    
    /**
     * Destructor
     */
    ~CollisionSystem();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Register a collidable object
     * @param collidable Collidable object
     */
    void registerCollidable(std::shared_ptr<ICollidable> collidable);
    
    /**
     * Unregister a collidable object
     * @param collidable Collidable object
     * @return true if the object was unregistered
     */
    bool unregisterCollidable(std::shared_ptr<ICollidable> collidable);
    
    /**
     * Unregister a collidable object by ID
     * @param collidableID Collidable ID
     * @return true if the object was unregistered
     */
    bool unregisterCollidable(uint32_t collidableID);
    
    /**
     * Update a collidable object's position
     * @param collidable Collidable object
     */
    void updateCollidable(std::shared_ptr<ICollidable> collidable);
    
    /**
     * Check if a point is inside a shape
     * @param point Point to check
     * @param shape Shape to check against
     * @return true if the point is inside the shape
     */
    bool pointInShape(const Vector2& point, const CollisionShape& shape) const;
    
    /**
     * Check if two shapes are colliding
     * @param shape1 First shape
     * @param shape2 Second shape
     * @param result Optional collision result
     * @return true if the shapes are colliding
     */
    bool checkCollision(const CollisionShape& shape1, const CollisionShape& shape2, CollisionResult* result = nullptr) const;
    
    /**
     * Check if a shape is colliding with any registered collidable
     * @param shape Shape to check
     * @param layer Collision layer
     * @param mask Collision mask
     * @param result Optional collision result
     * @param excludeID Optional collidable ID to exclude
     * @return Collidable that the shape is colliding with, or nullptr if none
     */
    std::shared_ptr<ICollidable> checkCollision(const CollisionShape& shape, uint32_t layer, uint32_t mask, 
                                              CollisionResult* result = nullptr, uint32_t excludeID = 0) const;
    
    /**
     * Get all collidables that a shape is colliding with
     * @param shape Shape to check
     * @param layer Collision layer
     * @param mask Collision mask
     * @param excludeID Optional collidable ID to exclude
     * @return Vector of collidables that the shape is colliding with
     */
    std::vector<std::shared_ptr<ICollidable>> getAllCollisions(const CollisionShape& shape, uint32_t layer, uint32_t mask, 
                                                             uint32_t excludeID = 0) const;
    
    /**
     * Cast a ray and find the first collidable it hits
     * @param start Ray start position
     * @param direction Ray direction
     * @param maxDistance Maximum ray distance
     * @param layer Collision layer
     * @param mask Collision mask
     * @param result Optional collision result
     * @param excludeID Optional collidable ID to exclude
     * @return Collidable that the ray hit, or nullptr if none
     */
    std::shared_ptr<ICollidable> rayCast(const Vector2& start, const Vector2& direction, float maxDistance, 
                                       uint32_t layer, uint32_t mask, CollisionResult* result = nullptr, 
                                       uint32_t excludeID = 0) const;
    
    /**
     * Get all collidables that a ray intersects
     * @param start Ray start position
     * @param direction Ray direction
     * @param maxDistance Maximum ray distance
     * @param layer Collision layer
     * @param mask Collision mask
     * @param excludeID Optional collidable ID to exclude
     * @return Vector of collidables that the ray intersects
     */
    std::vector<std::shared_ptr<ICollidable>> rayCastAll(const Vector2& start, const Vector2& direction, float maxDistance, 
                                                       uint32_t layer, uint32_t mask, uint32_t excludeID = 0) const;
    
    /**
     * Query collidables in a region
     * @param shape Region shape
     * @param callback Callback function for each collidable in the region
     */
    void queryRegion(const CollisionShape& shape, 
                   const std::function<void(std::shared_ptr<ICollidable>)>& callback) const;
    
    /**
     * Get the spatial partitioning system
     * @return Spatial partitioning system
     */
    std::shared_ptr<ISpatialPartitioning> getSpatialPartitioning() const { return m_spatialPartitioning; }
    
    /**
     * Get the number of collidables
     * @return Number of collidables
     */
    size_t getCollidableCount() const;
    
    /**
     * Get the number of collisions detected in the last update
     * @return Number of collisions
     */
    size_t getCollisionCount() const { return m_collisionCount; }
    
    /**
     * Set whether to generate collision events
     * @param generate Whether to generate collision events
     */
    void setGenerateCollisionEvents(bool generate) { m_generateCollisionEvents = generate; }
    
    /**
     * Check if collision events are being generated
     * @return true if collision events are being generated
     */
    bool isGeneratingCollisionEvents() const { return m_generateCollisionEvents; }
    
private:
    // Spatial partitioning
    std::shared_ptr<ISpatialPartitioning> m_spatialPartitioning;
    
    // World dimensions
    float m_worldWidth;
    float m_worldHeight;
    float m_cellSize;
    
    // Collision tracking
    size_t m_collisionCount;
    bool m_generateCollisionEvents;
    
    // Collision pairs from the previous update
    std::unordered_map<uint64_t, bool> m_previousCollisions;
};

} // namespace Physics
} // namespace RPGEngine