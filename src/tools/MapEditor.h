#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../tilemap/Tilemap.h"
#include "../tilemap/Tileset.h"
#include "../core/Types.h"

namespace Engine {
    namespace Tools {

        struct TileSelection {
            int tilesetId;
            int tileId;
            int layer;
        };

        struct EditorTile {
            int tileId;
            int tilesetId;
            bool collision;
            std::string properties;
        };

        class MapEditor {
        public:
            MapEditor();
            ~MapEditor();

            // Map operations
            bool createNewMap(int width, int height, int tileSize);
            bool loadMap(const std::string& filepath);
            bool saveMap(const std::string& filepath);
            void clearMap();

            // Tile operations
            void placeTile(int x, int y, int layer, const TileSelection& tile);
            void removeTile(int x, int y, int layer);
            EditorTile getTile(int x, int y, int layer) const;
            
            // Layer operations
            void addLayer(const std::string& name);
            void removeLayer(int layerIndex);
            void setLayerVisible(int layerIndex, bool visible);
            void setLayerOpacity(int layerIndex, float opacity);
            void moveLayer(int fromIndex, int toIndex);

            // Tileset operations
            bool loadTileset(const std::string& filepath);
            void removeTileset(int tilesetId);
            std::vector<int> getAvailableTilesets() const;

            // Selection and tools
            void setSelectedTile(const TileSelection& selection);
            TileSelection getSelectedTile() const;
            void setTool(const std::string& toolName);
            std::string getCurrentTool() const;

            // Map properties
            int getMapWidth() const { return m_mapWidth; }
            int getMapHeight() const { return m_mapHeight; }
            int getTileSize() const { return m_tileSize; }
            int getLayerCount() const { return m_layers.size(); }

            // Grid and display
            void setGridVisible(bool visible) { m_gridVisible = visible; }
            bool isGridVisible() const { return m_gridVisible; }
            void setSnapToGrid(bool snap) { m_snapToGrid = snap; }
            bool isSnapToGrid() const { return m_snapToGrid; }

            // Undo/Redo
            void undo();
            void redo();
            bool canUndo() const;
            bool canRedo() const;

            // Validation
            bool validateMap() const;
            std::vector<std::string> getValidationErrors() const;

        private:
            struct MapLayer {
                std::string name;
                std::vector<std::vector<EditorTile>> tiles;
                bool visible;
                float opacity;
            };

            struct EditorAction {
                enum Type { PlaceTile, RemoveTile, AddLayer, RemoveLayer };
                Type type;
                int x, y, layer;
                EditorTile oldTile, newTile;
                std::string data;
            };

            int m_mapWidth;
            int m_mapHeight;
            int m_tileSize;
            std::vector<MapLayer> m_layers;
            std::vector<std::string> m_tilesetPaths;
            
            TileSelection m_selectedTile;
            std::string m_currentTool;
            bool m_gridVisible;
            bool m_snapToGrid;
            
            std::vector<EditorAction> m_undoStack;
            std::vector<EditorAction> m_redoStack;
            size_t m_maxUndoSteps;

            void addUndoAction(const EditorAction& action);
            void clearRedoStack();
            bool isValidPosition(int x, int y) const;
            bool isValidLayer(int layer) const;
        };

    } // namespace Tools
} // namespace Engine