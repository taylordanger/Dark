#pragma once

#include <string>

namespace RPGEngine {
    
    /**
     * Engine configuration settings
     */
    struct EngineConfig {
        // Frame timing settings
        float targetFPS = 60.0f;
        bool vsyncEnabled = true;
        bool frameRateLimitEnabled = true;
        float fixedTimeStep = 1.0f / 60.0f; // For physics and other fixed-rate updates
        
        // Window settings (for future use)
        int windowWidth = 1280;
        int windowHeight = 720;
        std::string windowTitle = "RPG Engine";
        bool fullscreen = false;
        bool borderless = false;
        bool resizable = true;
        
        // Engine settings
        bool debugMode = false;
        bool showFPS = false;
        int maxFrameSkip = 5;
        bool pauseOnFocusLoss = false;
        
        // System settings
        bool enableMultithreading = false;
        int workerThreadCount = 0; // 0 = auto-detect
        bool useFixedTimeStep = true; // Use fixed time step for physics
        
        // Rendering settings
        bool enableVSync = true;
        int msaaSamples = 0; // 0 = disabled, 2/4/8 = enabled with sample count
        bool enablePostProcessing = false;
        
        // Audio settings
        float masterVolume = 1.0f;
        float musicVolume = 1.0f;
        float sfxVolume = 1.0f;
        int audioChannels = 32;
        int audioSampleRate = 44100;
        
        // Logging settings
        bool enableLogging = true;
        std::string logLevel = "INFO"; // DEBUG, INFO, WARN, ERROR
        std::string logFile = "engine.log";
        bool logToConsole = true;
        
        /**
         * Load configuration from file
         * @param filename Configuration file path
         * @return true if loaded successfully
         */
        bool loadFromFile(const std::string& filename);
        
        /**
         * Save configuration to file
         * @param filename Configuration file path
         * @return true if saved successfully
         */
        bool saveToFile(const std::string& filename) const;
        
        /**
         * Reset to default values
         */
        void resetToDefaults();
        
        /**
         * Validate configuration values
         * @return true if all values are valid
         */
        bool validate() const;
    };
    
} // namespace RPGEngine