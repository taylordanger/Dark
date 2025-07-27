#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include "../core/ConfigurationManager.h"
#include "../components/Component.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <ctime>

namespace RPGEngine {
    namespace Entities {

        /**
         * Component factory function type - using void* to avoid template issues
         */
        using ComponentFactory = std::function<void*(RPGEngine::EntityId, const Core::ConfigValue&)>;

        /**
         * Entity template for data-driven entity creation
         */
        struct EntityTemplate {
            std::string name;
            std::unordered_map<std::string, Core::ConfigValue> components;
            Core::ConfigValue properties;

            EntityTemplate() = default;
            EntityTemplate(const Core::ConfigValue& config);
        };

        /**
         * Factory for creating entities from configuration data
         */
        class EntityFactory {
        public:
            EntityFactory(std::shared_ptr<EntityManager> entityManager);
            ~EntityFactory() = default;

            // Template management
            void loadTemplatesFromFile(const std::string& filePath);
            void loadTemplatesFromConfig(const Core::ConfigurationManager& config);
            void registerTemplate(const std::string& name, const EntityTemplate& entityTemplate);
            bool hasTemplate(const std::string& name) const;
            const EntityTemplate& getTemplate(const std::string& name) const;

            // Component factory registration
            void registerComponentFactory(const std::string& componentType, ComponentFactory factory);
            bool hasComponentFactory(const std::string& componentType) const;

            // Entity creation
            RPGEngine::EntityId createEntity(const std::string& templateName);
            RPGEngine::EntityId createEntity(const std::string& templateName, const Core::ConfigValue& overrides);
            RPGEngine::EntityId createEntityFromConfig(const Core::ConfigValue& config);

            // Batch entity creation
            std::vector<RPGEngine::EntityId> createEntitiesFromFile(const std::string& filePath);
            std::vector<RPGEngine::EntityId> createEntitiesFromConfig(const Core::ConfigValue& entitiesConfig);

            // Entity serialization
            Core::ConfigValue serializeEntity(RPGEngine::EntityId entityId) const;
            bool deserializeEntity(RPGEngine::EntityId entityId, const Core::ConfigValue& config);

            // Hot reloading support
            void enableHotReloading(bool enable) { hotReloadingEnabled_ = enable; }
            void checkForTemplateChanges();

        private:
            std::shared_ptr<EntityManager> entityManager_;
            std::unordered_map<std::string, EntityTemplate> templates_;
            std::unordered_map<std::string, ComponentFactory> componentFactories_;
            std::unordered_map<std::string, std::time_t> templateFileTimestamps_;
            bool hotReloadingEnabled_ = false;

            // Helper methods
            void* createComponent(const std::string& componentType, 
                                RPGEngine::EntityId entityId, 
                                const Core::ConfigValue& config);
            void setupDefaultComponentFactories();
            std::time_t getFileModificationTime(const std::string& filename) const;
        };

    } // namespace Entities
} // namespace RPGEngine