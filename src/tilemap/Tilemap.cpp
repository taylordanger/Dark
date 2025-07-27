#include "Tilemap.h"
#include <algorithm>

namespace RPGEngine {
namespace Tilemap {

Tilemap::Tilemap(const MapProperties& properties)
    : m_properties(properties)
{
}

Tilemap::~Tilemap() {
    // Nothing to do here
}

std::shared_ptr<TileLayer> Tilemap::getLayer(size_t index) const {
    if (index >= m_layers.size()) {
        return nullptr;
    }
    
    return m_layers[index];
}

std::shared_ptr<TileLayer> Tilemap::getLayerByName(const std::string& name) const {
    for (const auto& layer : m_layers) {
        if (layer->getProperties().name == name) {
            return layer;
        }
    }
    
    return nullptr;
}

size_t Tilemap::addLayer(std::shared_ptr<TileLayer> layer) {
    if (!layer) {
        return m_layers.size();
    }
    
    m_layers.push_back(layer);
    return m_layers.size() - 1;
}

bool Tilemap::insertLayer(size_t index, std::shared_ptr<TileLayer> layer) {
    if (!layer || index > m_layers.size()) {
        return false;
    }
    
    m_layers.insert(m_layers.begin() + index, layer);
    return true;
}

bool Tilemap::removeLayer(size_t index) {
    if (index >= m_layers.size()) {
        return false;
    }
    
    m_layers.erase(m_layers.begin() + index);
    return true;
}

bool Tilemap::removeLayerByName(const std::string& name) {
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if ((*it)->getProperties().name == name) {
            m_layers.erase(it);
            return true;
        }
    }
    
    return false;
}

bool Tilemap::moveLayer(size_t fromIndex, size_t toIndex) {
    if (fromIndex >= m_layers.size() || toIndex >= m_layers.size()) {
        return false;
    }
    
    if (fromIndex == toIndex) {
        return true;
    }
    
    auto layer = m_layers[fromIndex];
    m_layers.erase(m_layers.begin() + fromIndex);
    
    if (toIndex > fromIndex) {
        toIndex--;
    }
    
    m_layers.insert(m_layers.begin() + toIndex, layer);
    return true;
}

std::shared_ptr<Tileset> Tilemap::getTileset(size_t index) const {
    if (index >= m_tilesets.size()) {
        return nullptr;
    }
    
    return m_tilesets[index];
}

std::shared_ptr<Tileset> Tilemap::getTilesetByName(const std::string& name) const {
    for (const auto& tileset : m_tilesets) {
        if (tileset->getName() == name) {
            return tileset;
        }
    }
    
    return nullptr;
}

size_t Tilemap::addTileset(std::shared_ptr<Tileset> tileset) {
    if (!tileset) {
        return m_tilesets.size();
    }
    
    // Calculate first GID for the new tileset
    uint32_t firstGid = 1;
    if (!m_tilesets.empty()) {
        size_t lastIndex = m_tilesets.size() - 1;
        firstGid = m_firstGids[lastIndex] + m_tilesets[lastIndex]->getTileCount();
    }
    
    m_tilesets.push_back(tileset);
    m_firstGids.push_back(firstGid);
    
    return m_tilesets.size() - 1;
}

bool Tilemap::removeTileset(size_t index) {
    if (index >= m_tilesets.size()) {
        return false;
    }
    
    m_tilesets.erase(m_tilesets.begin() + index);
    m_firstGids.erase(m_firstGids.begin() + index);
    
    // Update first GIDs for subsequent tilesets
    for (size_t i = index; i < m_tilesets.size(); ++i) {
        if (i == 0) {
            m_firstGids[i] = 1;
        } else {
            m_firstGids[i] = m_firstGids[i - 1] + m_tilesets[i - 1]->getTileCount();
        }
    }
    
    return true;
}

bool Tilemap::removeTilesetByName(const std::string& name) {
    for (size_t i = 0; i < m_tilesets.size(); ++i) {
        if (m_tilesets[i]->getName() == name) {
            return removeTileset(i);
        }
    }
    
    return false;
}

bool Tilemap::findTilesetAndLocalId(uint32_t globalTileId, std::shared_ptr<Tileset>& tileset, uint32_t& localTileId) const {
    if (m_tilesets.empty() || globalTileId == 0) {
        return false;
    }
    
    // Find the tileset that contains the tile
    for (int i = static_cast<int>(m_tilesets.size()) - 1; i >= 0; --i) {
        if (globalTileId >= m_firstGids[i]) {
            tileset = m_tilesets[i];
            localTileId = globalTileId - m_firstGids[i];
            
            // Check if the local ID is valid
            if (localTileId < tileset->getTileCount()) {
                return true;
            }
            
            break;
        }
    }
    
    return false;
}

void Tilemap::clear() {
    m_layers.clear();
    m_tilesets.clear();
    m_firstGids.clear();
}

} // namespace Tilemap
} // namespace RPGEngine