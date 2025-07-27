#pragma once

#include "IEngine.h"
#include "ISystem.h"
#include "Event.h"
#include "EngineConfig.h"
#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace RPGEngine {
    
    /**
     * Engine state enumeration
     */
    enum class EngineState {
        Uninitialized,
        Initializing,
        Running,
        Paused,
        Shutting_Down,
        Shutdown
    };
    
    /**
     * Main engine implementation managing the game loop and systems
     */
    class EngineCore : public IEngine {
    public:
        EngineCore();
        explicit EngineCore(const EngineConfig& config);
        virtual ~EngineCore();
        
        // IEngine implementation
        bool initialize() override;
        void run() override;
        void shutdown() override;
        std::shared_ptr<ISystem> getSystem(SystemType systemType) override;
        
        // Engine state management
        bool isRunning() const override { return m_state == EngineState::Running; }
        void requestShutdown() override;
        void pause();
        void resume();
        bool isPaused() const { return m_state == EngineState::Paused; }
        EngineState getState() const { return m_state; }
        
        // Frame timing and performance
        float getDeltaTime() const { return m_deltaTime; }
        float getFPS() const { return m_fps; }
        float getAverageFPS() const { return m_averageFPS; }
        uint64_t getFrameCount() const { return m_totalFrameCount; }
        float getRunTime() const;
        
        // Configuration
        const EngineConfig& getConfig() const { return m_config; }
        void setConfig(const EngineConfig& config);
        bool loadConfig(const std::string& filename);
        bool saveConfig(const std::string& filename) const;
        
        // Event system
        EventDispatcher& getEventDispatcher() { return m_eventDispatcher; }
        
        // System management
        template<typename T>
        std::shared_ptr<T> getSystemAs(SystemType systemType);
        
        // Public system registration for testing/setup
        void addSystem(SystemType type, std::shared_ptr<ISystem> system);
        
        // System dependency management
        void setSystemDependency(SystemType dependent, SystemType dependency);
        
    protected:
        void registerSystem(SystemType type, std::shared_ptr<ISystem> system) override;
        
    private:
        void gameLoop();
        void updateSystems(float deltaTime);
        void updateFixedSystems(float fixedDeltaTime);
        void updateVariableSystems(float deltaTime);
        void calculateFrameTiming();
        void initializeDefaultSystems();
        void updateSystemOrder();
        void handleFrameRateLimit();
        void updatePerformanceStats();
        
        // Engine state
        EngineState m_state;
        mutable std::mutex m_stateMutex;
        
        // Configuration
        EngineConfig m_config;
        
        // Event system
        EventDispatcher m_eventDispatcher;
        
        // Frame timing
        float m_deltaTime;
        float m_fps;
        float m_averageFPS;
        uint64_t m_totalFrameCount;
        int m_frameCount;
        
        std::chrono::high_resolution_clock::time_point m_startTime;
        std::chrono::high_resolution_clock::time_point m_lastFrameTime;
        std::chrono::high_resolution_clock::time_point m_fpsUpdateTime;
        std::chrono::high_resolution_clock::time_point m_pauseStartTime;
        std::chrono::duration<float> m_totalPausedTime;
        
        // Performance tracking
        std::vector<float> m_frameTimeHistory;
        static constexpr size_t MAX_FRAME_HISTORY = 60;
        
        // System management
        std::unordered_map<SystemType, std::shared_ptr<ISystem>> m_systems;
        std::vector<SystemType> m_systemUpdateOrder;
        std::unordered_map<SystemType, std::vector<SystemType>> m_systemDependencies;
    };
    
    // Template implementation
    template<typename T>
    std::shared_ptr<T> EngineCore::getSystemAs(SystemType systemType) {
        auto it = m_systems.find(systemType);
        if (it != m_systems.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    
} // namespace RPGEngine