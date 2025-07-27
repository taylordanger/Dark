#pragma once

#include "Resource.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <future>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

namespace RPGEngine {
namespace Resources {

/**
 * Resource loading callback function type
 */
using ResourceCallback = std::function<void(std::shared_ptr<Resource>)>;

/**
 * Resource manager
 * Manages resource loading, caching, and reference counting
 */
class ResourceManager {
public:
    /**
     * Constructor
     * @param asyncLoadingEnabled Whether to enable asynchronous loading
     * @param maxAsyncLoads Maximum number of concurrent asynchronous loads
     */
    ResourceManager(bool asyncLoadingEnabled = true, int maxAsyncLoads = 4);
    
    /**
     * Destructor
     */
    ~ResourceManager();
    
    /**
     * Initialize the resource manager
     * @return true if initialization was successful
     */
    bool initialize();
    
    /**
     * Shutdown the resource manager
     */
    void shutdown();
    
    /**
     * Update the resource manager
     * Processes completed async loads and invokes callbacks
     */
    void update();
    
    /**
     * Get a resource by ID
     * @param id Resource ID
     * @return Resource, or nullptr if not found
     */
    std::shared_ptr<Resource> getResource(const std::string& id) const;
    
    /**
     * Check if a resource exists
     * @param id Resource ID
     * @return true if the resource exists
     */
    bool hasResource(const std::string& id) const;
    
    /**
     * Add a resource
     * @param resource Resource to add
     * @return true if the resource was added
     */
    bool addResource(std::shared_ptr<Resource> resource);
    
    /**
     * Remove a resource
     * @param id Resource ID
     * @return true if the resource was removed
     */
    bool removeResource(const std::string& id);
    
    /**
     * Load a resource
     * @param id Resource ID
     * @param callback Callback function to call when the resource is loaded
     * @return true if the resource was found and loading was initiated
     */
    bool loadResource(const std::string& id, ResourceCallback callback = nullptr);
    
    /**
     * Load a resource asynchronously
     * @param id Resource ID
     * @param callback Callback function to call when the resource is loaded
     * @return true if the resource was found and loading was initiated
     */
    bool loadResourceAsync(const std::string& id, ResourceCallback callback = nullptr);
    
    /**
     * Unload a resource
     * @param id Resource ID
     * @return true if the resource was found and unloaded
     */
    bool unloadResource(const std::string& id);
    
    /**
     * Reload a resource
     * @param id Resource ID
     * @param callback Callback function to call when the resource is reloaded
     * @return true if the resource was found and reloading was initiated
     */
    bool reloadResource(const std::string& id, ResourceCallback callback = nullptr);
    
    /**
     * Reload a resource asynchronously
     * @param id Resource ID
     * @param callback Callback function to call when the resource is reloaded
     * @return true if the resource was found and reloading was initiated
     */
    bool reloadResourceAsync(const std::string& id, ResourceCallback callback = nullptr);
    
    /**
     * Get all resources
     * @return Vector of all resources
     */
    std::vector<std::shared_ptr<Resource>> getAllResources() const;
    
    /**
     * Get all resources of a specific type
     * @tparam T Resource type
     * @return Vector of resources of the specified type
     */
    template<typename T>
    std::vector<std::shared_ptr<T>> getResourcesOfType() const {
        std::vector<std::shared_ptr<T>> result;
        
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        for (const auto& pair : m_resources) {
            auto resource = std::dynamic_pointer_cast<T>(pair.second);
            if (resource) {
                result.push_back(resource);
            }
        }
        
        return result;
    }
    
    /**
     * Get a resource of a specific type
     * @tparam T Resource type
     * @param id Resource ID
     * @return Resource of the specified type, or nullptr if not found or wrong type
     */
    template<typename T>
    std::shared_ptr<T> getResourceOfType(const std::string& id) const {
        auto resource = getResource(id);
        if (resource) {
            return std::dynamic_pointer_cast<T>(resource);
        }
        return nullptr;
    }
    
    /**
     * Set the base path for resources
     * @param basePath Base path
     */
    void setBasePath(const std::string& basePath) { m_basePath = basePath; }
    
    /**
     * Get the base path for resources
     * @return Base path
     */
    const std::string& getBasePath() const { return m_basePath; }
    
    /**
     * Set whether asynchronous loading is enabled
     * @param enabled Whether asynchronous loading is enabled
     */
    void setAsyncLoadingEnabled(bool enabled);
    
    /**
     * Check if asynchronous loading is enabled
     * @return true if asynchronous loading is enabled
     */
    bool isAsyncLoadingEnabled() const { return m_asyncLoadingEnabled; }
    
    /**
     * Set the maximum number of concurrent asynchronous loads
     * @param maxLoads Maximum number of concurrent asynchronous loads
     */
    void setMaxAsyncLoads(int maxLoads);
    
    /**
     * Get the maximum number of concurrent asynchronous loads
     * @return Maximum number of concurrent asynchronous loads
     */
    int getMaxAsyncLoads() const { return m_maxAsyncLoads; }
    
    /**
     * Get the number of resources
     * @return Number of resources
     */
    size_t getResourceCount() const;
    
    /**
     * Get the number of loaded resources
     * @return Number of loaded resources
     */
    size_t getLoadedResourceCount() const;
    
    /**
     * Get the number of resources currently loading
     * @return Number of resources currently loading
     */
    size_t getLoadingResourceCount() const;
    
    /**
     * Get the number of resources that failed to load
     * @return Number of resources that failed to load
     */
    size_t getFailedResourceCount() const;
    
    /**
     * Clear all resources
     */
    void clearResources();
    
    /**
     * Clear unused resources (reference count = 0)
     * @return Number of resources cleared
     */
    size_t clearUnusedResources();
    
private:
    /**
     * Async load task structure
     */
    struct AsyncLoadTask {
        std::string resourceId;
        ResourceCallback callback;
        bool isReload;
        
        AsyncLoadTask(const std::string& id, ResourceCallback cb, bool reload = false)
            : resourceId(id), callback(cb), isReload(reload) {}
    };
    
    /**
     * Async load result structure
     */
    struct AsyncLoadResult {
        std::string resourceId;
        bool success;
        ResourceCallback callback;
        
        AsyncLoadResult(const std::string& id, bool success, ResourceCallback cb)
            : resourceId(id), success(success), callback(cb) {}
    };
    
    /**
     * Worker thread function
     */
    void workerThreadFunc();
    
    /**
     * Process completed async loads
     */
    void processCompletedAsyncLoads();
    
    /**
     * Queue an async load task
     * @param task Async load task
     */
    void queueAsyncLoadTask(const AsyncLoadTask& task);
    
    // Resources
    mutable std::mutex m_resourceMutex;
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;
    
    // Async loading
    bool m_asyncLoadingEnabled;
    int m_maxAsyncLoads;
    std::atomic<bool> m_shutdownThreads;
    std::vector<std::thread> m_workerThreads;
    std::mutex m_taskQueueMutex;
    std::condition_variable m_taskQueueCondition;
    std::queue<AsyncLoadTask> m_taskQueue;
    std::mutex m_resultQueueMutex;
    std::queue<AsyncLoadResult> m_resultQueue;
    std::atomic<int> m_activeAsyncLoads;
    
    // Base path
    std::string m_basePath;
};

} // namespace Resources
} // namespace RPGEngine