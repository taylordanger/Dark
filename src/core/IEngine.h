#pragma once

#include <memory>
#include <string>

namespace RPGEngine {
    
    // Forward declarations
    class ISystem;
    
    enum class SystemType {
        Rendering,
        Input,
        Physics,
        Audio,
        ECS,
        Resource,
        Scene,
        Combat
    };
    
    /**
     * @brief Main engine interface
     * 
     * The IEngine interface defines the core contract for the game engine,
     * providing methods for initialization, execution, and system management.
     */
    class IEngine {
    public:
        virtual ~IEngine() = default;
        
        /**
         * @brief Initialize the engine and all systems
         * @return true if initialization was successful, false otherwise
         */
        virtual bool initialize() = 0;
        
        /**
         * @brief Run the main game loop
         * 
         * This method starts the main game loop and will continue running
         * until the engine is requested to shutdown.
         */
        virtual void run() = 0;
        
        /**
         * @brief Shutdown the engine and cleanup resources
         * 
         * This method should be called to properly cleanup all systems
         * and resources before the application exits.
         */
        virtual void shutdown() = 0;
        
        /**
         * @brief Get a system by type
         * @param systemType The type of system to retrieve
         * @return Pointer to the system, or nullptr if not found
         */
        virtual std::shared_ptr<ISystem> getSystem(SystemType systemType) = 0;
        
        /**
         * @brief Check if the engine should continue running
         * @return true if the engine should continue, false to exit
         */
        virtual bool isRunning() const = 0;
        
        /**
         * @brief Request the engine to shutdown
         */
        virtual void requestShutdown() = 0;
        
    protected:
        /**
         * @brief Register a system with the engine
         * @param type The system type
         * @param system The system instance
         */
        virtual void registerSystem(SystemType type, std::shared_ptr<ISystem> system) = 0;
    };
    
} // namespace RPGEngine