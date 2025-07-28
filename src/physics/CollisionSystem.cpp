#include "CollisionSystem.h"
#include <iostream>
#include <algorithm>
#include "../core/Event.h"

namespace RPGEngine {
namespace Physics {

// Helper function to create a unique key for a collision pair
uint64_t createCollisionPairKey(uint32_t id1, uint32_t id2) {
    // Ensure consistent ordering (smaller ID first)
    if (id1 > id2) {
        std::swap(id1, id2);
    }
    
    return (static_cast<uint64_t>(id1) << 32) | static_cast<uint64_t>(id2);
}

CollisionSystem::CollisionSystem(float worldWidth, float worldHeight, float cellSize)
    : System("CollisionSystem")
    , m_worldWidth(worldWidth)
    , m_worldHeight(worldHeight)
    , m_cellSize(cellSize)
    , m_collisionCount(0)
    , m_generateCollisionEvents(true)
{
}

CollisionSystem::~CollisionSystem() {
    if (isInitialized()) {
        shutdown();
    }
}

bool CollisionSystem::onInitialize() {
    // Create spatial partitioning system
    m_spatialPartitioning = std::make_shared<GridPartitioning>(m_cellSize, m_worldWidth, m_worldHeight);
    
    if (!m_spatialPartitioning->initialize()) {
        std::cerr << "Failed to initialize spatial partitioning system" << std::endl;
        return false;
    }
    
    std::cout << "CollisionSystem initialized" << std::endl;
    return true;
}

void CollisionSystem::onUpdate(float deltaTime) {
    if (!m_spatialPartitioning) {
        return;
    }
    
    // Update spatial partitioning
    m_spatialPartitioning->update();
    
    // Reset collision count
    m_collisionCount = 0;
    
    // Store current collisions
    std::unordered_map<uint64_t, bool> currentCollisions;
    
    // Check collisions between collidables
    size_t collidableCount = m_spatialPartitioning->getCollidableCount();
    std::vector<std::shared_ptr<ICollidable>> collidables;
    
    // Query all collidables
    m_spatialPartitioning->queryRegion(CircleShape(std::max(m_worldWidth, m_worldHeight)), 
                                     [&collidables](std::shared_ptr<ICollidable> collidable) {
        collidables.push_back(collidable);
    });
    
    // Check collisions between collidables
    for (size_t i = 0; i < collidables.size(); ++i) {
        auto& collidable1 = collidables[i];
        
        // Get potential collisions
        std::vector<std::shared_ptr<ICollidable>> potentialCollisions = 
            m_spatialPartitioning->getPotentialCollisions(collidable1);
        
        for (auto& collidable2 : potentialCollisions) {
            // Skip self-collision
            if (collidable1->getCollidableID() == collidable2->getCollidableID()) {
                continue;
            }
            
            // Create collision pair key
            uint64_t pairKey = createCollisionPairKey(collidable1->getCollidableID(), collidable2->getCollidableID());
            
            // Skip if already checked
            if (currentCollisions.find(pairKey) != currentCollisions.end()) {
                continue;
            }
            
            // Check collision
            CollisionResult result;
            bool colliding = checkCollision(collidable1->getCollisionShape(), collidable2->getCollisionShape(), &result);
            
            // Store collision state
            currentCollisions[pairKey] = colliding;
            
            if (colliding) {
                m_collisionCount++;
                
                // Check if collision events should be generated
                if (m_generateCollisionEvents) {
                    bool wasColliding = false;
                    
                    // Check if this pair was colliding in the previous update
                    auto it = m_previousCollisions.find(pairKey);
                    if (it != m_previousCollisions.end()) {
                        wasColliding = it->second;
                    }
                    
                    // Generate collision event
                    if (!wasColliding) {
                        // New collision
                        CollisionEvent event(collidable1, collidable2, result);
                        
                    }
                }
            } else {
                // Check if this pair was colliding in the previous update
                auto it = m_previousCollisions.find(pairKey);
                if (it != m_previousCollisions.end() && it->second) {
                    // Collision ended
                    // TODO: Generate collision end event if needed
                }
            }
        }
    }
    
    // Store current collisions for the next update
    m_previousCollisions = currentCollisions;
}

void CollisionSystem::onShutdown() {
    if (m_spatialPartitioning) {
        m_spatialPartitioning->shutdown();
        m_spatialPartitioning.reset();
    }
    
    m_previousCollisions.clear();
    
    std::cout << "CollisionSystem shutdown" << std::endl;
}

void CollisionSystem::registerCollidable(std::shared_ptr<ICollidable> collidable) {
    if (!m_spatialPartitioning || !collidable) {
        return;
    }
    
    m_spatialPartitioning->addCollidable(collidable);
}

bool CollisionSystem::unregisterCollidable(std::shared_ptr<ICollidable> collidable) {
    if (!m_spatialPartitioning || !collidable) {
        return false;
    }
    
    return m_spatialPartitioning->removeCollidable(collidable);
}

bool CollisionSystem::unregisterCollidable(uint32_t collidableID) {
    if (!m_spatialPartitioning) {
        return false;
    }
    
    return m_spatialPartitioning->removeCollidable(collidableID);
}

void CollisionSystem::updateCollidable(std::shared_ptr<ICollidable> collidable) {
    if (!m_spatialPartitioning || !collidable) {
        return;
    }
    
    m_spatialPartitioning->updateCollidable(collidable);
}

bool CollisionSystem::pointInShape(const Vector2& point, const CollisionShape& shape) const {
    return CollisionDetection::pointInShape(point, shape);
}

bool CollisionSystem::checkCollision(const CollisionShape& shape1, const CollisionShape& shape2, CollisionResult* result) const {
    return CollisionDetection::checkCollision(shape1, shape2, result);
}

std::shared_ptr<ICollidable> CollisionSystem::checkCollision(const CollisionShape& shape, uint32_t layer, uint32_t mask, 
                                                          CollisionResult* result, uint32_t excludeID) const {
    if (!m_spatialPartitioning) {
        return nullptr;
    }
    
    // Get potential collisions
    std::vector<std::shared_ptr<ICollidable>> potentialCollisions = 
        m_spatialPartitioning->getPotentialCollisions(shape, layer, mask);
    
    // Check collisions
    CollisionResult bestResult;
    float closestDistance = std::numeric_limits<float>::max();
    std::shared_ptr<ICollidable> closestCollidable = nullptr;
    
    for (auto& collidable : potentialCollisions) {
        // Skip excluded collidable
        if (collidable->getCollidableID() == excludeID) {
            continue;
        }
        
        // Check collision
        CollisionResult currentResult;
        bool colliding = checkCollision(shape, collidable->getCollisionShape(), &currentResult);
        
        if (colliding) {
            // Calculate distance
            float distance = (currentResult.contactPoint - shape.getPosition()).lengthSquared();
            
            // Update closest collision
            if (distance < closestDistance) {
                closestDistance = distance;
                bestResult = currentResult;
                closestCollidable = collidable;
            }
        }
    }
    
    // Copy result if requested
    if (result && closestCollidable) {
        *result = bestResult;
    }
    
    return closestCollidable;
}

std::vector<std::shared_ptr<ICollidable>> CollisionSystem::getAllCollisions(const CollisionShape& shape, uint32_t layer, uint32_t mask, 
                                                                         uint32_t excludeID) const {
    if (!m_spatialPartitioning) {
        return {};
    }
    
    std::vector<std::shared_ptr<ICollidable>> result;
    
    // Get potential collisions
    std::vector<std::shared_ptr<ICollidable>> potentialCollisions = 
        m_spatialPartitioning->getPotentialCollisions(shape, layer, mask);
    
    // Check collisions
    for (auto& collidable : potentialCollisions) {
        // Skip excluded collidable
        if (collidable->getCollidableID() == excludeID) {
            continue;
        }
        
        // Check collision
        if (checkCollision(shape, collidable->getCollisionShape())) {
            result.push_back(collidable);
        }
    }
    
    return result;
}

std::shared_ptr<ICollidable> CollisionSystem::rayCast(const Vector2& start, const Vector2& direction, float maxDistance, 
                                                   uint32_t layer, uint32_t mask, CollisionResult* result, 
                                                   uint32_t excludeID) const {
    if (!m_spatialPartitioning) {
        return nullptr;
    }
    
    // Normalize direction
    Vector2 normalizedDirection = direction;
    float length = direction.length();
    if (length > 0.0001f) {
        normalizedDirection = direction / length;
    }
    
    // Calculate end point
    Vector2 end = start + normalizedDirection * maxDistance;
    
    // Create a rectangle shape that covers the ray
    Vector2 min(std::min(start.x, end.x), std::min(start.y, end.y));
    Vector2 max(std::max(start.x, end.x), std::max(start.y, end.y));
    
    float width = max.x - min.x;
    float height = max.y - min.y;
    
    // Ensure minimum size
    if (width < 1.0f) width = 1.0f;
    if (height < 1.0f) height = 1.0f;
    
    RectangleShape rayBounds(width, height);
    rayBounds.setPosition(min.x + width * 0.5f, min.y + height * 0.5f);
    
    // Get potential collisions
    std::vector<std::shared_ptr<ICollidable>> potentialCollisions = 
        m_spatialPartitioning->getPotentialCollisions(rayBounds, layer, mask);
    
    // Check ray intersections
    float closestDistance = maxDistance;
    std::shared_ptr<ICollidable> closestCollidable = nullptr;
    CollisionResult bestResult;
    
    for (auto& collidable : potentialCollisions) {
        // Skip excluded collidable
        if (collidable->getCollidableID() == excludeID) {
            continue;
        }
        
        // TODO: Implement proper ray-shape intersection
        // For now, just check if the ray passes through the shape's bounding circle
        
        const CollisionShape& shape = collidable->getCollisionShape();
        Vector2 shapePos = shape.getPosition();
        float radius = shape.getBoundingRadius();
        
        // Calculate closest point on ray to shape center
        Vector2 toShape = shapePos - start;
        float projectionLength = toShape.dot(normalizedDirection);
        
        // If the closest point is behind the ray start or beyond the max distance, skip
        if (projectionLength < 0.0f || projectionLength > maxDistance) {
            continue;
        }
        
        // Calculate closest point on ray to shape center
        Vector2 closestPoint = start + normalizedDirection * projectionLength;
        
        // Calculate distance from closest point to shape center
        float distanceToCenter = (closestPoint - shapePos).length();
        
        // Check if ray intersects shape
        if (distanceToCenter <= radius) {
            // Calculate intersection points
            float halfChordLength = std::sqrt(radius * radius - distanceToCenter * distanceToCenter);
            float intersection1 = projectionLength - halfChordLength;
            float intersection2 = projectionLength + halfChordLength;
            
            // Use the closest intersection point that is in front of the ray
            float intersectionDistance = (intersection1 >= 0.0f) ? intersection1 : intersection2;
            
            // Update closest intersection
            if (intersectionDistance >= 0.0f && intersectionDistance < closestDistance) {
                closestDistance = intersectionDistance;
                closestCollidable = collidable;
                
                // Calculate intersection point
                Vector2 intersectionPoint = start + normalizedDirection * intersectionDistance;
                
                // Create collision result
                bestResult.colliding = true;
                bestResult.normal = (intersectionPoint - shapePos).normalized();
                bestResult.penetration = radius - distanceToCenter;
                bestResult.contactPoint = intersectionPoint;
            }
        }
    }
    
    // Copy result if requested
    if (result && closestCollidable) {
        *result = bestResult;
    }
    
    return closestCollidable;
}

std::vector<std::shared_ptr<ICollidable>> CollisionSystem::rayCastAll(const Vector2& start, const Vector2& direction, float maxDistance, 
                                                                   uint32_t layer, uint32_t mask, uint32_t excludeID) const {
    if (!m_spatialPartitioning) {
        return {};
    }
    
    std::vector<std::shared_ptr<ICollidable>> result;
    
    // Normalize direction
    Vector2 normalizedDirection = direction;
    float length = direction.length();
    if (length > 0.0001f) {
        normalizedDirection = direction / length;
    }
    
    // Calculate end point
    Vector2 end = start + normalizedDirection * maxDistance;
    
    // Create a rectangle shape that covers the ray
    Vector2 min(std::min(start.x, end.x), std::min(start.y, end.y));
    Vector2 max(std::max(start.x, end.x), std::max(start.y, end.y));
    
    float width = max.x - min.x;
    float height = max.y - min.y;
    
    // Ensure minimum size
    if (width < 1.0f) width = 1.0f;
    if (height < 1.0f) height = 1.0f;
    
    RectangleShape rayBounds(width, height);
    rayBounds.setPosition(min.x + width * 0.5f, min.y + height * 0.5f);
    
    // Get potential collisions
    std::vector<std::shared_ptr<ICollidable>> potentialCollisions = 
        m_spatialPartitioning->getPotentialCollisions(rayBounds, layer, mask);
    
    // Check ray intersections
    for (auto& collidable : potentialCollisions) {
        // Skip excluded collidable
        if (collidable->getCollidableID() == excludeID) {
            continue;
        }
        
        // TODO: Implement proper ray-shape intersection
        // For now, just check if the ray passes through the shape's bounding circle
        
        const CollisionShape& shape = collidable->getCollisionShape();
        Vector2 shapePos = shape.getPosition();
        float radius = shape.getBoundingRadius();
        
        // Calculate closest point on ray to shape center
        Vector2 toShape = shapePos - start;
        float projectionLength = toShape.dot(normalizedDirection);
        
        // If the closest point is behind the ray start or beyond the max distance, skip
        if (projectionLength < 0.0f || projectionLength > maxDistance) {
            continue;
        }
        
        // Calculate closest point on ray to shape center
        Vector2 closestPoint = start + normalizedDirection * projectionLength;
        
        // Calculate distance from closest point to shape center
        float distanceToCenter = (closestPoint - shapePos).length();
        
        // Check if ray intersects shape
        if (distanceToCenter <= radius) {
            result.push_back(collidable);
        }
    }
    
    return result;
}

void CollisionSystem::queryRegion(const CollisionShape& shape, 
                               const std::function<void(std::shared_ptr<ICollidable>)>& callback) const {
    if (!m_spatialPartitioning || !callback) {
        return;
    }
    
    m_spatialPartitioning->queryRegion(shape, callback);
}

size_t CollisionSystem::getCollidableCount() const {
    if (!m_spatialPartitioning) {
        return 0;
    }
    
    return m_spatialPartitioning->getCollidableCount();
}

} // namespace Physics
} // namespace RPGEngine