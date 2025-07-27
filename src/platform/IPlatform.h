#pragma once

#include <string>

namespace RPGEngine {
    
    /**
     * Platform abstraction interface for OS-specific functionality
     */
    class IPlatform {
    public:
        virtual ~IPlatform() = default;
        
        /**
         * Initialize platform-specific systems
         * @return true if initialization was successful
         */
        virtual bool initialize() = 0;
        
        /**
         * Shutdown platform-specific systems
         */
        virtual void shutdown() = 0;
        
        /**
         * Get the platform name
         * @return Platform name as string
         */
        virtual const std::string& getPlatformName() const = 0;
        
        /**
         * Get the current working directory
         * @return Current working directory path
         */
        virtual std::string getCurrentDirectory() const = 0;
        
        /**
         * Check if a file exists
         * @param filepath Path to the file
         * @return true if file exists
         */
        virtual bool fileExists(const std::string& filepath) const = 0;
        
        /**
         * Get the time in milliseconds since platform initialization
         * @return Time in milliseconds
         */
        virtual uint64_t getTimeMilliseconds() const = 0;
        
        /**
         * Sleep for the specified number of milliseconds
         * @param milliseconds Time to sleep
         */
        virtual void sleep(uint32_t milliseconds) const = 0;
    };
    
    /**
     * Get the platform instance (singleton)
     * @return Pointer to platform instance
     */
    IPlatform* getPlatform();
    
} // namespace RPGEngine