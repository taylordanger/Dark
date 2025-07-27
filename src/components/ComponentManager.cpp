#include "ComponentManager.h"
#include <iostream>

namespace RPGEngine {

ComponentManager::ComponentManager()
    : m_initialized(false)
{
}

ComponentManager::~ComponentManager() {
    if (m_initialized) {
        shutdown();
    }
}

bool ComponentManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Clear any existing data
    m_components.clear();
    
    m_initialized = true;
    std::cout << "ComponentManager initialized" << std::endl;
    return true;
}

void ComponentManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Clear all components
    clearAllComponents();
    
    m_initialized = false;
    std::cout << "ComponentManager shutdown" << std::endl;
}

void ComponentManager::removeAllComponents(Entity entity) {
    if (!m_initialized || !entity.isValid()) {
        return;
    }
    
    // Remove all components for this entity
    for (auto& pair : m_components) {
        pair.second.erase(entity.getID());
    }
    
    // Remove empty component type maps
    auto it = m_components.begin();
    while (it != m_components.end()) {
        if (it->second.empty()) {
            it = m_components.erase(it);
        } else {
            ++it;
        }
    }
}

void ComponentManager::clearAllComponents() {
    if (!m_initialized) {
        return;
    }
    
    m_components.clear();
}

} // namespace RPGEngine