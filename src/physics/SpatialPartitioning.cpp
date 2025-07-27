#include "SpatialPartitioning.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>

namespace RPGEngine {
namespace Physics {

GridPartitioning::GridPartitioning(float cellSize, float worldWidth, float worldHeight)
    : m_cellSize(cellSize)
    , m_worldWidth(worldWidth)
    , m_worldHeight(worldHeight)
    , m_gridWidth(0)
    , m_gridHeight(0)
    , m_initialized(false)
{
}

GridPartitioning::~GridPartitioning() {
    if (m_initialized) {
        shutdown();
    }
}

bool GridPartitioning::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Calculate grid dimensions
    m_gridWidth = static_cast<int>(std::ceil(m_worldWidth / m_cellSize));
    m_gridHeight = static_cast<int>(std::ceil(m_worldHeight / m_cellSize));
    
    // Create grid cells
    m_cells.resize(m_gridWidth * m_gridHeight);
    
    m_initialized = true;
    std::cout << "GridPartitioning initialized with " << m_gridWidth << "x" << m_gridHeight << " cells" << std::endl;
    return true;
}

void GridPartitioning::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Clear all cells
    clear();
    
    m_initialized = false;
    std::cout << "GridPartitioning shutdown" << std::endl;
}

void GridPartitioning::update() {
    if (!m_initialized) {
        return;
    }
    
    // Clear all cells
    for (auto& cell : m_cells) {
        cell.collidables.clear();
    }
    
    // Re-add all collidables
    for (const auto& pair : m_collidables) {
        const auto& collidable = pair.second;
        const CollisionShape& shape = collidable->getCollisionShape();
        
        // Get overlapping cells
        int minCellX, minCellY, maxCellX, maxCellY;
        if (getOverlappingCells(shape, minCellX, minCellY, maxCellX, maxCellY)) {
            // Add collidable to each overlapping cell
            for (int y = minCellY; y <= maxCellY; ++y) {
                for (int x = minCellX; x <= maxCellX; ++x) {
                    GridCell* cell = getCell(x, y);
                    if (cell) {
                        cell->collidables.push_back(collidable);
                    }
                }
            }
        }
    }
}

void GridPartitioning::addCollidable(std::shared_ptr<ICollidable> collidable) {
    if (!m_initialized || !collidable) {
        return;
    }
    
    uint32_t collidableID = collidable->getCollidableID();
    
    // Check if collidable already exists
    if (m_collidables.find(collidableID) != m_collidables.end()) {
        return;
    }
    
    // Add collidable to the map
    m_collidables[collidableID] = collidable;
    
    // Add collidable to cells
    const CollisionShape& shape = collidable->getCollisionShape();
    
    // Get overlapping cells
    int minCellX, minCellY, maxCellX, maxCellY;
    if (getOverlappingCells(shape, minCellX, minCellY, maxCellX, maxCellY)) {
        // Add collidable to each overlapping cell
        for (int y = minCellY; y <= maxCellY; ++y) {
            for (int x = minCellX; x <= maxCellX; ++x) {
                GridCell* cell = getCell(x, y);
                if (cell) {
                    cell->collidables.push_back(collidable);
                }
            }
        }
    }
}

bool GridPartitioning::removeCollidable(std::shared_ptr<ICollidable> collidable) {
    if (!m_initialized || !collidable) {
        return false;
    }
    
    return removeCollidable(collidable->getCollidableID());
}

bool GridPartitioning::removeCollidable(uint32_t collidableID) {
    if (!m_initialized) {
        return false;
    }
    
    // Check if collidable exists
    auto it = m_collidables.find(collidableID);
    if (it == m_collidables.end()) {
        return false;
    }
    
    // Remove collidable from the map
    m_collidables.erase(it);
    
    // Remove collidable from cells (will be done in the next update)
    
    return true;
}

void GridPartitioning::updateCollidable(std::shared_ptr<ICollidable> collidable) {
    if (!m_initialized || !collidable) {
        return;
    }
    
    // Remove and re-add the collidable
    removeCollidable(collidable);
    addCollidable(collidable);
}

std::vector<std::shared_ptr<ICollidable>> GridPartitioning::getPotentialCollisions(std::shared_ptr<ICollidable> collidable) {
    if (!m_initialized || !collidable) {
        return {};
    }
    
    return getPotentialCollisions(collidable->getCollisionShape(), 
                                collidable->getCollisionLayer(), 
                                collidable->getCollisionMask());
}

std::vector<std::shared_ptr<ICollidable>> GridPartitioning::getPotentialCollisions(const CollisionShape& shape, 
                                                                                uint32_t layer, uint32_t mask) {
    if (!m_initialized) {
        return {};
    }
    
    std::vector<std::shared_ptr<ICollidable>> result;
    std::unordered_set<uint32_t> addedCollidables;
    
    // Get overlapping cells
    int minCellX, minCellY, maxCellX, maxCellY;
    if (getOverlappingCells(shape, minCellX, minCellY, maxCellX, maxCellY)) {
        // Check collidables in each overlapping cell
        for (int y = minCellY; y <= maxCellY; ++y) {
            for (int x = minCellX; x <= maxCellX; ++x) {
                const GridCell* cell = getCell(x, y);
                if (!cell) {
                    continue;
                }
                
                for (const auto& other : cell->collidables) {
                    uint32_t otherID = other->getCollidableID();
                    
                    // Skip if already added
                    if (addedCollidables.find(otherID) != addedCollidables.end()) {
                        continue;
                    }
                    
                    // Check layer collision
                    if (!canLayersCollide(layer, mask, other->getCollisionLayer(), other->getCollisionMask())) {
                        continue;
                    }
                    
                    // Add to result
                    result.push_back(other);
                    addedCollidables.insert(otherID);
                }
            }
        }
    }
    
    return result;
}

void GridPartitioning::queryRegion(const CollisionShape& shape, 
                                const std::function<void(std::shared_ptr<ICollidable>)>& callback) {
    if (!m_initialized || !callback) {
        return;
    }
    
    std::unordered_set<uint32_t> processedCollidables;
    
    // Get overlapping cells
    int minCellX, minCellY, maxCellX, maxCellY;
    if (getOverlappingCells(shape, minCellX, minCellY, maxCellX, maxCellY)) {
        // Check collidables in each overlapping cell
        for (int y = minCellY; y <= maxCellY; ++y) {
            for (int x = minCellX; x <= maxCellX; ++x) {
                const GridCell* cell = getCell(x, y);
                if (!cell) {
                    continue;
                }
                
                for (const auto& collidable : cell->collidables) {
                    uint32_t collidableID = collidable->getCollidableID();
                    
                    // Skip if already processed
                    if (processedCollidables.find(collidableID) != processedCollidables.end()) {
                        continue;
                    }
                    
                    // Add to processed set
                    processedCollidables.insert(collidableID);
                    
                    // Call the callback
                    callback(collidable);
                }
            }
        }
    }
}

void GridPartitioning::clear() {
    if (!m_initialized) {
        return;
    }
    
    // Clear all cells
    for (auto& cell : m_cells) {
        cell.collidables.clear();
    }
    
    // Clear collidables map
    m_collidables.clear();
}

bool GridPartitioning::getCellCoords(const Vector2& position, int& cellX, int& cellY) const {
    // Convert position to cell coordinates
    cellX = static_cast<int>(position.x / m_cellSize);
    cellY = static_cast<int>(position.y / m_cellSize);
    
    // Check if within bounds
    return cellX >= 0 && cellX < m_gridWidth && cellY >= 0 && cellY < m_gridHeight;
}

bool GridPartitioning::getOverlappingCells(const CollisionShape& shape, int& minCellX, int& minCellY, int& maxCellX, int& maxCellY) const {
    // Get shape AABB
    Vector2 min, max;
    shape.getAABB(min, max);
    
    // Convert to cell coordinates
    minCellX = static_cast<int>(min.x / m_cellSize);
    minCellY = static_cast<int>(min.y / m_cellSize);
    maxCellX = static_cast<int>(max.x / m_cellSize);
    maxCellY = static_cast<int>(max.y / m_cellSize);
    
    // Clamp to grid bounds
    minCellX = std::max(0, std::min(minCellX, m_gridWidth - 1));
    minCellY = std::max(0, std::min(minCellY, m_gridHeight - 1));
    maxCellX = std::max(0, std::min(maxCellX, m_gridWidth - 1));
    maxCellY = std::max(0, std::min(maxCellY, m_gridHeight - 1));
    
    // Check if any cells are within bounds
    return minCellX <= maxCellX && minCellY <= maxCellY;
}

GridCell* GridPartitioning::getCell(int cellX, int cellY) {
    if (cellX < 0 || cellX >= m_gridWidth || cellY < 0 || cellY >= m_gridHeight) {
        return nullptr;
    }
    
    return &m_cells[cellY * m_gridWidth + cellX];
}

const GridCell* GridPartitioning::getCell(int cellX, int cellY) const {
    if (cellX < 0 || cellX >= m_gridWidth || cellY < 0 || cellY >= m_gridHeight) {
        return nullptr;
    }
    
    return &m_cells[cellY * m_gridWidth + cellX];
}

bool GridPartitioning::canLayersCollide(uint32_t layer1, uint32_t mask1, uint32_t layer2, uint32_t mask2) const {
    // Check if layer1 is in mask2 and layer2 is in mask1
    return (layer1 & mask2) != 0 && (layer2 & mask1) != 0;
}

} // namespace Physics
} // namespace RPGEngine