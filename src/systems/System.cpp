#include "System.h"
#include "SystemManager.h"
#include <iostream>

namespace RPGEngine {
    
    System::System(const std::string& name)
        : m_name(name)
        , m_initialized(false)
        , m_enabled(true)
        , m_useFixedTimestep(false)
        , m_priority(0)
        , m_systemManager(nullptr)
    {
    }
    
    bool System::initialize() {
        if (m_initialized) {
            return true;
        }
        
        // Check if all dependencies are initialized
        for (SystemType dependencyType : m_dependencies) {
            if (m_systemManager && !m_systemManager->isSystemInitialized(dependencyType)) {
                std::cerr << "System '" << m_name << "' depends on uninitialized system: " 
                          << static_cast<int>(dependencyType) << std::endl;
                return false;
            }
        }
        
        // Call system-specific initialization
        if (!onInitialize()) {
            std::cerr << "System '" << m_name << "' failed to initialize" << std::endl;
            return false;
        }
        
        m_initialized = true;
        return true;
    }
    
    void System::update(float deltaTime) {
        if (!m_initialized || !m_enabled) {
            return;
        }
        
        // Call system-specific update
        onUpdate(deltaTime);
    }
    
    void System::shutdown() {
        if (!m_initialized) {
            return;
        }
        
        // Call system-specific shutdown
        onShutdown();
        
        m_initialized = false;
    }
    
    void System::addDependency(SystemType systemType) {
        m_dependencies.insert(systemType);
    }
    
    bool System::hasDependency(SystemType systemType) const {
        return m_dependencies.find(systemType) != m_dependencies.end();
    }
    
    template<typename T>
    T* System::getSystem(SystemType systemType) const {
        if (m_systemManager) {
            return m_systemManager->getSystem<T>(systemType);
        }
        return nullptr;
    }
    
} // namespace RPGEngine