#include "MapLoader.h"
#include "../utils/Base64.h"
#include "../utils/Zlib.h"
#include "../resources/TextureResource.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace RPGEngine {
namespace Tilemap {

MapLoader::MapLoader(std::shared_ptr<Resources::ResourceManager> resourceManager)
    : m_resourceManager(resourceManager)
{
}

MapLoader::~MapLoader() {
    // Nothing to do here
}

std::shared_ptr<Tilemap> MapLoader::loadMap(const std::string& filename) {
    // Parse TMX file
    auto rootNode = m_xmlParser.parseFile(filename);
    if (!rootNode || rootNode->getName() != "map") {
        std::cerr << "Failed to parse TMX file: " << filename << std::endl;
        return nullptr;
    }
    
    // Parse map
    auto map = parseMap(rootNode);
    if (!map) {
        std::cerr << "Failed to parse map: " << filename << std::endl;
        return nullptr;
    }
    
    // Get base path for relative paths
    std::string basePath = filename.substr(0, filename.find_last_of("/\\") + 1);
    
    // Parse tilesets
    auto tilesetNodes = rootNode->getChildrenByName("tileset");
    for (const auto& tilesetNode : tilesetNodes) {
        uint32_t firstGid = tilesetNode->getAttributeInt("firstgid", 1);
        auto tileset = parseTileset(tilesetNode, firstGid, basePath);
        if (tileset) {
            map->addTileset(tileset);
        }
    }
    
    // Parse layers
    auto layerNodes = rootNode->getChildrenByName("layer");
    for (const auto& layerNode : layerNodes) {
        auto layer = parseLayer(layerNode, map);
        if (layer) {
            map->addLayer(layer);
        }
    }
    
    return map;
}

std::shared_ptr<Tilemap> MapLoader::parseMap(std::shared_ptr<Utils::XMLNode> mapNode) {
    // Parse map properties
    MapProperties properties;
    properties.name = mapNode->getAttribute("name", "Unnamed Map");
    
    // Parse orientation
    std::string orientation = mapNode->getAttribute("orientation", "orthogonal");
    if (orientation == "orthogonal") {
        properties.orientation = MapOrientation::Orthogonal;
    } else if (orientation == "isometric") {
        properties.orientation = MapOrientation::Isometric;
    } else if (orientation == "staggered") {
        properties.orientation = MapOrientation::Staggered;
    } else if (orientation == "hexagonal") {
        properties.orientation = MapOrientation::Hexagonal;
    } else {
        std::cerr << "Unsupported map orientation: " << orientation << std::endl;
        return nullptr;
    }
    
    // Parse dimensions
    properties.width = mapNode->getAttributeInt("width", 0);
    properties.height = mapNode->getAttributeInt("height", 0);
    properties.tileWidth = mapNode->getAttributeInt("tilewidth", 0);
    properties.tileHeight = mapNode->getAttributeInt("tileheight", 0);
    
    // Parse hex side length (for hexagonal maps)
    if (properties.orientation == MapOrientation::Hexagonal) {
        properties.hexSideLength = mapNode->getAttributeInt("hexsidelength", 0);
    }
    
    // Parse background color
    properties.backgroundColor = mapNode->getAttribute("backgroundcolor", "#000000");
    
    // Parse custom properties
    auto propertiesNode = mapNode->getChild("properties");
    if (propertiesNode) {
        auto propertyNodes = propertiesNode->getChildrenByName("property");
        for (const auto& propertyNode : propertyNodes) {
            std::string name = propertyNode->getAttribute("name");
            std::string value = propertyNode->getAttribute("value");
            if (!name.empty()) {
                properties.customProperties[name] = value;
            }
        }
    }
    
    // Create map
    return std::make_shared<Tilemap>(properties);
}

std::shared_ptr<Tileset> MapLoader::parseTileset(std::shared_ptr<Utils::XMLNode> tilesetNode, uint32_t firstGid, const std::string& basePath) {
    // Check if this is an external tileset
    std::string source = tilesetNode->getAttribute("source");
    if (!source.empty()) {
        // Load external tileset
        std::string tilesetPath = basePath + source;
        auto externalTilesetNode = m_xmlParser.parseFile(tilesetPath);
        if (!externalTilesetNode || externalTilesetNode->getName() != "tileset") {
            std::cerr << "Failed to parse external tileset: " << tilesetPath << std::endl;
            return nullptr;
        }
        
        // Parse external tileset
        return parseTileset(externalTilesetNode, firstGid, basePath);
    }
    
    // Parse tileset properties
    std::string name = tilesetNode->getAttribute("name", "Unnamed Tileset");
    int tileWidth = tilesetNode->getAttributeInt("tilewidth", 0);
    int tileHeight = tilesetNode->getAttributeInt("tileheight", 0);
    int spacing = tilesetNode->getAttributeInt("spacing", 0);
    int margin = tilesetNode->getAttributeInt("margin", 0);
    
    // Create tileset
    auto tileset = std::make_shared<Tileset>(name, tileWidth, tileHeight, spacing, margin);
    
    // Parse image
    auto imageNode = tilesetNode->getChild("image");
    if (imageNode) {
        std::string source = imageNode->getAttribute("source");
        if (!source.empty()) {
            // Load texture
            std::string texturePath = basePath + source;
            std::string textureId = "tileset_" + name;
            
            // Check if texture already exists
            auto texture = m_resourceManager->getResourceOfType<Resources::TextureResource>(textureId);
            if (!texture) {
                // Create and load texture
                texture = std::make_shared<Resources::TextureResource>(textureId, texturePath);
                m_resourceManager->addResource(texture);
                m_resourceManager->loadResource(textureId);
            }
            
            // Set tileset texture
            tileset->setTexture(texture);
        }
    }
    
    // Parse tile properties
    auto tileNodes = tilesetNode->getChildrenByName("tile");
    for (const auto& tileNode : tileNodes) {
        int tileId = tileNode->getAttributeInt("id", 0);
        parseTileProperties(tileNode, tileset, tileId);
    }
    
    return tileset;
}

std::shared_ptr<TileLayer> MapLoader::parseLayer(std::shared_ptr<Utils::XMLNode> layerNode, std::shared_ptr<Tilemap> map) {
    // Parse layer properties
    LayerProperties properties;
    properties.name = layerNode->getAttribute("name", "Unnamed Layer");
    properties.visible = layerNode->getAttributeBool("visible", true);
    properties.opacity = layerNode->getAttributeFloat("opacity", 1.0f);
    properties.offsetX = layerNode->getAttributeInt("offsetx", 0);
    properties.offsetY = layerNode->getAttributeInt("offsety", 0);
    
    // Parse parallax factors
    properties.parallaxX = layerNode->getAttributeFloat("parallaxx", 1.0f);
    properties.parallaxY = layerNode->getAttributeFloat("parallaxy", 1.0f);
    
    // Parse dimensions
    int width = layerNode->getAttributeInt("width", map->getProperties().width);
    int height = layerNode->getAttributeInt("height", map->getProperties().height);
    
    // Create layer
    auto layer = std::make_shared<TileLayer>(width, height, properties);
    
    // Parse data
    auto dataNode = layerNode->getChild("data");
    if (dataNode) {
        if (!parseLayerData(dataNode, layer)) {
            std::cerr << "Failed to parse layer data: " << properties.name << std::endl;
            return nullptr;
        }
    }
    
    return layer;
}

bool MapLoader::parseLayerData(std::shared_ptr<Utils::XMLNode> dataNode, std::shared_ptr<TileLayer> layer) {
    // Get encoding and compression
    std::string encoding = dataNode->getAttribute("encoding", "");
    std::string compression = dataNode->getAttribute("compression", "");
    
    // Get data
    std::string data = dataNode->getValue();
    
    // Parse data based on encoding
    if (encoding == "csv") {
        return parseCSVData(data, layer);
    } else if (encoding == "base64") {
        return parseBase64Data(data, layer, compression);
    } else if (encoding.empty()) {
        // Parse XML data
        auto tileNodes = dataNode->getChildrenByName("tile");
        int index = 0;
        
        for (const auto& tileNode : tileNodes) {
            uint32_t gid = tileNode->getAttributeInt("gid", 0);
            
            int x = index % layer->getWidth();
            int y = index / layer->getWidth();
            
            if (x < layer->getWidth() && y < layer->getHeight()) {
                layer->setTile(x, y, Tile(gid));
            }
            
            index++;
        }
        
        return true;
    }
    
    std::cerr << "Unsupported layer data encoding: " << encoding << std::endl;
    return false;
}

bool MapLoader::parseCSVData(const std::string& data, std::shared_ptr<TileLayer> layer) {
    std::stringstream ss(data);
    std::string line;
    int y = 0;
    
    while (std::getline(ss, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Parse CSV line
        std::stringstream lineStream(line);
        std::string cell;
        int x = 0;
        
        while (std::getline(lineStream, cell, ',')) {
            // Trim whitespace
            cell.erase(std::remove_if(cell.begin(), cell.end(), [](unsigned char c) { return std::isspace(c); }), cell.end());
            
            // Skip empty cells
            if (cell.empty()) {
                continue;
            }
            
            // Parse tile ID
            try {
                uint32_t gid = std::stoul(cell);
                
                if (x < layer->getWidth() && y < layer->getHeight()) {
                    layer->setTile(x, y, Tile(gid));
                }
                
                x++;
            } catch (...) {
                std::cerr << "Failed to parse tile ID: " << cell << std::endl;
            }
        }
        
        y++;
    }
    
    return true;
}

bool MapLoader::parseBase64Data(const std::string& data, std::shared_ptr<TileLayer> layer, const std::string& compression) {
    // Decode base64 data
    std::string trimmedData = data;
    trimmedData.erase(std::remove_if(trimmedData.begin(), trimmedData.end(), [](unsigned char c) { return std::isspace(c); }), trimmedData.end());
    
    auto decodedData = Utils::Base64::decode(trimmedData);
    
    // Decompress data if needed
    if (compression == "zlib" || compression == "gzip") {
        size_t expectedSize = layer->getWidth() * layer->getHeight() * 4; // 4 bytes per tile
        decodedData = Utils::Zlib::decompress(decodedData, expectedSize);
    } else if (!compression.empty()) {
        std::cerr << "Unsupported compression: " << compression << std::endl;
        return false;
    }
    
    // Parse tile data
    size_t expectedSize = layer->getWidth() * layer->getHeight() * 4; // 4 bytes per tile
    if (decodedData.size() < expectedSize) {
        std::cerr << "Insufficient tile data: expected " << expectedSize << " bytes, got " << decodedData.size() << " bytes" << std::endl;
        return false;
    }
    
    for (int y = 0; y < layer->getHeight(); ++y) {
        for (int x = 0; x < layer->getWidth(); ++x) {
            size_t index = (y * layer->getWidth() + x) * 4;
            
            // Parse tile ID (little-endian)
            uint32_t gid = decodedData[index] |
                          (decodedData[index + 1] << 8) |
                          (decodedData[index + 2] << 16) |
                          (decodedData[index + 3] << 24);
            
            layer->setTile(x, y, Tile(gid));
        }
    }
    
    return true;
}

bool MapLoader::parseTileProperties(std::shared_ptr<Utils::XMLNode> tileNode, std::shared_ptr<Tileset> tileset, uint32_t tileId) {
    // Parse tile flags
    uint32_t flags = TileFlags::None;
    
    // Parse properties
    auto propertiesNode = tileNode->getChild("properties");
    if (propertiesNode) {
        auto propertyNodes = propertiesNode->getChildrenByName("property");
        for (const auto& propertyNode : propertyNodes) {
            std::string name = propertyNode->getAttribute("name");
            std::string value = propertyNode->getAttribute("value");
            
            // Check for special properties
            if (name == "solid" && (value == "true" || value == "1")) {
                flags |= TileFlags::Solid;
            } else if (name == "trigger" && (value == "true" || value == "1")) {
                flags |= TileFlags::Trigger;
            } else if (name == "water" && (value == "true" || value == "1")) {
                flags |= TileFlags::Water;
            } else if (name == "lava" && (value == "true" || value == "1")) {
                flags |= TileFlags::Lava;
            } else if (name == "damage" && (value == "true" || value == "1")) {
                flags |= TileFlags::Damage;
            } else if (name == "heal" && (value == "true" || value == "1")) {
                flags |= TileFlags::Heal;
            } else if (name == "slippery" && (value == "true" || value == "1")) {
                flags |= TileFlags::Slippery;
            } else if (name == "slow" && (value == "true" || value == "1")) {
                flags |= TileFlags::Slow;
            } else if (name == "fast" && (value == "true" || value == "1")) {
                flags |= TileFlags::Fast;
            }
        }
    }
    
    // Set tile flags
    tileset->setTileFlags(tileId, flags);
    
    // Parse animation
    auto animationNode = tileNode->getChild("animation");
    if (animationNode) {
        parseTileAnimation(animationNode, tileset, tileId);
    }
    
    return true;
}

bool MapLoader::parseTileAnimation(std::shared_ptr<Utils::XMLNode> animationNode, std::shared_ptr<Tileset> tileset, uint32_t tileId) {
    // Parse animation frames
    auto frameNodes = animationNode->getChildrenByName("frame");
    if (frameNodes.empty()) {
        return false;
    }
    
    TileAnimation animation;
    
    for (const auto& frameNode : frameNodes) {
        uint32_t frameTileId = frameNode->getAttributeInt("tileid", 0);
        uint32_t duration = frameNode->getAttributeInt("duration", 100);
        
        animation.frames.emplace_back(frameTileId, duration);
    }
    
    // Set tile animation
    tileset->setAnimation(tileId, animation);
    
    return true;
}

} // namespace Tilemap
} // namespace RPGEngine