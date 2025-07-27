#pragma once

#include "../core/ISystem.h"
#include "../core/IEngine.h"
#include "../core/Types.h"
#include "../core/Event.h"
#include "../core/ThreadPool.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>

namespace RPGEngine {
    
    // Forward declarations
    class System;
    
    /**
     * System update mode enumeration
     */
    enum class SystemUpdateMode {
        All,        // Update all systems
        Fixed,      // Update only systems that use fixed timestep
        Variable    // Update only systems that use variable timestep
    };
    
    /**
     * System event types
     */
    enum class SystemEventType {
        SystemRegistered,
        SystemInitialized,
        SystemShutdown,
        SystemEnabled,
        SystemDisabled
    };
    
    /**
     * System event data
     */
    struct SystemEvent : public Event<SystemEvent> {
        SystemEvent(SystemEventType eventType, SystemType systemType)
            : eventType(eventType), systemType(systemType) {}
        
        SystemEventType eventType;
        SystemType systemType;
    };
    
    /**
     * Manages all engine systems, their lifecycle, and execution order
     */
    class SystemManager {
    public:
        SystemManager();
        ~SystemManager();
        
        /**
         * Register a system with the manager
         * @param system Unique pointer to the system
         * @param systemType Type identifier for the system
         * @return true if registration was successful
         */
        bool registerSystem(std::unique_ptr<ISystem> system, SystemType systemType);
        
        /**
         * Register a system with the manager
         * @param system Shared pointer to the system
         * @param systemType Type identifier for the system
         * @return true if registration was successful
         */
        bool registerSystem(std::shared_ptr<ISystem> system, SystemType systemType);
        
        /**
         * Initialize all registered systems
         * @return true if all systems initialized successfully
         */
        bool initializeAll();
        
        /**
         * Initialize a specific system
         * @param systemType Type of system to initialize
         * @return true if initialization was successful
         */
        bool initializeSystem(SystemType systemType);
        
        /**
         * Update all systems with delta time
         * @param deltaTime Time elapsed since last update in seconds
         * @param mode Update mode (all, fixed, or variable)
         */
        void updateAll(float deltaTime, SystemUpdateMode mode = SystemUpdateMode::All);
        
        /**
         * Update systems in parallel where possible
         * @param deltaTime Time elapsed since last update in seconds
         * @param mode Update mode (all, fixed, or variable)
         */
        void updateAllParallel(float deltaTime, SystemUpdateMode mode = SystemUpdateMode::All);
        
        /**
         * Update a specific system
         * @param systemType Type of system to update
         * @param deltaTime Time elapsed since last update in seconds
         * @return true if the system was updated
         */
        bool updateSystem(SystemType systemType, float deltaTime);
        
        /**
         * Shutdown all systems
         */
        void shutdownAll();
        
        /**
         * Shutdown a specific system
         * @param systemType Type of system to shutdown
         * @return true if the system was shut down
         */
        bool shutdownSystem(SystemType systemType);
        
        /**
         * Get a system by type
         * @param systemType The type of system to retrieve
         * @return Pointer to the system, or nullptr if not found
         */
        template<typename T>
        T* getSystem(SystemType systemType);
        
        /**
         * Check if a system is registered
         * @param systemType The type of system to check
         * @return true if system is registered
         */
        bool hasSystem(SystemType systemType) const;
        
        /**
         * Check if a system is initialized
         * @param systemType The type of system to check
         * @return true if system is initialized
         */
        bool isSystemInitialized(SystemType systemType) const;
        
        /**
         * Enable or disable a system
         * @param systemType Type of system to enable/disable
         * @param enabled true to enable, false to disable
         * @return true if the system state was changed
         */
        bool setSystemEnabled(SystemType systemType, bool enabled);
        
        /**
         * Check if a system is enabled
         * @param systemType Type of system to check
         * @return true if the system is enabled
         */
        bool isSystemEnabled(SystemType systemType) const;
        
        /**
         * Set a system's priority
         * @param systemType Type of system to set priority for
         * @param priority Priority value (lower = higher priority)
         * @return true if the priority was set
         */
        bool setSystemPriority(SystemType systemType, int priority);
        
        /**
         * Add a dependency between systems
         * @param dependentType Type of system that depends on another
         * @param dependencyType Type of system that is depended on
         * @return true if the dependency was added
         */
        bool addSystemDependency(SystemType dependentType, SystemType dependencyType);
        
        /**
         * Get the number of registered systems
         * @return Number of systems
         */
        size_t getSystemCount() const;
        
        /**
         * Get the event dispatcher for system events
         * @return Reference to the event dispatcher
         */
        EventDispatcher& getEventDispatcher() { return m_eventDispatcher; }
        
        /**
         * Get the execution order of systems
         * @return Vector of system types in execution order
         */
        const std::vector<SystemType>& getExecutionOrder() const { return m_executionOrder; }
        
        /**
         * Enable or disable parallel system updates
         * @param enabled true to enable parallel updates
         */
        void setParallelUpdatesEnabled(bool enabled) { m_parallelUpdatesEnabled = enabled; }
        
        /**
         * Check if parallel updates are enabled
         * @return true if parallel updates are enabled
         */
        bool isParallelUpdatesEnabled() const { return m_parallelUpdatesEnabled; }
        
    private:
        struct SystemEntry {
            std::shared_ptr<ISystem> system;
            SystemType type;
            int priority; // Lower values = higher priority (updated first)
            bool enabled;
            bool useFixedTimestep;
            std::unordered_set<SystemType> dependencies;
            
            // Make it movable
            SystemEntry() = default;
            SystemEntry(SystemEntry&&) = default;
            SystemEntry& operator=(SystemEntry&&) = default;
            
            // Delete copy operations
            SystemEntry(const SystemEntry&) = delete;
            SystemEntry& operator=(const SystemEntry&) = delete;
        };
        
        std::unordered_map<SystemType, SystemEntry> m_systems;
        std::vector<SystemType> m_executionOrder;
        bool m_initialized;
        bool m_parallelUpdatesEnabled;
        EventDispatcher m_eventDispatcher;
        std::unique_ptr<Core::ThreadPool> m_threadPool;
        
        /**
         * Update the execution order based on dependencies and priorities
         */
        void updateExecutionOrder();
        
        /**
         * Check if there are any cyclic dependencies
         * @return true if there are cyclic dependencies
         */
        bool hasCyclicDependencies() const;
        
        /**
         * Helper method for cycle detection
         * @param systemType Current system being checked
         * @param visited Map of visited systems
         * @param recursionStack Map of systems in the current recursion stack
         * @return true if a cycle is detected
         */
        bool hasCyclicDependenciesUtil(SystemType systemType, 
                                      std::unordered_map<SystemType, bool>& visited,
                                      std::unordered_map<SystemType, bool>& recursionStack) const;
        
        /**
         * Get the default priority for a system type
         * @param type System type
         * @return Default priority value
         */
        int getDefaultPriority(SystemType type) const;
        
        /**
         * Dispatch a system event
         * @param eventType Type of system event
         * @param systemType Type of system the event is for
         */
        void dispatchSystemEvent(SystemEventType eventType, SystemType systemType);
        
        /**
         * Check if all dependencies of a system are initialized
         * @param systemType Type of system to check
         * @return true if all dependencies are initialized
         */
        bool areDependenciesInitialized(SystemType systemType) const;
    };
    
    // Template implementation
    template<typename T>
    T* SystemManager::getSystem(SystemType systemType) {
        auto it = m_systems.find(systemType);
        if (it != m_systems.end()) {
            return static_cast<T*>(it->second.system.get());
        }
        return nullptr;
    }
    
} // namespace RPGEngine