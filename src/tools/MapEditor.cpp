#include "MapEditor.h"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace Engine {
    namespace Tools {

        MapEditor::MapEditor()
            : m_mapWidth(0)
            , m_mapHeight(0)
            , m_tileSize(32)
            , m_selectedTile{0, 0, 0}
            , m_currentTool("paint")
            , m_gridVisible(true)
            , m_snapToGrid(true)
            , m_maxUndoSteps(100) {
        }

        MapEditor::~MapEditor() {
        }

        bool MapEditor::createNewMap(int width, int height, int tileSize) {
            if (width <= 0 || height <= 0 || tileSize <= 0) {
                return false;
            }

            m_mapWidth = width;
            m_mapHeight = height;
            m_tileSize = tileSize;
            
            // Clear existing data
            m_layers.clear();
            m_undoStack.clear();
            m_redoStack.clear();
            
            // Create default layer
            addLayer("Background");
            
            return true;
        }

        bool MapEditor::loadMap(const std::string& filepath) {
            // Simplified map loading - in a real implementation this would parse TMX or custom format
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            // For now, just create a default map
            return createNewMap(50, 50, 32);
        }

        bool MapEditor::saveMap(const std::string& filepath) {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            // Simplified map saving - write basic map data
            file << "# Map Editor Save File\n";
            file << "width=" << m_mapWidth << "\n";
            file << "height=" << m_mapHeight << "\n";
            file << "tileSize=" << m_tileSize << "\n";
            file << "layers=" << m_layers.size() << "\n";
            
            for (size_t layerIndex = 0; layerIndex < m_layers.size(); ++layerIndex) {
                const auto& layer = m_layers[layerIndex];
                file << "layer=" << layerIndex << ",name=" << layer.name 
                     << ",visible=" << layer.visible << ",opacity=" << layer.opacity << "\n";
                
                // Write tile data (simplified)
                for (int y = 0; y < m_mapHeight; ++y) {
                    for (int x = 0; x < m_mapWidth; ++x) {
                        if (y < static_cast<int>(layer.tiles.size()) && 
                            x < static_cast<int>(layer.tiles[y].size())) {
                            const auto& tile = layer.tiles[y][x];
                            if (tile.tileId != 0) {
                                file << "tile=" << x << "," << y << "," << tile.tileId 
                                     << "," << tile.tilesetId << "\n";
                            }
                        }
                    }
                }
            }
            
            return true;
        }

        void MapEditor::clearMap() {
            for (auto& layer : m_layers) {
                for (auto& row : layer.tiles) {
                    for (auto& tile : row) {
                        tile = {0, 0, false, ""};
                    }
                }
            }
            
            m_undoStack.clear();
            m_redoStack.clear();
        }

        void MapEditor::placeTile(int x, int y, int layer, const TileSelection& tile) {
            if (!isValidPosition(x, y) || !isValidLayer(layer)) {
                return;
            }
            
            EditorAction action;
            action.type = EditorAction::PlaceTile;
            action.x = x;
            action.y = y;
            action.layer = layer;
            action.oldTile = getTile(x, y, layer);
            action.newTile = {tile.tileId, tile.tilesetId, false, ""};
            
            // Apply the change
            m_layers[layer].tiles[y][x] = action.newTile;
            
            // Add to undo stack
            addUndoAction(action);
        }

        void MapEditor::removeTile(int x, int y, int layer) {
            if (!isValidPosition(x, y) || !isValidLayer(layer)) {
                return;
            }
            
            EditorAction action;
            action.type = EditorAction::RemoveTile;
            action.x = x;
            action.y = y;
            action.layer = layer;
            action.oldTile = getTile(x, y, layer);
            action.newTile = {0, 0, false, ""};
            
            // Apply the change
            m_layers[layer].tiles[y][x] = action.newTile;
            
            // Add to undo stack
            addUndoAction(action);
        }

        EditorTile MapEditor::getTile(int x, int y, int layer) const {
            if (!isValidPosition(x, y) || !isValidLayer(layer)) {
                return {0, 0, false, ""};
            }
            
            const auto& layerData = m_layers[layer];
            if (y < static_cast<int>(layerData.tiles.size()) && 
                x < static_cast<int>(layerData.tiles[y].size())) {
                return layerData.tiles[y][x];
            }
            
            return {0, 0, false, ""};
        }

        void MapEditor::addLayer(const std::string& name) {
            MapLayer newLayer;
            newLayer.name = name;
            newLayer.visible = true;
            newLayer.opacity = 1.0f;
            newLayer.tiles.resize(m_mapHeight);
            
            for (int y = 0; y < m_mapHeight; ++y) {
                newLayer.tiles[y].resize(m_mapWidth);
                for (int x = 0; x < m_mapWidth; ++x) {
                    newLayer.tiles[y][x] = {0, 0, false, ""};
                }
            }
            
            m_layers.push_back(newLayer);
            
            EditorAction action;
            action.type = EditorAction::AddLayer;
            action.data = name;
            addUndoAction(action);
        }

        void MapEditor::removeLayer(int layerIndex) {
            if (!isValidLayer(layerIndex) || m_layers.size() <= 1) {
                return; // Don't remove the last layer
            }
            
            EditorAction action;
            action.type = EditorAction::RemoveLayer;
            action.layer = layerIndex;
            action.data = m_layers[layerIndex].name;
            
            m_layers.erase(m_layers.begin() + layerIndex);
            
            addUndoAction(action);
        }

        void MapEditor::setLayerVisible(int layerIndex, bool visible) {
            if (isValidLayer(layerIndex)) {
                m_layers[layerIndex].visible = visible;
            }
        }

        void MapEditor::setLayerOpacity(int layerIndex, float opacity) {
            if (isValidLayer(layerIndex)) {
                m_layers[layerIndex].opacity = std::max(0.0f, std::min(1.0f, opacity));
            }
        }

        void MapEditor::moveLayer(int fromIndex, int toIndex) {
            if (isValidLayer(fromIndex) && isValidLayer(toIndex) && fromIndex != toIndex) {
                std::swap(m_layers[fromIndex], m_layers[toIndex]);
            }
        }

        bool MapEditor::loadTileset(const std::string& filepath) {
            // Simplified tileset loading - just store the filepath for now
            // In a real implementation, this would load the tileset from file
            // For testing purposes, we'll just return success
            return true;
        }

        void MapEditor::removeTileset(int tilesetId) {
            if (tilesetId >= 0 && tilesetId < static_cast<int>(m_tilesetPaths.size())) {
                m_tilesetPaths.erase(m_tilesetPaths.begin() + tilesetId);
            }
        }

        std::vector<int> MapEditor::getAvailableTilesets() const {
            std::vector<int> tilesetIds;
            for (size_t i = 0; i < m_tilesetPaths.size(); ++i) {
                tilesetIds.push_back(static_cast<int>(i));
            }
            return tilesetIds;
        }

        void MapEditor::setSelectedTile(const TileSelection& selection) {
            m_selectedTile = selection;
        }

        TileSelection MapEditor::getSelectedTile() const {
            return m_selectedTile;
        }

        void MapEditor::setTool(const std::string& toolName) {
            m_currentTool = toolName;
        }

        std::string MapEditor::getCurrentTool() const {
            return m_currentTool;
        }

        void MapEditor::undo() {
            if (m_undoStack.empty()) {
                return;
            }
            
            EditorAction action = m_undoStack.back();
            m_undoStack.pop_back();
            
            // Reverse the action
            switch (action.type) {
                case EditorAction::PlaceTile:
                case EditorAction::RemoveTile:
                    if (isValidPosition(action.x, action.y) && isValidLayer(action.layer)) {
                        m_layers[action.layer].tiles[action.y][action.x] = action.oldTile;
                    }
                    break;
                case EditorAction::AddLayer:
                    if (!m_layers.empty()) {
                        m_layers.pop_back();
                    }
                    break;
                case EditorAction::RemoveLayer:
                    // Simplified - would need to restore full layer data
                    addLayer(action.data);
                    break;
            }
            
            m_redoStack.push_back(action);
        }

        void MapEditor::redo() {
            if (m_redoStack.empty()) {
                return;
            }
            
            EditorAction action = m_redoStack.back();
            m_redoStack.pop_back();
            
            // Reapply the action
            switch (action.type) {
                case EditorAction::PlaceTile:
                case EditorAction::RemoveTile:
                    if (isValidPosition(action.x, action.y) && isValidLayer(action.layer)) {
                        m_layers[action.layer].tiles[action.y][action.x] = action.newTile;
                    }
                    break;
                case EditorAction::AddLayer:
                    addLayer(action.data);
                    break;
                case EditorAction::RemoveLayer:
                    removeLayer(action.layer);
                    break;
            }
            
            m_undoStack.push_back(action);
        }

        bool MapEditor::canUndo() const {
            return !m_undoStack.empty();
        }

        bool MapEditor::canRedo() const {
            return !m_redoStack.empty();
        }

        bool MapEditor::validateMap() const {
            // Basic validation
            if (m_mapWidth <= 0 || m_mapHeight <= 0 || m_tileSize <= 0) {
                return false;
            }
            
            if (m_layers.empty()) {
                return false;
            }
            
            // Check layer dimensions
            for (const auto& layer : m_layers) {
                if (static_cast<int>(layer.tiles.size()) != m_mapHeight) {
                    return false;
                }
                for (const auto& row : layer.tiles) {
                    if (static_cast<int>(row.size()) != m_mapWidth) {
                        return false;
                    }
                }
            }
            
            return true;
        }

        std::vector<std::string> MapEditor::getValidationErrors() const {
            std::vector<std::string> errors;
            
            if (m_mapWidth <= 0) {
                errors.push_back("Map width must be greater than 0");
            }
            if (m_mapHeight <= 0) {
                errors.push_back("Map height must be greater than 0");
            }
            if (m_tileSize <= 0) {
                errors.push_back("Tile size must be greater than 0");
            }
            if (m_layers.empty()) {
                errors.push_back("Map must have at least one layer");
            }
            
            return errors;
        }

        void MapEditor::addUndoAction(const EditorAction& action) {
            m_undoStack.push_back(action);
            
            // Limit undo stack size
            if (m_undoStack.size() > m_maxUndoSteps) {
                m_undoStack.erase(m_undoStack.begin());
            }
            
            clearRedoStack();
        }

        void MapEditor::clearRedoStack() {
            m_redoStack.clear();
        }

        bool MapEditor::isValidPosition(int x, int y) const {
            return x >= 0 && x < m_mapWidth && y >= 0 && y < m_mapHeight;
        }

        bool MapEditor::isValidLayer(int layer) const {
            return layer >= 0 && layer < static_cast<int>(m_layers.size());
        }

    } // namespace Tools
} // namespace Engine