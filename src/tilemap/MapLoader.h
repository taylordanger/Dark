#pragma once

#include "Tilemap.h"
#include "../resources/ResourceManager.h"
#include "../utils/XMLParser.h"
#include <string>
#include <memory>

namespace RPGEngine {
namespace Tilemap {

/**
 * Map loader class
 * Loads maps from TMX (Tiled) files
 */
class MapLoader {
public:
    /**
     * Constructor
     * @param resourceManager Resource manager
     */
    MapLoader(std::shared_ptr<Resources::ResourceManager> resourceManager);
    
    /**
     * Destructor
     */
    ~MapLoader();
    
    /**
     * Load a map from a TMX file
     * @param filename TMX file path
     * @return Loaded map, or nullptr if loading failed
     */
    std::shared_ptr<Tilemap> loadMap(const std::string& filename);
    
    /**
     * Get the resource manager
     * @return Resource manager
     */
    std::shared_ptr<Resources::ResourceManager> getResourceManager() const { return m_resourceManager; }
    
private:
    /**
     * Parse a map node
     * @param mapNode Map node
     * @return Parsed map, or nullptr if parsing failed
     */
    std::shared_ptr<Tilemap> parseMap(std::shared_ptr<Utils::XMLNode> mapNode);
    
    /**
     * Parse a tileset node
     * @param tilesetNode Tileset node
     * @param firstGid First global tile ID
     * @param basePath Base path for relative paths
     * @return Parsed tileset, or nullptr if parsing failed
     */
    std::shared_ptr<Tileset> parseTileset(std::shared_ptr<Utils::XMLNode> tilesetNode, uint32_t firstGid, const std::string& basePath);
    
    /**
     * Parse a layer node
     * @param layerNode Layer node
     * @param map Map
     * @return Parsed layer, or nullptr if parsing failed
     */
    std::shared_ptr<TileLayer> parseLayer(std::shared_ptr<Utils::XMLNode> layerNode, std::shared_ptr<Tilemap> map);
    
    /**
     * Parse layer data
     * @param dataNode Data node
     * @param layer Layer
     * @return true if parsing was successful
     */
    bool parseLayerData(std::shared_ptr<Utils::XMLNode> dataNode, std::shared_ptr<TileLayer> layer);
    
    /**
     * Parse CSV layer data
     * @param data CSV data
     * @param layer Layer
     * @return true if parsing was successful
     */
    bool parseCSVData(const std::string& data, std::shared_ptr<TileLayer> layer);
    
    /**
     * Parse base64 layer data
     * @param data Base64 data
     * @param layer Layer
     * @param compression Compression type (empty, "zlib", or "gzip")
     * @return true if parsing was successful
     */
    bool parseBase64Data(const std::string& data, std::shared_ptr<TileLayer> layer, const std::string& compression);
    
    /**
     * Parse tile properties
     * @param tileNode Tile node
     * @param tileset Tileset
     * @param tileId Tile ID
     * @return true if parsing was successful
     */
    bool parseTileProperties(std::shared_ptr<Utils::XMLNode> tileNode, std::shared_ptr<Tileset> tileset, uint32_t tileId);
    
    /**
     * Parse tile animation
     * @param animationNode Animation node
     * @param tileset Tileset
     * @param tileId Tile ID
     * @return true if parsing was successful
     */
    bool parseTileAnimation(std::shared_ptr<Utils::XMLNode> animationNode, std::shared_ptr<Tileset> tileset, uint32_t tileId);
    
    // Resource manager
    std::shared_ptr<Resources::ResourceManager> m_resourceManager;
    
    // XML parser
    Utils::XMLParser m_xmlParser;
};

} // namespace Tilemap
} // namespace RPGEngine