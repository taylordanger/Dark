#include "PerformanceProfiler.h"
#include <algorithm>
#include <numeric>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif __APPLE__
#include <mach/mach.h>
#elif __linux__
#include <sys/resource.h>
#include <fstream>
#endif

namespace Engine {
    namespace Debug {

        PerformanceProfiler::PerformanceProfiler()
            : m_enabled(true)
            , m_maxFrameHistory(60)
            , m_frameCount(0)
            , m_currentMemoryUsage(0)
            , m_peakMemoryUsage(0)
            , m_currentEntityCount(0)
            , m_currentDrawCalls(0) {
            
            m_frameHistory.reserve(m_maxFrameHistory);
        }

        PerformanceProfiler::~PerformanceProfiler() {
        }

        void PerformanceProfiler::beginFrame() {
            if (!m_enabled) return;
            
            m_frameStartTime = std::chrono::high_resolution_clock::now();
            m_currentDrawCalls = 0; // Reset draw call counter
        }

        void PerformanceProfiler::endFrame() {
            if (!m_enabled) return;
            
            auto frameEndTime = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - m_frameStartTime);
            float frameTimeMs = frameDuration.count() / 1000.0f;
            
            FrameStats stats;
            stats.frameTime = frameTimeMs;
            stats.fps = calculateFPS(frameTimeMs);
            stats.memoryUsage = getCurrentMemoryUsage();
            stats.entityCount = m_currentEntityCount;
            stats.drawCalls = m_currentDrawCalls;
            
            updateFrameHistory(stats);
            m_frameCount++;
            
            // Update peak memory usage
            if (stats.memoryUsage > m_peakMemoryUsage) {
                m_peakMemoryUsage = stats.memoryUsage;
            }
        }

        void PerformanceProfiler::beginSection(const std::string& name) {
            if (!m_enabled) return;
            
            auto& section = m_sections[name];
            section.name = name;
            section.startTime = std::chrono::high_resolution_clock::now();
            section.active = true;
        }

        void PerformanceProfiler::endSection(const std::string& name) {
            if (!m_enabled) return;
            
            auto it = m_sections.find(name);
            if (it != m_sections.end() && it->second.active) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second.startTime);
                float timeMs = duration.count() / 1000.0f;
                
                it->second.totalTime += timeMs;
                it->second.callCount++;
                it->second.active = false;
            }
        }

        PerformanceProfiler::SectionTimer::SectionTimer(PerformanceProfiler* profiler, const std::string& name)
            : m_profiler(profiler), m_name(name) {
            if (m_profiler) {
                m_profiler->beginSection(m_name);
            }
        }

        PerformanceProfiler::SectionTimer::~SectionTimer() {
            if (m_profiler) {
                m_profiler->endSection(m_name);
            }
        }

        FrameStats PerformanceProfiler::getCurrentFrameStats() const {
            if (m_frameHistory.empty()) {
                return {0.0f, 0.0f, 0, 0, 0};
            }
            return m_frameHistory.back();
        }

        float PerformanceProfiler::getAverageFPS() const {
            if (m_frameHistory.empty()) return 0.0f;
            
            float totalFPS = 0.0f;
            for (const auto& frame : m_frameHistory) {
                totalFPS += frame.fps;
            }
            return totalFPS / m_frameHistory.size();
        }

        float PerformanceProfiler::getAverageFrameTime() const {
            if (m_frameHistory.empty()) return 0.0f;
            
            float totalTime = 0.0f;
            for (const auto& frame : m_frameHistory) {
                totalTime += frame.frameTime;
            }
            return totalTime / m_frameHistory.size();
        }

        size_t PerformanceProfiler::getPeakMemoryUsage() const {
            return m_peakMemoryUsage;
        }

        std::vector<ProfilerSection> PerformanceProfiler::getAllSections() const {
            std::vector<ProfilerSection> sections;
            for (const auto& pair : m_sections) {
                sections.push_back(pair.second);
            }
            return sections;
        }

        ProfilerSection PerformanceProfiler::getSectionStats(const std::string& name) const {
            auto it = m_sections.find(name);
            if (it != m_sections.end()) {
                return it->second;
            }
            return {name, {}, 0.0f, 0, false};
        }

        void PerformanceProfiler::recordMemoryUsage(size_t bytes) {
            m_currentMemoryUsage = bytes;
        }

        void PerformanceProfiler::recordEntityCount(size_t count) {
            m_currentEntityCount = count;
        }

        void PerformanceProfiler::recordDrawCalls(size_t count) {
            m_currentDrawCalls += count;
        }

        void PerformanceProfiler::reset() {
            m_frameHistory.clear();
            m_sections.clear();
            m_frameCount = 0;
            m_peakMemoryUsage = 0;
        }

        void PerformanceProfiler::resetSection(const std::string& name) {
            auto it = m_sections.find(name);
            if (it != m_sections.end()) {
                it->second.totalTime = 0.0f;
                it->second.callCount = 0;
            }
        }

        void PerformanceProfiler::update() {
            // Update current memory usage
            m_currentMemoryUsage = getCurrentMemoryUsage();
        }

        void PerformanceProfiler::renderProfilerUI() {
            // This would integrate with the UI system to render profiler information
            // Could show:
            // - Current FPS and frame time
            // - Memory usage graph
            // - Section timing breakdown
            // - Entity count over time
            // - Draw call statistics
        }

        float PerformanceProfiler::calculateFPS(float frameTime) const {
            if (frameTime <= 0.0f) return 0.0f;
            return 1000.0f / frameTime;
        }

        void PerformanceProfiler::updateFrameHistory(const FrameStats& stats) {
            if (m_frameHistory.size() >= m_maxFrameHistory) {
                m_frameHistory.erase(m_frameHistory.begin());
            }
            m_frameHistory.push_back(stats);
        }

        size_t PerformanceProfiler::getCurrentMemoryUsage() const {
#ifdef _WIN32
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                return pmc.WorkingSetSize;
            }
#elif __APPLE__
            struct mach_task_basic_info info;
            mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
            if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
                return info.resident_size;
            }
#elif __linux__
            std::ifstream file("/proc/self/status");
            std::string line;
            while (std::getline(file, line)) {
                if (line.substr(0, 6) == "VmRSS:") {
                    size_t kb = std::stoul(line.substr(7));
                    return kb * 1024; // Convert KB to bytes
                }
            }
#endif
            return m_currentMemoryUsage; // Fallback to recorded value
        }

    } // namespace Debug
} // namespace Engine