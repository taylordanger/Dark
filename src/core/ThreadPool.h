#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <type_traits>

namespace RPGEngine {
namespace Core {

/**
 * Thread pool for parallel execution of tasks
 * Allows systems to be updated in parallel where appropriate
 */
class ThreadPool {
public:
    /**
     * Constructor
     * @param numThreads Number of worker threads (0 = auto-detect)
     */
    explicit ThreadPool(size_t numThreads = 0);
    
    /**
     * Destructor
     */
    ~ThreadPool();
    
    /**
     * Submit a task to the thread pool
     * @param task Function to execute
     * @param args Arguments to pass to the function
     * @return Future for the result
     */
    template<typename F, typename... Args>
    auto submit(F&& task, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;
    
    /**
     * Submit multiple tasks and wait for all to complete
     * @param tasks Vector of tasks to execute
     */
    void submitAndWait(const std::vector<std::function<void()>>& tasks);
    
    /**
     * Get the number of worker threads
     * @return Number of threads
     */
    size_t getThreadCount() const { return m_threads.size(); }
    
    /**
     * Get the number of pending tasks
     * @return Number of tasks in queue
     */
    size_t getPendingTaskCount() const;
    
    /**
     * Check if the thread pool is busy
     * @return true if there are active or pending tasks
     */
    bool isBusy() const;
    
    /**
     * Wait for all current tasks to complete
     */
    void waitForAll();
    
private:
    /**
     * Worker thread function
     */
    void workerThread();
    
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    std::condition_variable m_finished;
    std::atomic<bool> m_stop;
    std::atomic<size_t> m_activeTasks;
};

/**
 * Parallel system updater
 * Manages parallel execution of system updates
 */
class ParallelSystemUpdater {
public:
    /**
     * Constructor
     * @param threadPool Thread pool to use for parallel execution
     */
    explicit ParallelSystemUpdater(ThreadPool& threadPool);
    
    /**
     * Update systems in parallel where possible
     * @param systems Vector of systems to update
     * @param deltaTime Delta time for updates
     */
    template<typename SystemType>
    void updateSystemsParallel(const std::vector<SystemType*>& systems, float deltaTime);
    
    /**
     * Update systems with dependencies in correct order
     * @param systems Vector of systems with their dependencies
     * @param deltaTime Delta time for updates
     */
    template<typename SystemType>
    void updateSystemsWithDependencies(const std::vector<std::pair<SystemType*, std::vector<SystemType*>>>& systems, float deltaTime);
    
private:
    ThreadPool& m_threadPool;
};

// Template implementations

template<typename F, typename... Args>
auto ThreadPool::submit(F&& task, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using ReturnType = typename std::invoke_result<F, Args...>::type;
    
    auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(task), std::forward<Args>(args)...)
    );
    
    std::future<ReturnType> result = taskPtr->get_future();
    
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        
        if (m_stop) {
            throw std::runtime_error("Cannot submit task to stopped thread pool");
        }
        
        m_tasks.emplace([taskPtr]() { (*taskPtr)(); });
    }
    
    m_condition.notify_one();
    return result;
}

template<typename SystemType>
void ParallelSystemUpdater::updateSystemsParallel(const std::vector<SystemType*>& systems, float deltaTime) {
    std::vector<std::future<void>> futures;
    futures.reserve(systems.size());
    
    for (SystemType* system : systems) {
        if (system && system->isInitialized()) {
            futures.push_back(m_threadPool.submit([system, deltaTime]() {
                system->update(deltaTime);
            }));
        }
    }
    
    // Wait for all systems to complete
    for (auto& future : futures) {
        future.wait();
    }
}

template<typename SystemType>
void ParallelSystemUpdater::updateSystemsWithDependencies(
    const std::vector<std::pair<SystemType*, std::vector<SystemType*>>>& systems, 
    float deltaTime) {
    
    // This is a simplified implementation
    // In a full implementation, we would build a dependency graph
    // and execute systems in parallel when their dependencies are satisfied
    
    std::vector<std::future<void>> futures;
    
    for (const auto& systemPair : systems) {
        SystemType* system = systemPair.first;
        const std::vector<SystemType*>& dependencies = systemPair.second;
        
        if (system && system->isInitialized()) {
            // For now, just execute sequentially
            // TODO: Implement proper dependency-aware parallel execution
            futures.push_back(m_threadPool.submit([system, deltaTime]() {
                system->update(deltaTime);
            }));
        }
    }
    
    // Wait for all systems to complete
    for (auto& future : futures) {
        future.wait();
    }
}

} // namespace Core
} // namespace RPGEngine