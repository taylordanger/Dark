#pragma once

#include "../core/ISystem.h"
#include "../core/IEngine.h"  // For SystemType
#include "../core/Event.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

namespace RPGEngine {
    
    // Forward declarations
    class SystemManager;
    
    /**
     * Base class for all engine systems
     * Implements common functionality for system lifecycle management
     */
    class System : public ISystem {
    public:
        /**
         * Constructor
         * @param name System name for debugging
         */
        explicit System(const std::string& name);
        
        /**
         * Virtual destructor
         */
        virtual ~System() = default;
        
        // ISystem interface implementation
        bool initialize() override;
        void update(float deltaTime) override;
        void shutdown() override;
        const std::string& getName() const override { return m_name; }
        bool isInitialized() const override { return m_initialized; }
        
        /**
         * Set the system manager that owns this system
         * @param manager Pointer to the system manager
         */
        void setSystemManager(SystemManager* manager) { m_systemManager = manager; }
        
        /**
         * Add a dependency on another system
         * @param systemType Type of system this system depends on
         */
        void addDependency(SystemType systemType);
        
        /**
         * Get the dependencies of this system
         * @return Set of system types this system depends on
         */
        const std::unordered_set<SystemType>& getDependencies() const { return m_dependencies; }
        
        /**
         * Check if this system has a dependency on another system
         * @param systemType Type of system to check
         * @return true if this system depends on the specified system
         */
        bool hasDependency(SystemType systemType) const;
        
        /**
         * Set whether this system should use fixed timestep updates
         * @param useFixedTimestep true to use fixed timestep, false for variable
         */
        void setUseFixedTimestep(bool useFixedTimestep) { m_useFixedTimestep = useFixedTimestep; }
        
        /**
         * Check if this system uses fixed timestep updates
         * @return true if this system uses fixed timestep
         */
        bool usesFixedTimestep() const { return m_useFixedTimestep; }
        
        /**
         * Set the priority of this system
         * @param priority Priority value (lower = higher priority)
         */
        void setPriority(int priority) { m_priority = priority; }
        
        /**
         * Get the priority of this system
         * @return Priority value
         */
        int getPriority() const { return m_priority; }
        
        /**
         * Set whether this system is enabled
         * @param enabled true to enable, false to disable
         */
        void setEnabled(bool enabled) { m_enabled = enabled; }
        
        /**
         * Check if this system is enabled
         * @return true if this system is enabled
         */
        bool isEnabled() const { return m_enabled; }
        
    protected:
        /**
         * Called during initialization
         * Override this method to implement system-specific initialization
         * @return true if initialization was successful
         */
        virtual bool onInitialize() { return true; }
        
        /**
         * Called during update
         * Override this method to implement system-specific update logic
         * @param deltaTime Time elapsed since last update in seconds
         */
        virtual void onUpdate(float deltaTime) {}
        
        /**
         * Called during shutdown
         * Override this method to implement system-specific cleanup
         */
        virtual void onShutdown() {}
        
        /**
         * Get a system from the system manager
         * @param systemType Type of system to retrieve
         * @return Pointer to the system, or nullptr if not found
         */
        template<typename T>
        T* getSystem(SystemType systemType) const;
        
        // System state
        std::string m_name;
        bool m_initialized;
        bool m_enabled;
        bool m_useFixedTimestep;
        int m_priority;
        
        // Dependencies
        std::unordered_set<SystemType> m_dependencies;
        
        // System manager reference
        SystemManager* m_systemManager;
    };
    
} // namespace RPGEngine