#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "../entities/EntityManager.h"
#include "../components/ComponentManager.h"
#include "../core/Types.h"

namespace Engine {
    namespace Debug {

        struct EntityInfo {
            RPGEngine::EntityId id;
            bool active;
            std::vector<std::string> componentTypes;
            std::string name;
        };

        struct ComponentInfo {
            std::string type;
            std::string data;
            bool enabled;
        };

        class EntityInspector {
        public:
            EntityInspector(std::shared_ptr<RPGEngine::EntityManager> entityManager,
                          std::shared_ptr<RPGEngine::ComponentManager> componentManager);
            ~EntityInspector();

            // Entity inspection
            std::vector<EntityInfo> getAllEntities() const;
            EntityInfo getEntityInfo(RPGEngine::EntityId entityId) const;
            std::vector<ComponentInfo> getEntityComponents(RPGEngine::EntityId entityId) const;

            // Entity filtering and searching
            std::vector<EntityInfo> findEntitiesByName(const std::string& name) const;
            std::vector<EntityInfo> findEntitiesWithComponent(const std::string& componentType) const;
            std::vector<EntityInfo> getActiveEntities() const;
            std::vector<EntityInfo> getInactiveEntities() const;

            // Entity manipulation (for debugging)
            void setEntityActive(RPGEngine::EntityId entityId, bool active);
            void destroyEntity(RPGEngine::EntityId entityId);
            RPGEngine::EntityId createDebugEntity(const std::string& name);

            // Component inspection
            std::string getComponentData(RPGEngine::EntityId entityId, const std::string& componentType) const;
            void setComponentEnabled(RPGEngine::EntityId entityId, const std::string& componentType, bool enabled);

            // Statistics
            size_t getTotalEntityCount() const;
            size_t getActiveEntityCount() const;
            std::vector<std::pair<std::string, size_t>> getComponentTypeStats() const;

            // Debug callbacks
            using EntitySelectedCallback = std::function<void(RPGEngine::EntityId)>;
            void setEntitySelectedCallback(EntitySelectedCallback callback) { m_entitySelectedCallback = callback; }

            // Update inspector (call each frame)
            void update();

            // UI rendering (if integrated with UI system)
            void renderInspectorUI();

        private:
            std::shared_ptr<RPGEngine::EntityManager> m_entityManager;
            std::shared_ptr<RPGEngine::ComponentManager> m_componentManager;
            EntitySelectedCallback m_entitySelectedCallback;
            RPGEngine::EntityId m_selectedEntity;

            std::string componentToString(RPGEngine::EntityId entityId, const std::string& componentType) const;
            std::vector<std::string> getEntityComponentTypes(RPGEngine::EntityId entityId) const;
        };

    } // namespace Debug
} // namespace Engine