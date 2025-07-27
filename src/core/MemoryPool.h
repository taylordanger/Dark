#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <mutex>
#include <cstddef>
#include <type_traits>

namespace RPGEngine {
namespace Core {

/**
 * Generic memory pool for efficient allocation and deallocation
 * Reduces memory fragmentation and allocation overhead
 */
template<typename T>
class MemoryPool {
public:
    /**
     * Constructor
     * @param initialSize Initial number of objects to pre-allocate
     * @param growthSize Number of objects to allocate when pool is exhausted
     */
    explicit MemoryPool(size_t initialSize = 64, size_t growthSize = 32);
    
    /**
     * Destructor
     */
    ~MemoryPool();
    
    /**
     * Acquire an object from the pool
     * @return Pointer to an available object
     */
    T* acquire();
    
    /**
     * Release an object back to the pool
     * @param obj Pointer to the object to release
     */
    void release(T* obj);
    
    /**
     * Get the total number of objects in the pool
     * @return Total pool size
     */
    size_t getTotalSize() const { return m_totalSize; }
    
    /**
     * Get the number of available objects in the pool
     * @return Available objects count
     */
    size_t getAvailableCount() const { return m_available.size(); }
    
    /**
     * Get the number of objects currently in use
     * @return Objects in use count
     */
    size_t getUsedCount() const { return m_totalSize - m_available.size(); }
    
    /**
     * Clear the pool and deallocate all memory
     */
    void clear();
    
private:
    /**
     * Grow the pool by allocating more objects
     */
    void grow();
    
    std::vector<std::unique_ptr<T[]>> m_blocks;
    std::stack<T*> m_available;
    size_t m_initialSize;
    size_t m_growthSize;
    size_t m_totalSize;
    mutable std::mutex m_mutex;
};

/**
 * Specialized memory pool for POD types with better performance
 */
template<typename T>
class PODMemoryPool {
    static_assert(std::is_pod<T>::value, "PODMemoryPool can only be used with POD types");
    
public:
    explicit PODMemoryPool(size_t initialSize = 64, size_t growthSize = 32);
    ~PODMemoryPool();
    
    T* acquire();
    void release(T* obj);
    
    size_t getTotalSize() const { return m_totalSize; }
    size_t getAvailableCount() const { return m_freeList.size(); }
    size_t getUsedCount() const { return m_totalSize - m_freeList.size(); }
    
    void clear();
    
private:
    void grow();
    
    std::vector<void*> m_blocks;
    std::vector<T*> m_freeList;
    size_t m_initialSize;
    size_t m_growthSize;
    size_t m_totalSize;
    mutable std::mutex m_mutex;
};

/**
 * Global memory pool manager for common types
 */
class MemoryPoolManager {
public:
    static MemoryPoolManager& getInstance();
    
    /**
     * Get a memory pool for a specific type
     * @return Reference to the memory pool
     */
    template<typename T>
    MemoryPool<T>& getPool();
    
    /**
     * Get statistics for all pools
     */
    struct PoolStats {
        size_t totalAllocated;
        size_t totalUsed;
        size_t totalAvailable;
        size_t poolCount;
    };
    
    PoolStats getStats() const;
    
    /**
     * Clear all pools
     */
    void clearAll();
    
private:
    MemoryPoolManager() = default;
    ~MemoryPoolManager() = default;
    
    // Non-copyable
    MemoryPoolManager(const MemoryPoolManager&) = delete;
    MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
    
    mutable std::mutex m_mutex;
    // Pool storage will be implemented using type erasure for different types
};

// Template implementations

template<typename T>
MemoryPool<T>::MemoryPool(size_t initialSize, size_t growthSize)
    : m_initialSize(initialSize)
    , m_growthSize(growthSize)
    , m_totalSize(0)
{
    if (initialSize > 0) {
        grow();
    }
}

template<typename T>
MemoryPool<T>::~MemoryPool() {
    clear();
}

template<typename T>
T* MemoryPool<T>::acquire() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_available.empty()) {
        grow();
    }
    
    if (m_available.empty()) {
        return nullptr; // Failed to grow
    }
    
    T* obj = m_available.top();
    m_available.pop();
    
    // Call constructor for non-POD types
    if (!std::is_pod<T>::value) {
        new(obj) T();
    }
    
    return obj;
}

template<typename T>
void MemoryPool<T>::release(T* obj) {
    if (!obj) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Call destructor for non-POD types
    if (!std::is_pod<T>::value) {
        obj->~T();
    }
    
    m_available.push(obj);
}

template<typename T>
void MemoryPool<T>::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_blocks.clear();
    while (!m_available.empty()) {
        m_available.pop();
    }
    m_totalSize = 0;
}

template<typename T>
void MemoryPool<T>::grow() {
    size_t growSize = m_blocks.empty() ? m_initialSize : m_growthSize;
    
    auto block = std::make_unique<T[]>(growSize);
    T* blockPtr = block.get();
    
    // Add all objects in the block to the available stack
    for (size_t i = 0; i < growSize; ++i) {
        m_available.push(&blockPtr[i]);
    }
    
    m_totalSize += growSize;
    m_blocks.push_back(std::move(block));
}

} // namespace Core
} // namespace RPGEngine