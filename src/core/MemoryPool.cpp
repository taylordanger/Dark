#include "MemoryPool.h"
#include <cstdlib>
#include <cstring>

namespace RPGEngine {
namespace Core {

// PODMemoryPool implementation
template<typename T>
PODMemoryPool<T>::PODMemoryPool(size_t initialSize, size_t growthSize)
    : m_initialSize(initialSize)
    , m_growthSize(growthSize)
    , m_totalSize(0)
{
    if (initialSize > 0) {
        grow();
    }
}

template<typename T>
PODMemoryPool<T>::~PODMemoryPool() {
    clear();
}

template<typename T>
T* PODMemoryPool<T>::acquire() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_freeList.empty()) {
        grow();
    }
    
    if (m_freeList.empty()) {
        return nullptr; // Failed to grow
    }
    
    T* obj = m_freeList.back();
    m_freeList.pop_back();
    
    // Zero out the memory for POD types
    std::memset(obj, 0, sizeof(T));
    
    return obj;
}

template<typename T>
void PODMemoryPool<T>::release(T* obj) {
    if (!obj) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_freeList.push_back(obj);
}

template<typename T>
void PODMemoryPool<T>::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (void* block : m_blocks) {
        std::free(block);
    }
    
    m_blocks.clear();
    m_freeList.clear();
    m_totalSize = 0;
}

template<typename T>
void PODMemoryPool<T>::grow() {
    size_t growSize = m_blocks.empty() ? m_initialSize : m_growthSize;
    size_t blockSize = growSize * sizeof(T);
    
    void* block = std::malloc(blockSize);
    if (!block) {
        return; // Failed to allocate
    }
    
    T* typedBlock = static_cast<T*>(block);
    
    // Add all objects in the block to the free list
    for (size_t i = 0; i < growSize; ++i) {
        m_freeList.push_back(&typedBlock[i]);
    }
    
    m_totalSize += growSize;
    m_blocks.push_back(block);
}

// MemoryPoolManager implementation
MemoryPoolManager& MemoryPoolManager::getInstance() {
    static MemoryPoolManager instance;
    return instance;
}

MemoryPoolManager::PoolStats MemoryPoolManager::getStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    PoolStats stats = {};
    // This would be implemented with actual pool tracking
    // For now, return empty stats
    return stats;
}

void MemoryPoolManager::clearAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // This would clear all registered pools
}

// Explicit template instantiations for common types
template class MemoryPool<float>;
template class MemoryPool<int>;
template class MemoryPool<char>;

template class PODMemoryPool<float>;
template class PODMemoryPool<int>;
template class PODMemoryPool<char>;

} // namespace Core
} // namespace RPGEngine