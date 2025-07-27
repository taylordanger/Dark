#include "EntityFactory.h"
#include "EntityManager.h"
#include "../components/ScriptComponent.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace RPGEngine {
    namespace Entities {

        // EntityTemplate implementation
        EntityTemplate::EntityTemplate(const Core::ConfigValue& config) {
            if (config.hasKey("name")) {
                name = config["name"].asString();
            }

            if (config.hasKey("components")) {
                const Core::ConfigValue& componentsConfig = config["components"];
                for (const std::string& key : componentsConfig.getKeys()) {
                    components[key] = componentsConfig[key];
                }
            }

            if (config.hasKey("properties")) {
                properties = config["properties"];
            }
        }

        // EntityFactory implementation
        EntityFactory::EntityFactory(std::shared_ptr<EntityManager> entityManager) 
            : entityManager_(entityManager), hotReloadingEnabled_(false) {
            setupDefaultComponentFactories();
        }

        void EntityFactory::loadTemplatesFromFile(const std::string& filePath) {
            Core::ConfigurationManager config;
            if (!config.loadFromFile(filePath)) {
                std::cerr << "Failed to load entity templates from: " << filePath << std::endl;
                return;
            }

            loadTemplatesFromConfig(config);
            templateFileTimestamps_[filePath] = getFileModificationTime(filePath);
        }

        void EntityFactory::loadTemplatesFromConfig(const Core::ConfigurationManager& config) {
            Core::ConfigValue templatesConfig = config.get("templates");
            if (templatesConfig.isObject()) {
                for (const std::string& templateName : templatesConfig.getKeys()) {
                    EntityTemplate entityTemplate(templatesConfig[templateName]);
                    entityTemplate.name = templateName;
                    registerTemplate(templateName, entityTemplate);
                }
            }
        }

        void EntityFactory::registerTemplate(const std::string& name, const EntityTemplate& entityTemplate) {
            templates_[name] = entityTemplate;
            std::cout << "Registered entity template: " << name << std::endl;
        }

        bool EntityFactory::hasTemplate(const std::string& name) const {
            return templates_.find(name) != templates_.end();
        }

        const EntityTemplate& EntityFactory::getTemplate(const std::string& name) const {
            static EntityTemplate emptyTemplate;
            auto it = templates_.find(name);
            return (it != templates_.end()) ? it->second : emptyTemplate;
        }

        void EntityFactory::registerComponentFactory(const std::string& componentType, ComponentFactory factory) {
            componentFactories_[componentType] = factory;
        }

        bool EntityFactory::hasComponentFactory(const std::string& componentType) const {
            return componentFactories_.find(componentType) != componentFactories_.end();
        }

        RPGEngine::EntityId EntityFactory::createEntity(const std::string& templateName) {
            return createEntity(templateName, Core::ConfigValue());
        }

        RPGEngine::EntityId EntityFactory::createEntity(const std::string& templateName, const Core::ConfigValue& overrides) {
            if (!hasTemplate(templateName)) {
                std::cerr << "Entity template not found: " << templateName << std::endl;
                return 0;
            }

            const EntityTemplate& entityTemplate = getTemplate(templateName);
            Entity entity = entityManager_->createEntity();
            RPGEngine::EntityId entityId = entity.getID();

            // Create components from template
            for (const auto& pair : entityTemplate.components) {
                const std::string& componentType = pair.first;
                Core::ConfigValue componentConfig = pair.second;

                // Apply overrides if they exist
                if (overrides.hasKey("components") && overrides["components"].hasKey(componentType)) {
                    // TODO: Merge component config with overrides
                    componentConfig = overrides["components"][componentType];
                }

                auto component = createComponent(componentType, entityId, componentConfig);
                if (component) {
                    // TODO: Add component to entity through component manager
                    std::cout << "Created component " << componentType << " for entity " << entityId << std::endl;
                }
            }

            return entityId;
        }

        RPGEngine::EntityId EntityFactory::createEntityFromConfig(const Core::ConfigValue& config) {
            EntityTemplate tempTemplate(config);
            Entity entity = entityManager_->createEntity();
            RPGEngine::EntityId entityId = entity.getID();

            // Create components
            for (const auto& pair : tempTemplate.components) {
                const std::string& componentType = pair.first;
                const Core::ConfigValue& componentConfig = pair.second;

                auto component = createComponent(componentType, entityId, componentConfig);
                if (component) {
                    std::cout << "Created component " << componentType << " for entity " << entityId << std::endl;
                }
            }

            return entityId;
        }

        std::vector<Components::EntityId> EntityFactory::createEntitiesFromFile(const std::string& filePath) {
            Core::ConfigurationManager config;
            if (!config.loadFromFile(filePath)) {
                std::cerr << "Failed to load entities from: " << filePath << std::endl;
                return {};
            }

            return createEntitiesFromConfig(config.get("entities"));
        }

        std::vector<Components::EntityId> EntityFactory::createEntitiesFromConfig(const Core::ConfigValue& entitiesConfig) {
            std::vector<Components::EntityId> entities;

            if (entitiesConfig.isArray()) {
                for (size_t i = 0; i < entitiesConfig.size(); ++i) {
                    const Core::ConfigValue& entityConfig = entitiesConfig[i];
                    
                    Components::EntityId entityId;
                    if (entityConfig.hasKey("template")) {
                        std::string templateName = entityConfig["template"].asString();
                        entityId = createEntity(templateName, entityConfig);
                    } else {
                        entityId = createEntityFromConfig(entityConfig);
                    }
                    
                    if (entityId != 0) {
                        entities.push_back(entityId);
                    }
                }
            }

            return entities;
        }

        Core::ConfigValue EntityFactory::serializeEntity(Components::EntityId entityId) const {
            // TODO: Implement entity serialization
            nlohmann::json entityJson;
            entityJson["id"] = entityId;
            entityJson["components"] = nlohmann::json::object();
            return Core::ConfigValue(entityJson);
        }

        bool EntityFactory::deserializeEntity(Components::EntityId entityId, const Core::ConfigValue& config) {
            // TODO: Implement entity deserialization
            return true;
        }

        void EntityFactory::checkForTemplateChanges() {
            if (!hotReloadingEnabled_) {
                return;
            }

            for (auto& pair : templateFileTimestamps_) {
                const std::string& filePath = pair.first;
                std::time_t& lastModified = pair.second;
                
                std::time_t currentModified = getFileModificationTime(filePath);
                if (currentModified > lastModified) {
                    std::cout << "Entity template file changed, reloading: " << filePath << std::endl;
                    loadTemplatesFromFile(filePath);
                    lastModified = currentModified;
                }
            }
        }

        void* EntityFactory::createComponent(const std::string& componentType, 
                                    Components::EntityId entityId, 
                                    const Core::ConfigValue& config) {
            auto it = componentFactories_.find(componentType);
            if (it != componentFactories_.end()) {
                return it->second(entityId, config);
            }

            std::cerr << "No factory registered for component type: " << componentType << std::endl;
            return nullptr;
        }

        void EntityFactory::setupDefaultComponentFactories() {
            // Register ScriptComponent factory
            registerComponentFactory("ScriptComponent", [](Components::EntityId entityId, const Core::ConfigValue& config) -> void* {
                auto component = std::make_shared<Components::ScriptComponent>(entityId);
                
                if (config.hasKey("script")) {
                    component->setScript(config["script"].asString());
                }
                
                if (config.hasKey("scriptFile")) {
                    component->setScriptFile(config["scriptFile"].asString());
                }
                
                if (config.hasKey("eventHandlers")) {
                    const Core::ConfigValue& handlers = config["eventHandlers"];
                    for (const std::string& eventName : handlers.getKeys()) {
                        component->registerScriptEvent(eventName, handlers[eventName].asString());
                    }
                }
                
                return component.get();
            });

            // Register StatsComponent factory
            registerComponentFactory("StatsComponent", [](Components::EntityId entityId, const Core::ConfigValue& config) -> void* {
                auto component = std::make_shared<Components::StatsComponent>(entityId);
                
                if (config.hasKey("health")) {
                    component->setCurrentHP(config["health"].asFloat());
                }
                
                if (config.hasKey("maxHealth")) {
                    component->setBaseMaxHP(config["maxHealth"].asFloat());
                }
                
                if (config.hasKey("mana")) {
                    component->setCurrentMP(config["mana"].asFloat());
                }
                
                if (config.hasKey("maxMana")) {
                    component->setBaseMaxMP(config["maxMana"].asFloat());
                }
                
                if (config.hasKey("level")) {
                    component->setLevel(config["level"].asInt());
                }
                
                if (config.hasKey("experience")) {
                    component->addExperience(config["experience"].asInt());
                }
                
                return component.get();
            });

            // Register InventoryComponent factory
            registerComponentFactory("InventoryComponent", [](Components::EntityId entityId, const Core::ConfigValue& config) -> void* {
                auto component = std::make_shared<Components::InventoryComponent>(entityId);
                
                if (config.hasKey("capacity")) {
                    component->setCapacity(config["capacity"].asInt());
                }
                
                if (config.hasKey("items")) {
                    const Core::ConfigValue& items = config["items"];
                    if (items.isArray()) {
                        for (size_t i = 0; i < items.size(); ++i) {
                            const Core::ConfigValue& item = items[i];
                            if (item.hasKey("id") && item.hasKey("quantity")) {
                                std::string itemId = item["id"].asString();
                                int quantity = item["quantity"].asInt();
                                component->addItem(itemId, quantity);
                            }
                        }
                    }
                }
                
                return component.get();
            });
        }

        std::time_t EntityFactory::getFileModificationTime(const std::string& filename) const {
            struct stat fileInfo;
            if (stat(filename.c_str(), &fileInfo) == 0) {
                return fileInfo.st_mtime;
            }
            return 0;
        }

    } // namespace Entities
} // namespace RPGEngine