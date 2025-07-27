#pragma once

#include <cstdint>
#include <typeindex>

namespace RPGEngine {
    namespace Components {
        
        // Type definitions
        using EntityId = uint32_t;
        using ComponentId = uint32_t;
        
        // Invalid entity constant
        static constexpr EntityId INVALID_ENTITY = 0;
        
        /**
         * Base component interface
         * All components must inherit from this class
         */
        class IComponent {
        public:
            IComponent(EntityId entityId) : m_entityId(entityId) {}
            virtual ~IComponent() = default;
            
            EntityId getEntityId() const { return m_entityId; }
            virtual std::type_index getType() const = 0;
            
        protected:
            EntityId m_entityId;
        };
        
        /**
         * Template base class for typed components
         */
        template<typename T>
        class Component : public IComponent {
        public:
            Component(EntityId entityId) : IComponent(entityId) {}
            
            std::type_index getType() const override {
                return std::type_index(typeid(T));
            }
            
            static std::type_index getStaticType() {
                return std::type_index(typeid(T));
            }
        };
        
        /**
         * Component type registry for runtime type information
         */
        class ComponentTypeRegistry {
        public:
            template<typename T>
            static ComponentId getComponentId() {
                static ComponentId id = generateId();
                return id;
            }
            
            template<typename T>
            static const char* getComponentName() {
                return typeid(T).name();
            }
            
        private:
            static ComponentId generateId() {
                static ComponentId nextId = 1;
                return nextId++;
            }
        };
        
    } // namespace Components
} // namespace RPGEngine