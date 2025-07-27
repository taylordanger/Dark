#include "EngineCore.h"
#include <iostream>
#include <thread>
#include <algorithm>

namespace RPGEngine {
    
    EngineCore::EngineCore() 
        : EngineCore(EngineConfig{})
    {
    }
    
    EngineCore::EngineCore(const EngineConfig& config)
        : m_state(EngineState::Uninitialized)
        , m_config(config)
        , m_deltaTime(0.0f)
        , m_fps(0.0f)
        , m_averageFPS(0.0f)
        , m_totalFrameCount(0)
        , m_frameCount(0)
        , m_totalPausedTime(0.0f)
    {
        // Initialize system update order
        m_systemUpdateOrder = {
            SystemType::Input,
            SystemType::Physics,
            SystemType::ECS,
            SystemType::Audio,
            SystemType::Rendering,
            SystemType::Scene
        };
        
        // Reserve space for frame time history
        m_frameTimeHistory.reserve(MAX_FRAME_HISTORY);
    }
    
    EngineCore::~EngineCore() {
        shutdown();
    }
    
    bool EngineCore::initialize() {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        if (m_state != EngineState::Uninitialized) {
            return m_state == EngineState::Running;
        }
        
        m_state = EngineState::Initializing;
        
        std::cout << "Initializing RPG Engine Core..." << std::endl;
        
        // Validate configuration
        if (!m_config.validate()) {
            std::cerr << "Invalid engine configuration!" << std::endl;
            m_state = EngineState::Uninitialized;
            return false;
        }
        
        // Initialize timing
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastFrameTime = m_startTime;
        m_fpsUpdateTime = m_startTime;
        
        // Initialize default systems if needed
        initializeDefaultSystems();
        
        // Update system order based on dependencies
        updateSystemOrder();
        
        // Initialize all registered systems
        for (SystemType systemType : m_systemUpdateOrder) {
            auto it = m_systems.find(systemType);
            if (it != m_systems.end()) {
                std::cout << "Initializing system: " << it->second->getName() << std::endl;
                if (!it->second->initialize()) {
                    std::cerr << "Failed to initialize system: " << it->second->getName() << std::endl;
                    m_state = EngineState::Uninitialized;
                    return false;
                }
            }
        }
        
        m_state = EngineState::Running;
        std::cout << "Engine initialization complete." << std::endl;
        return true;
    }
    
    void EngineCore::run() {
        if (!initialize()) {
            std::cerr << "Engine initialization failed!" << std::endl;
            return;
        }
        
        std::cout << "Starting game loop..." << std::endl;
        
        gameLoop();
        
        std::cout << "Game loop ended." << std::endl;
    }
    
    void EngineCore::shutdown() {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        if (m_state == EngineState::Shutdown || m_state == EngineState::Uninitialized) {
            return;
        }
        
        m_state = EngineState::Shutting_Down;
        std::cout << "Shutting down engine..." << std::endl;
        
        // Shutdown systems in reverse order
        for (auto it = m_systemUpdateOrder.rbegin(); it != m_systemUpdateOrder.rend(); ++it) {
            auto systemIt = m_systems.find(*it);
            if (systemIt != m_systems.end()) {
                std::cout << "Shutting down system: " << systemIt->second->getName() << std::endl;
                systemIt->second->shutdown();
            }
        }
        
        // Clear event system
        m_eventDispatcher.clear();
        
        m_systems.clear();
        m_state = EngineState::Shutdown;
        std::cout << "Engine shutdown complete." << std::endl;
    }
    
    std::shared_ptr<ISystem> EngineCore::getSystem(SystemType systemType) {
        auto it = m_systems.find(systemType);
        if (it != m_systems.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    void EngineCore::requestShutdown() {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        if (m_state == EngineState::Running) {
            m_state = EngineState::Shutting_Down;
        }
    }
    
    void EngineCore::pause() {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        if (m_state == EngineState::Running) {
            m_state = EngineState::Paused;
            m_pauseStartTime = std::chrono::high_resolution_clock::now();
        }
    }
    
    void EngineCore::resume() {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        if (m_state == EngineState::Paused) {
            auto pauseEndTime = std::chrono::high_resolution_clock::now();
            m_totalPausedTime += pauseEndTime - m_pauseStartTime;
            m_state = EngineState::Running;
        }
    }
    
    float EngineCore::getRunTime() const {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - m_startTime);
        return totalTime.count() - m_totalPausedTime.count();
    }
    
    void EngineCore::setConfig(const EngineConfig& config) {
        if (config.validate()) {
            m_config = config;
        }
    }
    
    bool EngineCore::loadConfig(const std::string& filename) {
        EngineConfig newConfig;
        if (newConfig.loadFromFile(filename)) {
            m_config = newConfig;
            return true;
        }
        return false;
    }
    
    bool EngineCore::saveConfig(const std::string& filename) const {
        return m_config.saveToFile(filename);
    }
    
    void EngineCore::addSystem(SystemType type, std::shared_ptr<ISystem> system) {
        registerSystem(type, system);
    }
    
    void EngineCore::setSystemDependency(SystemType dependent, SystemType dependency) {
        m_systemDependencies[dependent].push_back(dependency);
        updateSystemOrder();
    }
    
    void EngineCore::registerSystem(SystemType type, std::shared_ptr<ISystem> system) {
        if (system) {
            m_systems[type] = system;
            std::cout << "Registered system: " << static_cast<int>(type) << std::endl;
        }
    }
    
    void EngineCore::gameLoop() {
        // Variables for fixed timestep
        float accumulator = 0.0f;
        const float fixedTimeStep = m_config.fixedTimeStep;
        
        while (m_state == EngineState::Running || m_state == EngineState::Paused) {
            // Handle pause state
            if (m_state == EngineState::Paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }
            
            calculateFrameTiming();
            
            // Process queued events
            m_eventDispatcher.processQueuedEvents();
            
            if (m_config.useFixedTimeStep) {
                // Fixed timestep update for physics and other systems that need consistent timing
                accumulator += m_deltaTime;
                
                // Prevent spiral of death by capping the accumulator
                if (accumulator > 0.2f) {
                    accumulator = 0.2f;
                }
                
                // Update physics at fixed intervals
                while (accumulator >= fixedTimeStep) {
                    updateFixedSystems(fixedTimeStep);
                    accumulator -= fixedTimeStep;
                }
                
                // Update variable systems with the actual delta time
                updateVariableSystems(m_deltaTime);
            } else {
                // Simple update for all systems with the actual delta time
                updateSystems(m_deltaTime);
            }
            
            // Update performance statistics
            updatePerformanceStats();
            
            // Handle frame rate limiting
            if (m_config.frameRateLimitEnabled) {
                handleFrameRateLimit();
            }
            
            m_totalFrameCount++;
        }
    }
    
    void EngineCore::updateSystems(float deltaTime) {
        // Update systems in the defined order
        for (SystemType systemType : m_systemUpdateOrder) {
            auto it = m_systems.find(systemType);
            if (it != m_systems.end() && it->second->isInitialized()) {
                it->second->update(deltaTime);
            }
        }
    }
    
    void EngineCore::updateFixedSystems(float fixedDeltaTime) {
        // Update physics and other systems that need fixed timestep
        // For now, just update physics system
        auto it = m_systems.find(SystemType::Physics);
        if (it != m_systems.end() && it->second->isInitialized()) {
            it->second->update(fixedDeltaTime);
        }
    }
    
    void EngineCore::updateVariableSystems(float deltaTime) {
        // Update all systems except physics with variable timestep
        for (SystemType systemType : m_systemUpdateOrder) {
            if (systemType != SystemType::Physics) {
                auto it = m_systems.find(systemType);
                if (it != m_systems.end() && it->second->isInitialized()) {
                    it->second->update(deltaTime);
                }
            }
        }
    }
    
    void EngineCore::calculateFrameTiming() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        // Calculate delta time
        auto deltaTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_lastFrameTime);
        m_deltaTime = deltaTimeNs.count() / 1000000000.0f; // Convert to seconds
        m_lastFrameTime = currentTime;
        
        // Calculate FPS (update every second)
        m_frameCount++;
        auto fpsUpdateDelta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_fpsUpdateTime);
        if (fpsUpdateDelta.count() >= 1000) {
            m_fps = m_frameCount / (fpsUpdateDelta.count() / 1000.0f);
            m_frameCount = 0;
            m_fpsUpdateTime = currentTime;
        }
    }
    
    void EngineCore::initializeDefaultSystems() {
        // This will be expanded as we implement more systems
        // For now, just a placeholder for future system initialization
    }
    
    void EngineCore::updateSystemOrder() {
        // Simple topological sort for system dependencies
        // For now, use the default order - this will be enhanced in Task 2.2
        std::vector<SystemType> newOrder;
        
        // Add systems without dependencies first
        for (SystemType type : m_systemUpdateOrder) {
            if (m_systemDependencies.find(type) == m_systemDependencies.end()) {
                newOrder.push_back(type);
            }
        }
        
        // Add systems with dependencies (simplified for now)
        for (SystemType type : m_systemUpdateOrder) {
            if (m_systemDependencies.find(type) != m_systemDependencies.end()) {
                newOrder.push_back(type);
            }
        }
        
        m_systemUpdateOrder = newOrder;
    }
    
    void EngineCore::handleFrameRateLimit() {
        const float TARGET_FRAME_TIME = 1.0f / m_config.targetFPS;
        
        if (m_deltaTime < TARGET_FRAME_TIME) {
            float sleepTime = TARGET_FRAME_TIME - m_deltaTime;
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }
    }
    
    void EngineCore::updatePerformanceStats() {
        // Track frame time history for average FPS calculation
        m_frameTimeHistory.push_back(m_deltaTime);
        if (m_frameTimeHistory.size() > MAX_FRAME_HISTORY) {
            m_frameTimeHistory.erase(m_frameTimeHistory.begin());
        }
        
        // Calculate average FPS
        if (!m_frameTimeHistory.empty()) {
            float totalTime = 0.0f;
            for (float frameTime : m_frameTimeHistory) {
                totalTime += frameTime;
            }
            float avgFrameTime = totalTime / m_frameTimeHistory.size();
            m_averageFPS = (avgFrameTime > 0.0f) ? (1.0f / avgFrameTime) : 0.0f;
        }
    }
    
} // namespace RPGEngine