#pragma once

#include "TileLayer.h"
#include "Tileset.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace RPGEngine {
namespace Tilemap {

/**
 * Map orientation enumeration
 */
enum class MapOrientation {
    Orthogonal,     // Orthogonal (rectangular) tiles
    Isometric,      // Isometric tiles
    Staggered,      // Staggered isometric tiles
    Hexagonal       // Hexagonal tiles
};

/**
 * Map properties
 */
struct MapProperties {
    std::string name;                  // Map name
    MapOrientation orientation;        // Map orientation
    int width;                         // Map width in tiles
    int height;                        // Map height in tiles
    int tileWidth;                     // Tile width in pixels
    int tileHeight;                    // Tile height in pixels
    int hexSideLength;                 // Hex side length (for hexagonal maps)
    std::string backgroundColor;        // Background color (hex format)
    std::unordered_map<std::string, std::string> customProperties; // Custom properties
    
    MapProperties()
        : name("")
        , orientation(MapOrientation::Orthogonal)
        , width(0)
        , height(0)
        , tileWidth(0)
        , tileHeight(0)
        , hexSideLength(0)
        , backgroundColor("#000000")
    {}
};

/**
 * Tilemap class
 * Represents a 2D tilemap
 */
class Tilemap {
public:
    /**
     * Constructor
     * @param properties Map properties
     */
    Tilemap(const MapProperties& properties = MapProperties());
    
    /**
     * Destructor
     */
    ~Tilemap();
    
    /**
     * Get the map properties
     * @return Map properties
     */
    const MapProperties& getProperties() const { return m_properties; }
    
    /**
     * Set the map properties
     * @param properties Map properties
     */
    void setProperties(const MapProperties& properties) { m_properties = properties; }
    
    /**
     * Get the number of layers
     * @return Number of layers
     */
    size_t getLayerCount() const { return m_layers.size(); }
    
    /**
     * Get a layer by index
     * @param index Layer index
     * @return Layer, or nullptr if index is out of bounds
     */
    std::shared_ptr<TileLayer> getLayer(size_t index) const;
    
    /**
     * Get a layer by name
     * @param name Layer name
     * @return Layer, or nullptr if not found
     */
    std::shared_ptr<TileLayer> getLayerByName(const std::string& name) const;
    
    /**
     * Add a layer
     * @param layer Layer to add
     * @return Index of the added layer
     */
    size_t addLayer(std::shared_ptr<TileLayer> layer);
    
    /**
     * Insert a layer at the specified index
     * @param index Index to insert at
     * @param layer Layer to insert
     * @return true if the layer was inserted
     */
    bool insertLayer(size_t index, std::shared_ptr<TileLayer> layer);
    
    /**
     * Remove a layer by index
     * @param index Layer index
     * @return true if the layer was removed
     */
    bool removeLayer(size_t index);
    
    /**
     * Remove a layer by name
     * @param name Layer name
     * @return true if the layer was removed
     */
    bool removeLayerByName(const std::string& name);
    
    /**
     * Move a layer to a new index
     * @param fromIndex Current layer index
     * @param toIndex New layer index
     * @return true if the layer was moved
     */
    bool moveLayer(size_t fromIndex, size_t toIndex);
    
    /**
     * Get the number of tilesets
     * @return Number of tilesets
     */
    size_t getTilesetCount() const { return m_tilesets.size(); }
    
    /**
     * Get a tileset by index
     * @param index Tileset index
     * @return Tileset, or nullptr if index is out of bounds
     */
    std::shared_ptr<Tileset> getTileset(size_t index) const;
    
    /**
     * Get a tileset by name
     * @param name Tileset name
     * @return Tileset, or nullptr if not found
     */
    std::shared_ptr<Tileset> getTilesetByName(const std::string& name) const;
    
    /**
     * Add a tileset
     * @param tileset Tileset to add
     * @return Index of the added tileset
     */
    size_t addTileset(std::shared_ptr<Tileset> tileset);
    
    /**
     * Remove a tileset by index
     * @param index Tileset index
     * @return true if the tileset was removed
     */
    bool removeTileset(size_t index);
    
    /**
     * Remove a tileset by name
     * @param name Tileset name
     * @return true if the tileset was removed
     */
    bool removeTilesetByName(const std::string& name);
    
    /**
     * Find the tileset and local tile ID for a global tile ID
     * @param globalTileId Global tile ID
     * @param tileset Output tileset
     * @param localTileId Output local tile ID
     * @return true if the tile ID is valid
     */
    bool findTilesetAndLocalId(uint32_t globalTileId, std::shared_ptr<Tileset>& tileset, uint32_t& localTileId) const;
    
    /**
     * Clear the tilemap
     */
    void clear();
    
private:
    MapProperties m_properties;                            // Map properties
    std::vector<std::shared_ptr<TileLayer>> m_layers;      // Map layers
    std::vector<std::shared_ptr<Tileset>> m_tilesets;      // Map tilesets
    std::vector<uint32_t> m_firstGids;                     // First global tile ID for each tileset
};

} // namespace Tilemap
} // namespace RPGEngine