#pragma once

#include "Tile.h"
#include <vector>
#include <string>
#include <memory>

namespace RPGEngine {
namespace Tilemap {

/**
 * Layer type enumeration
 */
enum class LayerType {
    Tile,       // Tile layer
    Object,     // Object layer
    Image,      // Image layer
    Group       // Group layer
};

/**
 * Layer properties
 */
struct LayerProperties {
    std::string name;      // Layer name
    bool visible;          // Layer visibility
    float opacity;         // Layer opacity (0.0 to 1.0)
    int offsetX;           // X offset
    int offsetY;           // Y offset
    float parallaxX;       // X parallax factor
    float parallaxY;       // Y parallax factor
    
    LayerProperties()
        : name("")
        , visible(true)
        , opacity(1.0f)
        , offsetX(0)
        , offsetY(0)
        , parallaxX(1.0f)
        , parallaxY(1.0f)
    {}
};

/**
 * Tile layer class
 * Represents a layer of tiles in a tilemap
 */
class TileLayer {
public:
    /**
     * Constructor
     * @param width Layer width in tiles
     * @param height Layer height in tiles
     * @param properties Layer properties
     */
    TileLayer(int width, int height, const LayerProperties& properties = LayerProperties());
    
    /**
     * Destructor
     */
    ~TileLayer();
    
    /**
     * Get the layer width
     * @return Layer width in tiles
     */
    int getWidth() const { return m_width; }
    
    /**
     * Get the layer height
     * @return Layer height in tiles
     */
    int getHeight() const { return m_height; }
    
    /**
     * Get the layer properties
     * @return Layer properties
     */
    const LayerProperties& getProperties() const { return m_properties; }
    
    /**
     * Set the layer properties
     * @param properties Layer properties
     */
    void setProperties(const LayerProperties& properties) { m_properties = properties; }
    
    /**
     * Get a tile at the specified position
     * @param x X position
     * @param y Y position
     * @return Tile at the specified position, or nullptr if out of bounds
     */
    const Tile* getTile(int x, int y) const;
    
    /**
     * Set a tile at the specified position
     * @param x X position
     * @param y Y position
     * @param tile Tile to set
     * @return true if the tile was set
     */
    bool setTile(int x, int y, const Tile& tile);
    
    /**
     * Clear a tile at the specified position
     * @param x X position
     * @param y Y position
     * @return true if the tile was cleared
     */
    bool clearTile(int x, int y);
    
    /**
     * Clear all tiles
     */
    void clearAllTiles();
    
    /**
     * Resize the layer
     * @param width New width
     * @param height New height
     * @param preserveData Whether to preserve existing tile data
     */
    void resize(int width, int height, bool preserveData = true);
    
    /**
     * Check if a position is within the layer bounds
     * @param x X position
     * @param y Y position
     * @return true if the position is within bounds
     */
    bool isInBounds(int x, int y) const;
    
    /**
     * Get the layer type
     * @return Layer type
     */
    LayerType getType() const { return LayerType::Tile; }
    
private:
    int m_width;                   // Layer width in tiles
    int m_height;                  // Layer height in tiles
    LayerProperties m_properties;  // Layer properties
    std::vector<Tile> m_tiles;     // Tile data
};

} // namespace Tilemap
} // namespace RPGEngine