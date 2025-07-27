#pragma once

#include "../resources/TextureResource.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace RPGEngine {
namespace Tilemap {

/**
 * Tile animation frame
 */
struct TileAnimationFrame {
    uint32_t tileId;   // Tile ID
    uint32_t duration; // Duration in milliseconds
    
    TileAnimationFrame(uint32_t tileId = 0, uint32_t duration = 0)
        : tileId(tileId), duration(duration) {}
};

/**
 * Tile animation
 */
struct TileAnimation {
    std::vector<TileAnimationFrame> frames; // Animation frames
    
    TileAnimation() {}
    TileAnimation(const std::vector<TileAnimationFrame>& frames) : frames(frames) {}
};

/**
 * Tileset class
 * Represents a collection of tiles used in a tilemap
 */
class Tileset {
public:
    /**
     * Constructor
     * @param name Tileset name
     * @param tileWidth Tile width in pixels
     * @param tileHeight Tile height in pixels
     * @param spacing Spacing between tiles in pixels
     * @param margin Margin around tiles in pixels
     */
    Tileset(const std::string& name, int tileWidth, int tileHeight, int spacing = 0, int margin = 0);
    
    /**
     * Destructor
     */
    ~Tileset();
    
    /**
     * Get the tileset name
     * @return Tileset name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * Get the tile width
     * @return Tile width in pixels
     */
    int getTileWidth() const { return m_tileWidth; }
    
    /**
     * Get the tile height
     * @return Tile height in pixels
     */
    int getTileHeight() const { return m_tileHeight; }
    
    /**
     * Get the spacing between tiles
     * @return Spacing in pixels
     */
    int getSpacing() const { return m_spacing; }
    
    /**
     * Get the margin around tiles
     * @return Margin in pixels
     */
    int getMargin() const { return m_margin; }
    
    /**
     * Get the number of tiles in the tileset
     * @return Number of tiles
     */
    int getTileCount() const { return m_tileCount; }
    
    /**
     * Get the number of columns in the tileset
     * @return Number of columns
     */
    int getColumns() const { return m_columns; }
    
    /**
     * Get the texture resource
     * @return Texture resource
     */
    std::shared_ptr<Resources::TextureResource> getTexture() const { return m_texture; }
    
    /**
     * Set the texture resource
     * @param texture Texture resource
     */
    void setTexture(std::shared_ptr<Resources::TextureResource> texture);
    
    /**
     * Get the tile animation for a tile ID
     * @param tileId Tile ID
     * @return Tile animation, or nullptr if not found
     */
    const TileAnimation* getAnimation(uint32_t tileId) const;
    
    /**
     * Set the tile animation for a tile ID
     * @param tileId Tile ID
     * @param animation Tile animation
     */
    void setAnimation(uint32_t tileId, const TileAnimation& animation);
    
    /**
     * Remove the tile animation for a tile ID
     * @param tileId Tile ID
     * @return true if the animation was removed
     */
    bool removeAnimation(uint32_t tileId);
    
    /**
     * Get the tile flags for a tile ID
     * @param tileId Tile ID
     * @return Tile flags
     */
    uint32_t getTileFlags(uint32_t tileId) const;
    
    /**
     * Set the tile flags for a tile ID
     * @param tileId Tile ID
     * @param flags Tile flags
     */
    void setTileFlags(uint32_t tileId, uint32_t flags);
    
    /**
     * Get the tile source rectangle
     * @param tileId Tile ID
     * @param x Output X position
     * @param y Output Y position
     * @param width Output width
     * @param height Output height
     * @return true if the tile ID is valid
     */
    bool getTileSourceRect(uint32_t tileId, int& x, int& y, int& width, int& height) const;
    
private:
    std::string m_name;                                                // Tileset name
    int m_tileWidth;                                                   // Tile width in pixels
    int m_tileHeight;                                                  // Tile height in pixels
    int m_spacing;                                                     // Spacing between tiles in pixels
    int m_margin;                                                      // Margin around tiles in pixels
    int m_tileCount;                                                   // Number of tiles in the tileset
    int m_columns;                                                     // Number of columns in the tileset
    std::shared_ptr<Resources::TextureResource> m_texture;             // Texture resource
    std::unordered_map<uint32_t, TileAnimation> m_animations;          // Tile animations
    std::unordered_map<uint32_t, uint32_t> m_tileFlags;                // Tile flags
};

} // namespace Tilemap
} // namespace RPGEngine