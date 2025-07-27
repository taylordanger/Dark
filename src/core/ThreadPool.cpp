#include "ThreadPool.h"
#include <algorithm>

namespace RPGEngine {
namespace Core {

ThreadPool::ThreadPool(size_t numThreads)
    : m_stop(false)
    , m_activeTasks(0)
{
    if (numThreads == 0) {
        numThreads = std::max(1u, std::thread::hardware_concurrency());
    }
    
    m_threads.reserve(numThreads);
    
    for (size_t i = 0; i < numThreads; ++i) {
        m_threads.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    
    m_condition.notify_all();
    
    for (std::thread& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadPool::submitAndWait(const std::vector<std::function<void()>>& tasks) {
    std::vector<std::future<void>> futures;
    futures.reserve(tasks.size());
    
    for (const auto& task : tasks) {
        futures.push_back(submit(task));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
}

size_t ThreadPool::getPendingTaskCount() const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    return m_tasks.size();
}

bool ThreadPool::isBusy() const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    return !m_tasks.empty() || m_activeTasks > 0;
}

void ThreadPool::waitForAll() {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_finished.wait(lock, [this] {
        return m_tasks.empty() && m_activeTasks == 0;
    });
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            m_condition.wait(lock, [this] {
                return m_stop || !m_tasks.empty();
            });
            
            if (m_stop && m_tasks.empty()) {
                break;
            }
            
            task = std::move(m_tasks.front());
            m_tasks.pop();
            m_activeTasks++;
        }
        
        try {
            task();
        } catch (...) {
            // Log error in a real implementation
        }
        
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_activeTasks--;
            if (m_tasks.empty() && m_activeTasks == 0) {
                m_finished.notify_all();
            }
        }
    }
}

// ParallelSystemUpdater implementation
ParallelSystemUpdater::ParallelSystemUpdater(ThreadPool& threadPool)
    : m_threadPool(threadPool)
{
}

} // namespace Core
} // namespace RPGEngine