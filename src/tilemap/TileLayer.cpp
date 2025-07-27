#include "TileLayer.h"
#include <algorithm>

namespace RPGEngine {
namespace Tilemap {

TileLayer::TileLayer(int width, int height, const LayerProperties& properties)
    : m_width(std::max(1, width))
    , m_height(std::max(1, height))
    , m_properties(properties)
{
    // Initialize tile data
    m_tiles.resize(m_width * m_height);
}

TileLayer::~TileLayer() {
    // Nothing to do here
}

const Tile* TileLayer::getTile(int x, int y) const {
    if (!isInBounds(x, y)) {
        return nullptr;
    }
    
    return &m_tiles[y * m_width + x];
}

bool TileLayer::setTile(int x, int y, const Tile& tile) {
    if (!isInBounds(x, y)) {
        return false;
    }
    
    m_tiles[y * m_width + x] = tile;
    return true;
}

bool TileLayer::clearTile(int x, int y) {
    if (!isInBounds(x, y)) {
        return false;
    }
    
    m_tiles[y * m_width + x] = Tile();
    return true;
}

void TileLayer::clearAllTiles() {
    std::fill(m_tiles.begin(), m_tiles.end(), Tile());
}

void TileLayer::resize(int width, int height, bool preserveData) {
    width = std::max(1, width);
    height = std::max(1, height);
    
    if (width == m_width && height == m_height) {
        return;
    }
    
    if (preserveData) {
        // Create new tile data
        std::vector<Tile> newTiles(width * height);
        
        // Copy existing data
        int minWidth = std::min(width, m_width);
        int minHeight = std::min(height, m_height);
        
        for (int y = 0; y < minHeight; ++y) {
            for (int x = 0; x < minWidth; ++x) {
                newTiles[y * width + x] = m_tiles[y * m_width + x];
            }
        }
        
        // Swap data
        m_tiles.swap(newTiles);
    } else {
        // Just resize and clear
        m_tiles.resize(width * height);
        clearAllTiles();
    }
    
    // Update dimensions
    m_width = width;
    m_height = height;
}

bool TileLayer::isInBounds(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

} // namespace Tilemap
} // namespace RPGEngine