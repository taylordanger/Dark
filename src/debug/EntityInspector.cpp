#include "EntityInspector.h"
#include "../components/TransformComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/PhysicsComponent.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
#include <sstream>
#include <algorithm>
#include <map>

namespace Engine {
    namespace Debug {

        EntityInspector::EntityInspector(std::shared_ptr<RPGEngine::EntityManager> entityManager,
                                       std::shared_ptr<RPGEngine::ComponentManager> componentManager)
            : m_entityManager(entityManager)
            , m_componentManager(componentManager)
            , m_selectedEntity(0) {
        }

        EntityInspector::~EntityInspector() {
        }

        std::vector<EntityInfo> EntityInspector::getAllEntities() const {
            std::vector<EntityInfo> entities;
            
            if (!m_entityManager) return entities;

            // Get all entities from entity manager
            auto allEntities = m_entityManager->getAllEntities();
            
            for (const auto& entity : allEntities) {
                EntityInfo info;
                info.id = entity.getID();
                info.active = entity.isActive();
                info.componentTypes = getEntityComponentTypes(entity.getID());
                info.name = entity.getName().empty() ? "Entity_" + std::to_string(entity.getID()) : entity.getName();
                
                entities.push_back(info);
            }
            
            return entities;
        }

        EntityInfo EntityInspector::getEntityInfo(RPGEngine::EntityId entityId) const {
            EntityInfo info;
            info.id = entityId;
            
            if (m_entityManager) {
                auto entity = m_entityManager->getEntity(entityId);
                info.active = entity.isActive();
                info.name = entity.getName().empty() ? "Entity_" + std::to_string(entityId) : entity.getName();
            } else {
                info.active = false;
                info.name = "Entity_" + std::to_string(entityId);
            }
            
            info.componentTypes = getEntityComponentTypes(entityId);
            
            return info;
        }

        std::vector<ComponentInfo> EntityInspector::getEntityComponents(RPGEngine::EntityId entityId) const {
            std::vector<ComponentInfo> components;
            
            if (!m_componentManager) return components;

            auto componentTypes = getEntityComponentTypes(entityId);
            
            for (const auto& type : componentTypes) {
                ComponentInfo info;
                info.type = type;
                info.data = getComponentData(entityId, type);
                info.enabled = true; // Simplified - would need component-specific enabled state
                
                components.push_back(info);
            }
            
            return components;
        }

        std::vector<EntityInfo> EntityInspector::findEntitiesByName(const std::string& name) const {
            std::vector<EntityInfo> result;
            auto allEntities = getAllEntities();
            
            for (const auto& entity : allEntities) {
                if (entity.name.find(name) != std::string::npos) {
                    result.push_back(entity);
                }
            }
            
            return result;
        }

        std::vector<EntityInfo> EntityInspector::findEntitiesWithComponent(const std::string& componentType) const {
            std::vector<EntityInfo> result;
            auto allEntities = getAllEntities();
            
            for (const auto& entity : allEntities) {
                auto it = std::find(entity.componentTypes.begin(), entity.componentTypes.end(), componentType);
                if (it != entity.componentTypes.end()) {
                    result.push_back(entity);
                }
            }
            
            return result;
        }

        std::vector<EntityInfo> EntityInspector::getActiveEntities() const {
            std::vector<EntityInfo> result;
            auto allEntities = getAllEntities();
            
            for (const auto& entity : allEntities) {
                if (entity.active) {
                    result.push_back(entity);
                }
            }
            
            return result;
        }

        std::vector<EntityInfo> EntityInspector::getInactiveEntities() const {
            std::vector<EntityInfo> result;
            auto allEntities = getAllEntities();
            
            for (const auto& entity : allEntities) {
                if (!entity.active) {
                    result.push_back(entity);
                }
            }
            
            return result;
        }

        void EntityInspector::setEntityActive(RPGEngine::EntityId entityId, bool active) {
            if (m_entityManager) {
                auto entity = m_entityManager->getEntity(entityId);
                m_entityManager->setEntityActive(entity, active);
            }
        }

        void EntityInspector::destroyEntity(RPGEngine::EntityId entityId) {
            if (m_entityManager) {
                auto entity = m_entityManager->getEntity(entityId);
                m_entityManager->destroyEntity(entity);
            }
        }

        RPGEngine::EntityId EntityInspector::createDebugEntity(const std::string& name) {
            if (m_entityManager) {
                auto entity = m_entityManager->createEntity(name);
                return entity.getID();
            }
            return 0;
        }

        std::string EntityInspector::getComponentData(RPGEngine::EntityId entityId, const std::string& componentType) const {
            if (!m_componentManager) return "No component manager";

            std::stringstream ss;
            
            if (componentType == "TransformComponent") {
                auto transform = m_componentManager->getComponent<RPGEngine::Components::TransformComponent>(entityId);
                if (transform) {
                    ss << "Position: (" << transform->getX() << ", " << transform->getY() << ")\n";
                    ss << "Rotation: " << transform->getRotation() << "\n";
                    ss << "Scale: (" << transform->getScaleX() << ", " << transform->getScaleY() << ")";
                }
            }
            else if (componentType == "SpriteComponent") {
                auto sprite = m_componentManager->getComponent<RPGEngine::Components::SpriteComponent>(entityId);
                if (sprite) {
                    ss << "Texture Path: " << sprite->getTexturePath() << "\n";
                    ss << "Size: (" << sprite->getWidth() << ", " << sprite->getHeight() << ")\n";
                    ss << "Visible: " << (sprite->isVisible() ? "true" : "false");
                }
            }
            else if (componentType == "PhysicsComponent") {
                auto physics = m_componentManager->getComponent<RPGEngine::Components::PhysicsComponent>(entityId);
                if (physics) {
                    ss << "Velocity: (" << physics->getVelocityX() << ", " << physics->getVelocityY() << ")\n";
                    ss << "Mass: " << physics->getMass() << "\n";
                    ss << "Friction: " << physics->getFriction();
                }
            }
            else if (componentType == "StatsComponent") {
                auto stats = m_componentManager->getComponent<RPGEngine::Components::StatsComponent>(entityId);
                if (stats) {
                    ss << "HP: " << stats->getCurrentHP() << "/" << stats->getMaxHP() << "\n";
                    ss << "MP: " << stats->getCurrentMP() << "/" << stats->getMaxMP() << "\n";
                    ss << "Level: " << stats->getLevel() << "\n";
                    ss << "Experience: " << "N/A"; // Experience getter not available
                }
            }
            else {
                ss << "Unknown component type: " << componentType;
            }
            
            return ss.str();
        }

        void EntityInspector::setComponentEnabled(RPGEngine::EntityId entityId, const std::string& componentType, bool enabled) {
            // This would need component-specific implementation
            // For now, just a placeholder
        }

        size_t EntityInspector::getTotalEntityCount() const {
            return m_entityManager ? m_entityManager->getEntityCount() : 0;
        }

        size_t EntityInspector::getActiveEntityCount() const {
            return getActiveEntities().size();
        }

        std::vector<std::pair<std::string, size_t>> EntityInspector::getComponentTypeStats() const {
            std::vector<std::pair<std::string, size_t>> stats;
            
            if (!m_componentManager) return stats;

            // Count components by type
            std::map<std::string, size_t> componentCounts;
            auto allEntities = getAllEntities();
            
            for (const auto& entity : allEntities) {
                for (const auto& componentType : entity.componentTypes) {
                    componentCounts[componentType]++;
                }
            }
            
            for (const auto& pair : componentCounts) {
                stats.push_back(pair);
            }
            
            return stats;
        }

        void EntityInspector::update() {
            // Update inspector state if needed
            // Could refresh entity list, check for changes, etc.
        }

        void EntityInspector::renderInspectorUI() {
            // This would integrate with the UI system to render the inspector
            // For now, just a placeholder that could be implemented with ImGui or similar
            
            // Example structure:
            // - Entity list window
            // - Selected entity details window
            // - Component inspector window
            // - Statistics window
        }

        std::string EntityInspector::componentToString(RPGEngine::EntityId entityId, const std::string& componentType) const {
            return getComponentData(entityId, componentType);
        }

        std::vector<std::string> EntityInspector::getEntityComponentTypes(RPGEngine::EntityId entityId) const {
            std::vector<std::string> types;
            
            if (!m_componentManager) return types;

            // Check for each component type
            if (m_componentManager->hasComponent<RPGEngine::Components::TransformComponent>(entityId)) {
                types.push_back("TransformComponent");
            }
            if (m_componentManager->hasComponent<RPGEngine::Components::SpriteComponent>(entityId)) {
                types.push_back("SpriteComponent");
            }
            if (m_componentManager->hasComponent<RPGEngine::Components::PhysicsComponent>(entityId)) {
                types.push_back("PhysicsComponent");
            }
            if (m_componentManager->hasComponent<RPGEngine::Components::StatsComponent>(entityId)) {
                types.push_back("StatsComponent");
            }
            if (m_componentManager->hasComponent<RPGEngine::Components::InventoryComponent>(entityId)) {
                types.push_back("InventoryComponent");
            }
            
            return types;
        }

    } // namespace Debug
} // namespace Engine