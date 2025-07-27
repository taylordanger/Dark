#pragma once

#include "CollisionShape.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace RPGEngine {
namespace Physics {

/**
 * Collidable interface
 * Represents an object that can collide with other objects
 */
class ICollidable {
public:
    /**
     * Virtual destructor
     */
    virtual ~ICollidable() = default;
    
    /**
     * Get the collision shape
     * @return Collision shape
     */
    virtual const CollisionShape& getCollisionShape() const = 0;
    
    /**
     * Get the collidable ID
     * @return Collidable ID
     */
    virtual uint32_t getCollidableID() const = 0;
    
    /**
     * Get the collision layer
     * @return Collision layer
     */
    virtual uint32_t getCollisionLayer() const = 0;
    
    /**
     * Get the collision mask
     * @return Collision mask
     */
    virtual uint32_t getCollisionMask() const = 0;
};

/**
 * Spatial partitioning interface
 * Base class for spatial partitioning systems
 */
class ISpatialPartitioning {
public:
    /**
     * Virtual destructor
     */
    virtual ~ISpatialPartitioning() = default;
    
    /**
     * Initialize the spatial partitioning system
     * @return true if initialization was successful
     */
    virtual bool initialize() = 0;
    
    /**
     * Shutdown the spatial partitioning system
     */
    virtual void shutdown() = 0;
    
    /**
     * Update the spatial partitioning system
     */
    virtual void update() = 0;
    
    /**
     * Add a collidable object
     * @param collidable Collidable object
     */
    virtual void addCollidable(std::shared_ptr<ICollidable> collidable) = 0;
    
    /**
     * Remove a collidable object
     * @param collidable Collidable object
     * @return true if the object was removed
     */
    virtual bool removeCollidable(std::shared_ptr<ICollidable> collidable) = 0;
    
    /**
     * Remove a collidable object by ID
     * @param collidableID Collidable ID
     * @return true if the object was removed
     */
    virtual bool removeCollidable(uint32_t collidableID) = 0;
    
    /**
     * Update a collidable object's position
     * @param collidable Collidable object
     */
    virtual void updateCollidable(std::shared_ptr<ICollidable> collidable) = 0;
    
    /**
     * Get potential collisions for a collidable object
     * @param collidable Collidable object
     * @return Vector of potentially colliding objects
     */
    virtual std::vector<std::shared_ptr<ICollidable>> getPotentialCollisions(std::shared_ptr<ICollidable> collidable) = 0;
    
    /**
     * Get potential collisions for a shape
     * @param shape Collision shape
     * @param layer Collision layer
     * @param mask Collision mask
     * @return Vector of potentially colliding objects
     */
    virtual std::vector<std::shared_ptr<ICollidable>> getPotentialCollisions(const CollisionShape& shape, 
                                                                           uint32_t layer, uint32_t mask) = 0;
    
    /**
     * Query objects in a region
     * @param shape Region shape
     * @param callback Callback function for each object in the region
     */
    virtual void queryRegion(const CollisionShape& shape, 
                           const std::function<void(std::shared_ptr<ICollidable>)>& callback) = 0;
    
    /**
     * Clear all collidable objects
     */
    virtual void clear() = 0;
    
    /**
     * Get the number of collidable objects
     * @return Number of collidable objects
     */
    virtual size_t getCollidableCount() const = 0;
};

/**
 * Grid cell structure
 * Represents a cell in the grid-based spatial partitioning system
 */
struct GridCell {
    std::vector<std::shared_ptr<ICollidable>> collidables;
};

/**
 * Grid-based spatial partitioning system
 */
class GridPartitioning : public ISpatialPartitioning {
public:
    /**
     * Constructor
     * @param cellSize Cell size
     * @param worldWidth World width
     * @param worldHeight World height
     */
    GridPartitioning(float cellSize, float worldWidth, float worldHeight);
    
    /**
     * Destructor
     */
    ~GridPartitioning();
    
    // ISpatialPartitioning interface implementation
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void addCollidable(std::shared_ptr<ICollidable> collidable) override;
    bool removeCollidable(std::shared_ptr<ICollidable> collidable) override;
    bool removeCollidable(uint32_t collidableID) override;
    void updateCollidable(std::shared_ptr<ICollidable> collidable) override;
    std::vector<std::shared_ptr<ICollidable>> getPotentialCollisions(std::shared_ptr<ICollidable> collidable) override;
    std::vector<std::shared_ptr<ICollidable>> getPotentialCollisions(const CollisionShape& shape, 
                                                                   uint32_t layer, uint32_t mask) override;
    void queryRegion(const CollisionShape& shape, 
                   const std::function<void(std::shared_ptr<ICollidable>)>& callback) override;
    void clear() override;
    size_t getCollidableCount() const override { return m_collidables.size(); }
    
private:
    /**
     * Get the cell coordinates for a position
     * @param position Position
     * @param cellX Output cell X coordinate
     * @param cellY Output cell Y coordinate
     * @return true if the position is within the grid bounds
     */
    bool getCellCoords(const Vector2& position, int& cellX, int& cellY) const;
    
    /**
     * Get the cells that a shape overlaps
     * @param shape Collision shape
     * @param minCellX Output minimum cell X coordinate
     * @param minCellY Output minimum cell Y coordinate
     * @param maxCellX Output maximum cell X coordinate
     * @param maxCellY Output maximum cell Y coordinate
     * @return true if the shape overlaps the grid
     */
    bool getOverlappingCells(const CollisionShape& shape, int& minCellX, int& minCellY, int& maxCellX, int& maxCellY) const;
    
    /**
     * Get a cell at the specified coordinates
     * @param cellX Cell X coordinate
     * @param cellY Cell Y coordinate
     * @return Pointer to the cell, or nullptr if out of bounds
     */
    GridCell* getCell(int cellX, int cellY);
    
    /**
     * Get a cell at the specified coordinates (const version)
     * @param cellX Cell X coordinate
     * @param cellY Cell Y coordinate
     * @return Pointer to the cell, or nullptr if out of bounds
     */
    const GridCell* getCell(int cellX, int cellY) const;
    
    /**
     * Check if two layers can collide
     * @param layer1 First layer
     * @param mask1 First mask
     * @param layer2 Second layer
     * @param mask2 Second mask
     * @return true if the layers can collide
     */
    bool canLayersCollide(uint32_t layer1, uint32_t mask1, uint32_t layer2, uint32_t mask2) const;
    
    // Grid properties
    float m_cellSize;
    float m_worldWidth;
    float m_worldHeight;
    int m_gridWidth;
    int m_gridHeight;
    
    // Grid cells
    std::vector<GridCell> m_cells;
    
    // Collidable objects
    std::unordered_map<uint32_t, std::shared_ptr<ICollidable>> m_collidables;
    
    // Initialization state
    bool m_initialized;
};

} // namespace Physics
} // namespace RPGEngine