#include "SystemManager.h"
#include "System.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>

namespace RPGEngine {
    
    SystemManager::SystemManager() 
        : m_initialized(false)
        , m_parallelUpdatesEnabled(false)
        , m_threadPool(std::make_unique<Core::ThreadPool>())
    {
    }
    
    SystemManager::~SystemManager() {
        if (m_initialized) {
            shutdownAll();
        }
    }
    
    bool SystemManager::registerSystem(std::unique_ptr<ISystem> system, SystemType systemType) {
        if (!system) {
            std::cerr << "SystemManager: Cannot register null system" << std::endl;
            return false;
        }
        
        return registerSystem(std::shared_ptr<ISystem>(std::move(system)), systemType);
    }
    
    bool SystemManager::registerSystem(std::shared_ptr<ISystem> system, SystemType systemType) {
        if (!system) {
            std::cerr << "SystemManager: Cannot register null system" << std::endl;
            return false;
        }
        
        // Check if system type is already registered
        if (hasSystem(systemType)) {
            std::cerr << "SystemManager: System type already registered" << std::endl;
            return false;
        }
        
        // Set up system entry
        SystemEntry entry;
        entry.system = system;
        entry.type = systemType;
        entry.priority = getDefaultPriority(systemType);
        entry.enabled = true;
        entry.useFixedTimestep = (systemType == SystemType::Physics); // Physics uses fixed timestep by default
        
        // If this is a System (not just ISystem), set the system manager
        System* baseSystem = dynamic_cast<System*>(system.get());
        if (baseSystem) {
            baseSystem->setSystemManager(this);
            
            // Copy dependencies from the system
            entry.dependencies = baseSystem->getDependencies();
            
            // Set fixed timestep flag
            entry.useFixedTimestep = baseSystem->usesFixedTimestep();
        }
        
        // Add the system to the map
        m_systems[systemType] = std::move(entry);
        
        // Update execution order
        updateExecutionOrder();
        
        // Dispatch system registered event
        dispatchSystemEvent(SystemEventType::SystemRegistered, systemType);
        
        std::cout << "SystemManager: Registered system of type " << static_cast<int>(systemType) << std::endl;
        return true;
    }
    
    bool SystemManager::initializeAll() {
        if (m_initialized) {
            std::cerr << "SystemManager: Systems already initialized" << std::endl;
            return false;
        }
        
        std::cout << "SystemManager: Initializing " << m_systems.size() << " systems..." << std::endl;
        
        // Check for cyclic dependencies
        if (hasCyclicDependencies()) {
            std::cerr << "SystemManager: Cyclic dependencies detected, cannot initialize systems" << std::endl;
            return false;
        }
        
        // Initialize systems in dependency order
        for (SystemType systemType : m_executionOrder) {
            if (!initializeSystem(systemType)) {
                std::cerr << "SystemManager: Failed to initialize system: " << static_cast<int>(systemType) << std::endl;
                
                // Shutdown any systems that were already initialized
                shutdownAll();
                return false;
            }
        }
        
        m_initialized = true;
        std::cout << "SystemManager: All systems initialized successfully" << std::endl;
        return true;
    }
    
    bool SystemManager::initializeSystem(SystemType systemType) {
        auto it = m_systems.find(systemType);
        if (it == m_systems.end()) {
            std::cerr << "SystemManager: Cannot initialize non-existent system: " << static_cast<int>(systemType) << std::endl;
            return false;
        }
        
        // Check if already initialized
        if (it->second.system->isInitialized()) {
            return true;
        }
        
        // Check if all dependencies are initialized
        if (!areDependenciesInitialized(systemType)) {
            std::cerr << "SystemManager: Cannot initialize system " << static_cast<int>(systemType) 
                      << " because not all dependencies are initialized" << std::endl;
            return false;
        }
        
        // Initialize the system
        std::cout << "Initializing system: " << it->second.system->getName() << std::endl;
        if (!it->second.system->initialize()) {
            std::cerr << "SystemManager: Failed to initialize system: " << it->second.system->getName() << std::endl;
            return false;
        }
        
        // Dispatch system initialized event
        dispatchSystemEvent(SystemEventType::SystemInitialized, systemType);
        
        return true;
    }
    
    void SystemManager::updateAll(float deltaTime, SystemUpdateMode mode) {
        if (!m_initialized) {
            return;
        }
        
        if (m_parallelUpdatesEnabled) {
            updateAllParallel(deltaTime, mode);
            return;
        }
        
        // Update systems in execution order
        for (SystemType systemType : m_executionOrder) {
            auto it = m_systems.find(systemType);
            if (it != m_systems.end() && it->second.enabled && it->second.system->isInitialized()) {
                // Check update mode
                if (mode == SystemUpdateMode::All ||
                    (mode == SystemUpdateMode::Fixed && it->second.useFixedTimestep) ||
                    (mode == SystemUpdateMode::Variable && !it->second.useFixedTimestep)) {
                    it->second.system->update(deltaTime);
                }
            }
        }
    }
    
    void SystemManager::updateAllParallel(float deltaTime, SystemUpdateMode mode) {
        if (!m_initialized || !m_threadPool) {
            return;
        }
        
        // Group systems by dependency level for parallel execution
        std::vector<std::vector<SystemType>> dependencyLevels;
        std::unordered_map<SystemType, int> systemLevels;
        
        // Calculate dependency levels
        for (SystemType systemType : m_executionOrder) {
            auto it = m_systems.find(systemType);
            if (it != m_systems.end() && it->second.enabled && it->second.system->isInitialized()) {
                int level = 0;
                for (SystemType dep : it->second.dependencies) {
                    auto depIt = systemLevels.find(dep);
                    if (depIt != systemLevels.end()) {
                        level = std::max(level, depIt->second + 1);
                    }
                }
                
                systemLevels[systemType] = level;
                
                // Ensure we have enough levels
                while (dependencyLevels.size() <= static_cast<size_t>(level)) {
                    dependencyLevels.emplace_back();
                }
                
                dependencyLevels[level].push_back(systemType);
            }
        }
        
        // Execute systems level by level
        for (const auto& level : dependencyLevels) {
            std::vector<std::future<void>> futures;
            
            for (SystemType systemType : level) {
                auto it = m_systems.find(systemType);
                if (it != m_systems.end()) {
                    // Check update mode
                    if (mode == SystemUpdateMode::All ||
                        (mode == SystemUpdateMode::Fixed && it->second.useFixedTimestep) ||
                        (mode == SystemUpdateMode::Variable && !it->second.useFixedTimestep)) {
                        
                        futures.push_back(m_threadPool->submit([&it, deltaTime]() {
                            it->second.system->update(deltaTime);
                        }));
                    }
                }
            }
            
            // Wait for all systems in this level to complete
            for (auto& future : futures) {
                future.wait();
            }
        }
    }
    
    bool SystemManager::updateSystem(SystemType systemType, float deltaTime) {
        auto it = m_systems.find(systemType);
        if (it == m_systems.end() || !it->second.enabled || !it->second.system->isInitialized()) {
            return false;
        }
        
        it->second.system->update(deltaTime);
        return true;
    }
    
    void SystemManager::shutdownAll() {
        if (!m_initialized) {
            return;
        }
        
        std::cout << "SystemManager: Shutting down systems..." << std::endl;
        
        // Shutdown in reverse execution order
        for (auto it = m_executionOrder.rbegin(); it != m_executionOrder.rend(); ++it) {
            shutdownSystem(*it);
        }
        
        m_initialized = false;
        std::cout << "SystemManager: All systems shut down" << std::endl;
    }
    
    bool SystemManager::shutdownSystem(SystemType systemType) {
        auto it = m_systems.find(systemType);
        if (it == m_systems.end() || !it->second.system->isInitialized()) {
            return false;
        }
        
        std::cout << "Shutting down system: " << it->second.system->getName() << std::endl;
        it->second.system->shutdown();
        
        // Dispatch system shutdown event
        dispatchSystemEvent(SystemEventType::SystemShutdown, systemType);
        
        return true;
    }
    
    bool SystemManager::hasSystem(SystemType systemType) const {
        return m_systems.find(systemType) != m_systems.end();
    }
    
    bool SystemManager::isSystemInitialized(SystemType systemType) const {
        auto it = m_systems.find(systemType);
        return it != m_systems.end() && it->second.system->isInitialized();
    }
    
    bool SystemManager::setSystemEnabled(SystemType systemType, bool enabled) {
        auto it = m_systems.find(systemType);
        if (it == m_systems.end()) {
            return false;
        }
        
        if (it->second.enabled != enabled) {
            it->second.enabled = enabled;
            
            // Dispatch system enabled/disabled event
            dispatchSystemEvent(enabled ? SystemEventType::SystemEnabled : SystemEventType::SystemDisabled, systemType);
        }
        
        return true;
    }
    
    bool SystemManager::isSystemEnabled(SystemType systemType) const {
        auto it = m_systems.find(systemType);
        return it != m_systems.end() && it->second.enabled;
    }
    
    bool SystemManager::setSystemPriority(SystemType systemType, int priority) {
        auto it = m_systems.find(systemType);
        if (it == m_systems.end()) {
            return false;
        }
        
        it->second.priority = priority;
        
        // Update execution order
        updateExecutionOrder();
        
        return true;
    }
    
    bool SystemManager::addSystemDependency(SystemType dependentType, SystemType dependencyType) {
        // Check if both systems exist
        if (!hasSystem(dependentType) || !hasSystem(dependencyType)) {
            return false;
        }
        
        // Add dependency
        m_systems[dependentType].dependencies.insert(dependencyType);
        
        // If the dependent system is a System (not just ISystem), update its dependencies
        System* baseSystem = dynamic_cast<System*>(m_systems[dependentType].system.get());
        if (baseSystem) {
            baseSystem->addDependency(dependencyType);
        }
        
        // Update execution order
        updateExecutionOrder();
        
        return true;
    }
    
    size_t SystemManager::getSystemCount() const {
        return m_systems.size();
    }
    
    void SystemManager::updateExecutionOrder() {
        m_executionOrder.clear();
        
        // Check for cyclic dependencies
        if (hasCyclicDependencies()) {
            std::cerr << "SystemManager: Cyclic dependencies detected, using priority-based order" << std::endl;
            
            // Fall back to priority-based order
            std::vector<std::pair<SystemType, int>> systemsByPriority;
            for (const auto& pair : m_systems) {
                systemsByPriority.push_back({pair.first, pair.second.priority});
            }
            
            // Sort by priority
            std::sort(systemsByPriority.begin(), systemsByPriority.end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                });
            
            // Extract system types in priority order
            for (const auto& pair : systemsByPriority) {
                m_executionOrder.push_back(pair.first);
            }
            
            return;
        }
        
        // Topological sort using Kahn's algorithm
        std::unordered_map<SystemType, int> inDegree;
        std::queue<SystemType> queue;
        
        // Initialize in-degree for all systems
        for (const auto& pair : m_systems) {
            inDegree[pair.first] = 0;
        }
        
        // Calculate in-degree for each system
        for (const auto& pair : m_systems) {
            for (SystemType dependency : pair.second.dependencies) {
                inDegree[dependency]++;
            }
        }
        
        // Add systems with no dependencies to the queue
        for (const auto& pair : inDegree) {
            if (pair.second == 0) {
                queue.push(pair.first);
            }
        }
        
        // Process the queue
        while (!queue.empty()) {
            SystemType current = queue.front();
            queue.pop();
            m_executionOrder.push_back(current);
            
            // Reduce in-degree for systems that depend on the current system
            for (const auto& pair : m_systems) {
                if (pair.second.dependencies.find(current) != pair.second.dependencies.end()) {
                    inDegree[pair.first]--;
                    if (inDegree[pair.first] == 0) {
                        queue.push(pair.first);
                    }
                }
            }
        }
        
        // If we couldn't process all systems, there's a cycle
        if (m_executionOrder.size() != m_systems.size()) {
            std::cerr << "SystemManager: Cyclic dependencies detected, using priority-based order" << std::endl;
            
            // Fall back to priority-based order
            m_executionOrder.clear();
            std::vector<std::pair<SystemType, int>> systemsByPriority;
            for (const auto& pair : m_systems) {
                systemsByPriority.push_back({pair.first, pair.second.priority});
            }
            
            // Sort by priority
            std::sort(systemsByPriority.begin(), systemsByPriority.end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                });
            
            // Extract system types in priority order
            for (const auto& pair : systemsByPriority) {
                m_executionOrder.push_back(pair.first);
            }
        }
    }
    
    bool SystemManager::hasCyclicDependencies() const {
        std::unordered_map<SystemType, bool> visited;
        std::unordered_map<SystemType, bool> recursionStack;
        
        // Initialize visited and recursion stack
        for (const auto& pair : m_systems) {
            visited[pair.first] = false;
            recursionStack[pair.first] = false;
        }
        
        // Check for cycles starting from each system
        for (const auto& pair : m_systems) {
            if (!visited[pair.first]) {
                if (hasCyclicDependenciesUtil(pair.first, visited, recursionStack)) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    bool SystemManager::hasCyclicDependenciesUtil(SystemType systemType, 
                                                std::unordered_map<SystemType, bool>& visited,
                                                std::unordered_map<SystemType, bool>& recursionStack) const {
        // Mark the current node as visited and part of recursion stack
        visited[systemType] = true;
        recursionStack[systemType] = true;
        
        // Check all dependencies
        const auto& dependencies = m_systems.at(systemType).dependencies;
        for (SystemType dependency : dependencies) {
            // If the dependency is not visited, check it recursively
            if (!visited[dependency]) {
                if (hasCyclicDependenciesUtil(dependency, visited, recursionStack)) {
                    return true;
                }
            }
            // If the dependency is in the recursion stack, there's a cycle
            else if (recursionStack[dependency]) {
                return true;
            }
        }
        
        // Remove the system from recursion stack
        recursionStack[systemType] = false;
        
        return false;
    }
    
    int SystemManager::getDefaultPriority(SystemType type) const {
        // Define system update priorities (lower = higher priority)
        switch (type) {
            case SystemType::Input:     return 0;  // Input first
            case SystemType::ECS:       return 1;  // Entity updates
            case SystemType::Physics:   return 2;  // Physics simulation
            case SystemType::Audio:     return 3;  // Audio processing
            case SystemType::Rendering: return 4;  // Rendering last
            case SystemType::Resource:  return 5;  // Resource management
            case SystemType::Scene:     return 6;  // Scene management
            default:                    return 100; // Unknown systems last
        }
    }
    
    void SystemManager::dispatchSystemEvent(SystemEventType eventType, SystemType systemType) {
        SystemEvent event(eventType, systemType);
        m_eventDispatcher.dispatch(event);
    }
    
    bool SystemManager::areDependenciesInitialized(SystemType systemType) const {
        auto it = m_systems.find(systemType);
        if (it == m_systems.end()) {
            return false;
        }
        
        for (SystemType dependencyType : it->second.dependencies) {
            if (!isSystemInitialized(dependencyType)) {
                return false;
            }
        }
        
        return true;
    }
    
} // namespace RPGEngine