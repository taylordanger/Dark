#include "TilemapRenderer.h"
#include <iostream>

namespace RPGEngine {
namespace Tilemap {

TilemapRenderer::TilemapRenderer(std::shared_ptr<Graphics::IGraphicsAPI> graphics)
    : System("TilemapRenderer")
    , m_graphics(graphics)
    , m_useFrustumCulling(true)
    , m_renderColliders(false)
    , m_colliderColor(0xFF0000FF) // Red with full alpha
{
}

TilemapRenderer::~TilemapRenderer() {
    if (isInitialized()) {
        shutdown();
    }
}

bool TilemapRenderer::onInitialize() {
    if (!m_graphics) {
        std::cerr << "Graphics API not provided to TilemapRenderer" << std::endl;
        return false;
    }
    
    std::cout << "TilemapRenderer initialized" << std::endl;
    return true;
}

void TilemapRenderer::onUpdate(float deltaTime) {
    if (!m_tilemap || !m_graphics) {
        return;
    }
    
    // Update animations
    updateAnimations(deltaTime);
    
    // Render each layer
    for (size_t i = 0; i < m_tilemap->getLayerCount(); ++i) {
        auto layer = m_tilemap->getLayer(i);
        if (layer && layer->getProperties().visible) {
            renderLayer(layer);
            
            // Render colliders if enabled
            if (m_renderColliders) {
                renderColliders(layer);
            }
        }
    }
}

void TilemapRenderer::onShutdown() {
    // Clear animation states
    m_animationStates.clear();
    
    std::cout << "TilemapRenderer shutdown" << std::endl;
}

void TilemapRenderer::updateAnimations(float deltaTime) {
    if (!m_tilemap) {
        return;
    }
    
    // Update animation states for each tileset
    for (size_t i = 0; i < m_tilemap->getTilesetCount(); ++i) {
        auto tileset = m_tilemap->getTileset(i);
        if (!tileset) {
            continue;
        }
        
        // Get first GID for this tileset
        uint32_t firstGid = 1;
        if (i > 0) {
            auto prevTileset = m_tilemap->getTileset(i - 1);
            firstGid = firstGid + prevTileset->getTileCount();
        }
        
        // Update animations for each tile
        for (uint32_t tileId = 0; tileId < tileset->getTileCount(); ++tileId) {
            const TileAnimation* animation = tileset->getAnimation(tileId);
            if (!animation || animation->frames.empty()) {
                continue;
            }
            
            // Get global tile ID
            uint32_t globalTileId = firstGid + tileId;
            
            // Get or create animation state
            auto& state = m_animationStates[globalTileId];
            
            // Update animation time
            state.time += deltaTime * 1000.0f; // Convert to milliseconds
            
            // Get current frame duration
            uint32_t frameDuration = animation->frames[state.frameIndex].duration;
            
            // Check if we need to advance to the next frame
            while (state.time >= frameDuration) {
                state.time -= frameDuration;
                state.frameIndex = (state.frameIndex + 1) % animation->frames.size();
                frameDuration = animation->frames[state.frameIndex].duration;
            }
        }
    }
}

void TilemapRenderer::renderLayer(std::shared_ptr<TileLayer> layer) {
    if (!layer || !m_tilemap) {
        return;
    }
    
    const auto& properties = m_tilemap->getProperties();
    const auto& layerProps = layer->getProperties();
    
    // Calculate layer offset
    float offsetX = layerProps.offsetX;
    float offsetY = layerProps.offsetY;
    
    // Apply parallax if camera is available
    if (m_camera) {
        offsetX += m_camera->getPosition().x * (1.0f - layerProps.parallaxX);
        offsetY += m_camera->getPosition().y * (1.0f - layerProps.parallaxY);
    }
    
    // Calculate visible tile range if using frustum culling
    int startX = 0;
    int startY = 0;
    int endX = layer->getWidth();
    int endY = layer->getHeight();
    
    if (m_useFrustumCulling && m_camera) {
        // Calculate visible tile range based on camera frustum
        float viewLeft = m_camera->getPosition().x - m_camera->getViewport().width * 0.5f;
        float viewRight = m_camera->getPosition().x + m_camera->getViewport().width * 0.5f;
        float viewTop = m_camera->getPosition().y - m_camera->getViewport().height * 0.5f;
        float viewBottom = m_camera->getPosition().y + m_camera->getViewport().height * 0.5f;
        
        // Convert to tile coordinates
        startX = static_cast<int>((viewLeft - offsetX) / properties.tileWidth) - 1;
        startY = static_cast<int>((viewTop - offsetY) / properties.tileHeight) - 1;
        endX = static_cast<int>((viewRight - offsetX) / properties.tileWidth) + 2;
        endY = static_cast<int>((viewBottom - offsetY) / properties.tileHeight) + 2;
        
        // Clamp to layer bounds
        startX = std::max(0, std::min(startX, layer->getWidth()));
        startY = std::max(0, std::min(startY, layer->getHeight()));
        endX = std::max(0, std::min(endX, layer->getWidth()));
        endY = std::max(0, std::min(endY, layer->getHeight()));
    }
    
    // Render visible tiles
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Tile* tile = layer->getTile(x, y);
            if (tile && tile->id != 0) {
                renderTile(x, y, *tile, layer);
            }
        }
    }
}

void TilemapRenderer::renderTile(int x, int y, const Tile& tile, std::shared_ptr<TileLayer> layer) {
    if (!m_tilemap) {
        return;
    }
    
    const auto& properties = m_tilemap->getProperties();
    const auto& layerProps = layer->getProperties();
    
    // Find tileset and local tile ID
    std::shared_ptr<Tileset> tileset;
    uint32_t localTileId;
    
    if (!m_tilemap->findTilesetAndLocalId(tile.id, tileset, localTileId)) {
        return;
    }
    
    // Check if tile is animated
    if (tileset->getAnimation(localTileId)) {
        localTileId = getCurrentAnimationFrame(tile.id, tileset);
    }
    
    // Get tile source rectangle
    int srcX, srcY, srcWidth, srcHeight;
    if (!tileset->getTileSourceRect(localTileId, srcX, srcY, srcWidth, srcHeight)) {
        return;
    }
    
    // Calculate destination rectangle
    float destX = x * properties.tileWidth + layerProps.offsetX;
    float destY = y * properties.tileHeight + layerProps.offsetY;
    float destWidth = properties.tileWidth;
    float destHeight = properties.tileHeight;
    
    // Apply parallax if camera is available
    if (m_camera) {
        destX += m_camera->getPosition().x * (1.0f - layerProps.parallaxX);
        destY += m_camera->getPosition().y * (1.0f - layerProps.parallaxY);
    }
    
    // Apply flip and rotation
    bool flipX = tile.isFlippedH();
    bool flipY = tile.isFlippedV();
    float rotation = 0.0f;
    
    if (tile.isRotated90()) {
        rotation = 90.0f;
    } else if (tile.isRotated180()) {
        rotation = 180.0f;
    } else if (tile.isRotated270()) {
        rotation = 270.0f;
    }
    
    // Draw tile
    m_graphics->drawTexture(
        tileset->getTexture()->getHandle(),
        destX, destY, destWidth, destHeight,
        srcX, srcY, srcWidth, srcHeight,
        rotation, flipX, flipY,
        0xFFFFFFFF, // White with full alpha
        layerProps.opacity
    );
}

void TilemapRenderer::renderColliders(std::shared_ptr<TileLayer> layer) {
    if (!layer || !m_tilemap) {
        return;
    }
    
    const auto& properties = m_tilemap->getProperties();
    const auto& layerProps = layer->getProperties();
    
    // Calculate layer offset
    float offsetX = layerProps.offsetX;
    float offsetY = layerProps.offsetY;
    
    // Apply parallax if camera is available
    if (m_camera) {
        offsetX += m_camera->getPosition().x * (1.0f - layerProps.parallaxX);
        offsetY += m_camera->getPosition().y * (1.0f - layerProps.parallaxY);
    }
    
    // Calculate visible tile range if using frustum culling
    int startX = 0;
    int startY = 0;
    int endX = layer->getWidth();
    int endY = layer->getHeight();
    
    if (m_useFrustumCulling && m_camera) {
        // Calculate visible tile range based on camera frustum
        float viewLeft = m_camera->getPosition().x - m_camera->getViewport().width * 0.5f;
        float viewRight = m_camera->getPosition().x + m_camera->getViewport().width * 0.5f;
        float viewTop = m_camera->getPosition().y - m_camera->getViewport().height * 0.5f;
        float viewBottom = m_camera->getPosition().y + m_camera->getViewport().height * 0.5f;
        
        // Convert to tile coordinates
        startX = static_cast<int>((viewLeft - offsetX) / properties.tileWidth) - 1;
        startY = static_cast<int>((viewTop - offsetY) / properties.tileHeight) - 1;
        endX = static_cast<int>((viewRight - offsetX) / properties.tileWidth) + 2;
        endY = static_cast<int>((viewBottom - offsetY) / properties.tileHeight) + 2;
        
        // Clamp to layer bounds
        startX = std::max(0, std::min(startX, layer->getWidth()));
        startY = std::max(0, std::min(startY, layer->getHeight()));
        endX = std::max(0, std::min(endX, layer->getWidth()));
        endY = std::max(0, std::min(endY, layer->getHeight()));
    }
    
    // Render colliders for solid tiles
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Tile* tile = layer->getTile(x, y);
            if (tile && tile->id != 0 && tile->isSolid()) {
                // Calculate destination rectangle
                float destX = x * properties.tileWidth + layerProps.offsetX;
                float destY = y * properties.tileHeight + layerProps.offsetY;
                float destWidth = properties.tileWidth;
                float destHeight = properties.tileHeight;
                
                // Apply parallax if camera is available
                if (m_camera) {
                    destX += m_camera->getPosition().x * (1.0f - layerProps.parallaxX);
                    destY += m_camera->getPosition().y * (1.0f - layerProps.parallaxY);
                }
                
                // Draw collider outline
                m_graphics->drawRectangleOutline(destX, destY, destWidth, destHeight, m_colliderColor);
            }
        }
    }
}

uint32_t TilemapRenderer::getCurrentAnimationFrame(uint32_t tileId, std::shared_ptr<Tileset> tileset) {
    // Find tileset and local tile ID
    std::shared_ptr<Tileset> animTileset;
    uint32_t localTileId;
    
    if (!m_tilemap->findTilesetAndLocalId(tileId, animTileset, localTileId)) {
        return tileId;
    }
    
    // Check if tile is animated
    const TileAnimation* animation = animTileset->getAnimation(localTileId);
    if (!animation || animation->frames.empty()) {
        return tileId;
    }
    
    // Get animation state
    auto it = m_animationStates.find(tileId);
    if (it == m_animationStates.end()) {
        return tileId;
    }
    
    // Get current frame
    const auto& state = it->second;
    if (state.frameIndex >= animation->frames.size()) {
        return tileId;
    }
    
    // Get frame tile ID
    uint32_t frameTileId = animation->frames[state.frameIndex].tileId;
    
    // Convert to global tile ID
    uint32_t firstGid = 1;
    for (size_t i = 0; i < m_tilemap->getTilesetCount(); ++i) {
        auto ts = m_tilemap->getTileset(i);
        if (ts == animTileset) {
            break;
        }
        firstGid += ts->getTileCount();
    }
    
    return firstGid + frameTileId;
}

} // namespace Tilemap
} // namespace RPGEngine