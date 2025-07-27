#include "Tileset.h"
#include <algorithm>

namespace RPGEngine {
namespace Tilemap {

Tileset::Tileset(const std::string& name, int tileWidth, int tileHeight, int spacing, int margin)
    : m_name(name)
    , m_tileWidth(std::max(1, tileWidth))
    , m_tileHeight(std::max(1, tileHeight))
    , m_spacing(std::max(0, spacing))
    , m_margin(std::max(0, margin))
    , m_tileCount(0)
    , m_columns(0)
{
}

Tileset::~Tileset() {
    // Nothing to do here
}

void Tileset::setTexture(std::shared_ptr<Resources::TextureResource> texture) {
    m_texture = texture;
    
    if (m_texture && m_texture->isLoaded()) {
        // Calculate number of columns and rows
        int textureWidth = m_texture->getWidth();
        int textureHeight = m_texture->getHeight();
        
        // Calculate usable width and height (accounting for margin)
        int usableWidth = textureWidth - 2 * m_margin;
        int usableHeight = textureHeight - 2 * m_margin;
        
        // Calculate number of columns and rows
        m_columns = usableWidth / (m_tileWidth + m_spacing);
        int rows = usableHeight / (m_tileHeight + m_spacing);
        
        // Calculate total number of tiles
        m_tileCount = m_columns * rows;
    } else {
        m_columns = 0;
        m_tileCount = 0;
    }
}

const TileAnimation* Tileset::getAnimation(uint32_t tileId) const {
    auto it = m_animations.find(tileId);
    if (it != m_animations.end()) {
        return &it->second;
    }
    
    return nullptr;
}

void Tileset::setAnimation(uint32_t tileId, const TileAnimation& animation) {
    m_animations[tileId] = animation;
}

bool Tileset::removeAnimation(uint32_t tileId) {
    auto it = m_animations.find(tileId);
    if (it != m_animations.end()) {
        m_animations.erase(it);
        return true;
    }
    
    return false;
}

uint32_t Tileset::getTileFlags(uint32_t tileId) const {
    auto it = m_tileFlags.find(tileId);
    if (it != m_tileFlags.end()) {
        return it->second;
    }
    
    return TileFlags::None;
}

void Tileset::setTileFlags(uint32_t tileId, uint32_t flags) {
    m_tileFlags[tileId] = flags;
}

bool Tileset::getTileSourceRect(uint32_t tileId, int& x, int& y, int& width, int& height) const {
    if (!m_texture || tileId >= m_tileCount) {
        return false;
    }
    
    // Calculate tile position in the tileset
    int column = tileId % m_columns;
    int row = tileId / m_columns;
    
    // Calculate source rectangle
    x = m_margin + column * (m_tileWidth + m_spacing);
    y = m_margin + row * (m_tileHeight + m_spacing);
    width = m_tileWidth;
    height = m_tileHeight;
    
    return true;
}

} // namespace Tilemap
} // namespace RPGEngine