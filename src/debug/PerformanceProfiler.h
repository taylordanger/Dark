#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine {
    namespace Debug {

        struct FrameStats {
            float frameTime;        // Time for this frame in milliseconds
            float fps;             // Frames per second
            size_t memoryUsage;    // Memory usage in bytes
            size_t entityCount;    // Number of active entities
            size_t drawCalls;      // Number of draw calls this frame
        };

        struct ProfilerSection {
            std::string name;
            std::chrono::high_resolution_clock::time_point startTime;
            float totalTime;       // Total time spent in this section (ms)
            size_t callCount;      // Number of times this section was called
            bool active;           // Whether this section is currently being timed
        };

        class PerformanceProfiler {
        public:
            PerformanceProfiler();
            ~PerformanceProfiler();

            // Frame timing
            void beginFrame();
            void endFrame();
            
            // Section profiling
            void beginSection(const std::string& name);
            void endSection(const std::string& name);
            
            // Automatic section profiling with RAII
            class SectionTimer {
            public:
                SectionTimer(PerformanceProfiler* profiler, const std::string& name);
                ~SectionTimer();
            private:
                PerformanceProfiler* m_profiler;
                std::string m_name;
            };

            // Statistics
            FrameStats getCurrentFrameStats() const;
            float getAverageFPS() const;
            float getAverageFrameTime() const;
            size_t getPeakMemoryUsage() const;
            
            // Section statistics
            std::vector<ProfilerSection> getAllSections() const;
            ProfilerSection getSectionStats(const std::string& name) const;
            
            // Memory tracking
            void recordMemoryUsage(size_t bytes);
            void recordEntityCount(size_t count);
            void recordDrawCalls(size_t count);
            
            // Configuration
            void setMaxFrameHistory(size_t maxFrames) { m_maxFrameHistory = maxFrames; }
            void setEnabled(bool enabled) { m_enabled = enabled; }
            bool isEnabled() const { return m_enabled; }
            
            // Reset statistics
            void reset();
            void resetSection(const std::string& name);
            
            // Update (call once per frame)
            void update();
            
            // UI rendering
            void renderProfilerUI();

        private:
            bool m_enabled;
            size_t m_maxFrameHistory;
            
            // Frame timing
            std::chrono::high_resolution_clock::time_point m_frameStartTime;
            std::vector<FrameStats> m_frameHistory;
            size_t m_frameCount;
            
            // Section profiling
            std::unordered_map<std::string, ProfilerSection> m_sections;
            
            // Memory tracking
            size_t m_currentMemoryUsage;
            size_t m_peakMemoryUsage;
            size_t m_currentEntityCount;
            size_t m_currentDrawCalls;
            
            // Helper methods
            float calculateFPS(float frameTime) const;
            void updateFrameHistory(const FrameStats& stats);
            size_t getCurrentMemoryUsage() const;
        };

        // Macro for easy section profiling
        #define PROFILE_SECTION(profiler, name) \
            Engine::Debug::PerformanceProfiler::SectionTimer _timer(profiler, name)

    } // namespace Debug
} // namespace Engine