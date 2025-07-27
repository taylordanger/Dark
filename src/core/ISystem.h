#pragma once

#include <string>

namespace RPGEngine {
    
    /**
     * Base interface for all engine systems
     */
    class ISystem {
    public:
        virtual ~ISystem() = default;
        
        /**
         * Initialize the system
         * @return true if initialization was successful
         */
        virtual bool initialize() = 0;
        
        /**
         * Update the system with delta time
         * @param deltaTime Time elapsed since last update in seconds
         */
        virtual void update(float deltaTime) = 0;
        
        /**
         * Shutdown the system and cleanup resources
         */
        virtual void shutdown() = 0;
        
        /**
         * Get the system name for debugging
         * @return System name as string
         */
        virtual const std::string& getName() const = 0;
        
        /**
         * Check if the system is initialized
         * @return true if system is ready to use
         */
        virtual bool isInitialized() const = 0;
    };
    
} // namespace RPGEngine