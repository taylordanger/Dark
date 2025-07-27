#pragma once

#include "../core/Types.h"

namespace RPGEngine {
    
    enum class ComponentType {
        Transform,
        Sprite,
        Physics,
        Stats,
        Inventory,
        AI,
        Animation,
        Audio,
        Collision,
        Trigger,
        Dialogue,
        Quest,
        Script
    };
    
    /**
     * Base interface for all components
     */
    class IComponent {
    public:
        virtual ~IComponent() = default;
        
        /**
         * Get the entity ID this component belongs to
         * @return Entity ID
         */
        EntityId getEntityId() const { return m_entityId; }
        
        /**
         * Get the component type
         * @return Component type identifier
         */
        virtual ComponentType getType() const = 0;
        
        /**
         * Check if the component is active
         * @return true if component is active
         */
        bool isActive() const { return m_active; }
        
        /**
         * Set component active state
         * @param active New active state
         */
        void setActive(bool active) { m_active = active; }
        
    protected:
        explicit IComponent(EntityId entityId) 
            : m_entityId(entityId), m_active(true) {}
        
    private:
        EntityId m_entityId;
        bool m_active;
    };
    
} // namespace RPGEngine