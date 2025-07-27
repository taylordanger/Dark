#pragma once

#include <string>
#include <atomic>
#include <memory>

namespace RPGEngine {
namespace Resources {

/**
 * Resource loading state
 */
enum class ResourceState {
    Unloaded,   // Resource is not loaded
    Loading,    // Resource is currently loading
    Loaded,     // Resource is loaded and ready to use
    Failed      // Resource failed to load
};

/**
 * Base resource class
 * Represents a loadable resource such as a texture, sound, or model
 */
class Resource {
public:
    /**
     * Constructor
     * @param id Resource ID
     * @param path Resource path
     */
    Resource(const std::string& id, const std::string& path)
        : m_id(id)
        , m_path(path)
        , m_state(ResourceState::Unloaded)
        , m_refCount(0)
    {}
    
    /**
     * Virtual destructor
     */
    virtual ~Resource() = default;
    
    /**
     * Get the resource ID
     * @return Resource ID
     */
    const std::string& getId() const { return m_id; }
    
    /**
     * Get the resource path
     * @return Resource path
     */
    const std::string& getPath() const { return m_path; }
    
    /**
     * Get the resource state
     * @return Resource state
     */
    ResourceState getState() const { return m_state; }
    
    /**
     * Check if the resource is loaded
     * @return true if the resource is loaded
     */
    bool isLoaded() const { return m_state == ResourceState::Loaded; }
    
    /**
     * Check if the resource is loading
     * @return true if the resource is loading
     */
    bool isLoading() const { return m_state == ResourceState::Loading; }
    
    /**
     * Check if the resource failed to load
     * @return true if the resource failed to load
     */
    bool isFailed() const { return m_state == ResourceState::Failed; }
    
    /**
     * Get the reference count
     * @return Reference count
     */
    int getRefCount() const { return m_refCount; }
    
    /**
     * Increment the reference count
     * @return New reference count
     */
    int addReference() { return ++m_refCount; }
    
    /**
     * Decrement the reference count
     * @return New reference count
     */
    int removeReference() { return --m_refCount; }
    
    /**
     * Load the resource
     * @return true if the resource was loaded successfully
     */
    virtual bool load() = 0;
    
    /**
     * Unload the resource
     */
    virtual void unload() = 0;
    
    /**
     * Reload the resource
     * @return true if the resource was reloaded successfully
     */
    virtual bool reload() {
        unload();
        return load();
    }
    
protected:
    /**
     * Set the resource state
     * @param state New resource state
     */
    void setState(ResourceState state) { m_state = state; }
    
private:
    std::string m_id;                // Resource ID
    std::string m_path;              // Resource path
    ResourceState m_state;           // Resource state
    std::atomic<int> m_refCount;     // Reference count
};

} // namespace Resources
} // namespace RPGEngine