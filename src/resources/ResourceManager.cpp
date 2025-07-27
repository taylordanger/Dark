#include "ResourceManager.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace RPGEngine {
namespace Resources {

ResourceManager::ResourceManager(bool asyncLoadingEnabled, int maxAsyncLoads)
    : m_asyncLoadingEnabled(asyncLoadingEnabled)
    , m_maxAsyncLoads(maxAsyncLoads)
    , m_shutdownThreads(false)
    , m_activeAsyncLoads(0)
    , m_basePath("")
{
}

ResourceManager::~ResourceManager() {
    if (m_asyncLoadingEnabled) {
        shutdown();
    }
    
    clearResources();
}

bool ResourceManager::initialize() {
    if (m_asyncLoadingEnabled) {
        // Create worker threads
        m_shutdownThreads = false;
        
        for (int i = 0; i < m_maxAsyncLoads; ++i) {
            m_workerThreads.emplace_back(&ResourceManager::workerThreadFunc, this);
        }
    }
    
    std::cout << "ResourceManager initialized" << std::endl;
    return true;
}

void ResourceManager::shutdown() {
    if (m_asyncLoadingEnabled) {
        // Signal worker threads to shut down
        m_shutdownThreads = true;
        m_taskQueueCondition.notify_all();
        
        // Wait for worker threads to finish
        for (auto& thread : m_workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        m_workerThreads.clear();
    }
    
    std::cout << "ResourceManager shutdown" << std::endl;
}

void ResourceManager::update() {
    if (m_asyncLoadingEnabled) {
        // Process completed async loads
        processCompletedAsyncLoads();
    }
}

std::shared_ptr<Resource> ResourceManager::getResource(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool ResourceManager::hasResource(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    return m_resources.find(id) != m_resources.end();
}

bool ResourceManager::addResource(std::shared_ptr<Resource> resource) {
    if (!resource) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    const std::string& id = resource->getId();
    
    // Check if resource already exists
    if (m_resources.find(id) != m_resources.end()) {
        return false;
    }
    
    // Add resource
    m_resources[id] = resource;
    
    return true;
}

bool ResourceManager::removeResource(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
        // Unload resource if loaded
        if (it->second->isLoaded()) {
            it->second->unload();
        }
        
        // Remove resource
        m_resources.erase(it);
        
        return true;
    }
    
    return false;
}

bool ResourceManager::loadResource(const std::string& id, ResourceCallback callback) {
    auto resource = getResource(id);
    if (!resource) {
        return false;
    }
    
    // Check if already loaded
    if (resource->isLoaded()) {
        if (callback) {
            callback(resource);
        }
        return true;
    }
    
    // Load resource
    bool success = resource->load();
    
    // Call callback
    if (callback) {
        callback(resource);
    }
    
    return success;
}

bool ResourceManager::loadResourceAsync(const std::string& id, ResourceCallback callback) {
    if (!m_asyncLoadingEnabled) {
        return loadResource(id, callback);
    }
    
    auto resource = getResource(id);
    if (!resource) {
        return false;
    }
    
    // Check if already loaded
    if (resource->isLoaded()) {
        if (callback) {
            callback(resource);
        }
        return true;
    }
    
    // Check if already loading
    if (resource->isLoading()) {
        if (callback) {
            // Queue callback to be called when loading is complete
            queueAsyncLoadTask(AsyncLoadTask(id, callback));
        }
        return true;
    }
    
    // Queue async load task
    queueAsyncLoadTask(AsyncLoadTask(id, callback));
    
    return true;
}

bool ResourceManager::unloadResource(const std::string& id) {
    auto resource = getResource(id);
    if (!resource) {
        return false;
    }
    
    // Check if loaded
    if (!resource->isLoaded()) {
        return true;
    }
    
    // Unload resource
    resource->unload();
    
    return true;
}

bool ResourceManager::reloadResource(const std::string& id, ResourceCallback callback) {
    auto resource = getResource(id);
    if (!resource) {
        return false;
    }
    
    // Reload resource
    bool success = resource->reload();
    
    // Call callback
    if (callback) {
        callback(resource);
    }
    
    return success;
}

bool ResourceManager::reloadResourceAsync(const std::string& id, ResourceCallback callback) {
    if (!m_asyncLoadingEnabled) {
        return reloadResource(id, callback);
    }
    
    auto resource = getResource(id);
    if (!resource) {
        return false;
    }
    
    // Queue async reload task
    queueAsyncLoadTask(AsyncLoadTask(id, callback, true));
    
    return true;
}

std::vector<std::shared_ptr<Resource>> ResourceManager::getAllResources() const {
    std::vector<std::shared_ptr<Resource>> result;
    
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    result.reserve(m_resources.size());
    
    for (const auto& pair : m_resources) {
        result.push_back(pair.second);
    }
    
    return result;
}

void ResourceManager::setAsyncLoadingEnabled(bool enabled) {
    if (m_asyncLoadingEnabled == enabled) {
        return;
    }
    
    if (m_asyncLoadingEnabled) {
        // Shutting down async loading
        shutdown();
    }
    
    m_asyncLoadingEnabled = enabled;
    
    if (m_asyncLoadingEnabled) {
        // Starting up async loading
        initialize();
    }
}

void ResourceManager::setMaxAsyncLoads(int maxLoads) {
    if (maxLoads <= 0) {
        maxLoads = 1;
    }
    
    if (m_maxAsyncLoads == maxLoads) {
        return;
    }
    
    if (m_asyncLoadingEnabled) {
        // Restart with new thread count
        shutdown();
        m_maxAsyncLoads = maxLoads;
        initialize();
    } else {
        m_maxAsyncLoads = maxLoads;
    }
}

size_t ResourceManager::getResourceCount() const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    return m_resources.size();
}

size_t ResourceManager::getLoadedResourceCount() const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    size_t count = 0;
    for (const auto& pair : m_resources) {
        if (pair.second->isLoaded()) {
            ++count;
        }
    }
    
    return count;
}

size_t ResourceManager::getLoadingResourceCount() const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    size_t count = 0;
    for (const auto& pair : m_resources) {
        if (pair.second->isLoading()) {
            ++count;
        }
    }
    
    return count;
}

size_t ResourceManager::getFailedResourceCount() const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    size_t count = 0;
    for (const auto& pair : m_resources) {
        if (pair.second->isFailed()) {
            ++count;
        }
    }
    
    return count;
}

void ResourceManager::clearResources() {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    // Unload all resources
    for (auto& pair : m_resources) {
        if (pair.second->isLoaded()) {
            pair.second->unload();
        }
    }
    
    // Clear resources
    m_resources.clear();
}

size_t ResourceManager::clearUnusedResources() {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    size_t count = 0;
    auto it = m_resources.begin();
    while (it != m_resources.end()) {
        if (it->second->getRefCount() <= 0) {
            // Unload resource if loaded
            if (it->second->isLoaded()) {
                it->second->unload();
            }
            
            // Remove resource
            it = m_resources.erase(it);
            ++count;
        } else {
            ++it;
        }
    }
    
    return count;
}

void ResourceManager::workerThreadFunc() {
    while (!m_shutdownThreads) {
        AsyncLoadTask task("", nullptr);
        
        // Wait for a task
        {
            std::unique_lock<std::mutex> lock(m_taskQueueMutex);
            m_taskQueueCondition.wait(lock, [this] {
                return m_shutdownThreads || !m_taskQueue.empty();
            });
            
            if (m_shutdownThreads) {
                break;
            }
            
            if (m_taskQueue.empty()) {
                continue;
            }
            
            task = m_taskQueue.front();
            m_taskQueue.pop();
            
            // Increment active async loads counter
            ++m_activeAsyncLoads;
        }
        
        // Get the resource
        auto resource = getResource(task.resourceId);
        if (!resource) {
            // Resource not found, decrement counter and continue
            --m_activeAsyncLoads;
            continue;
        }
        
        // Load or reload the resource
        bool success = task.isReload ? resource->reload() : resource->load();
        
        // Queue the result
        {
            std::lock_guard<std::mutex> lock(m_resultQueueMutex);
            m_resultQueue.emplace(task.resourceId, success, task.callback);
        }
        
        // Decrement active async loads counter
        --m_activeAsyncLoads;
    }
}

void ResourceManager::processCompletedAsyncLoads() {
    std::queue<AsyncLoadResult> results;
    
    // Get all completed async loads
    {
        std::lock_guard<std::mutex> lock(m_resultQueueMutex);
        std::swap(results, m_resultQueue);
    }
    
    // Process completed async loads
    while (!results.empty()) {
        const auto& result = results.front();
        
        // Get the resource
        auto resource = getResource(result.resourceId);
        if (resource && result.callback) {
            // Call the callback
            result.callback(resource);
        }
        
        results.pop();
    }
}

void ResourceManager::queueAsyncLoadTask(const AsyncLoadTask& task) {
    std::lock_guard<std::mutex> lock(m_taskQueueMutex);
    m_taskQueue.push(task);
    m_taskQueueCondition.notify_one();
}

} // namespace Resources
} // namespace RPGEngine