#include "EngineConfig.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace RPGEngine {
    
    bool EngineConfig::loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // Parse key=value pairs
            size_t equalPos = line.find('=');
            if (equalPos == std::string::npos) {
                continue;
            }
            
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Set configuration values
            if (key == "targetFPS") {
                targetFPS = std::stof(value);
            } else if (key == "vsyncEnabled") {
                vsyncEnabled = (value == "true" || value == "1");
            } else if (key == "frameRateLimitEnabled") {
                frameRateLimitEnabled = (value == "true" || value == "1");
            } else if (key == "fixedTimeStep") {
                fixedTimeStep = std::stof(value);
            } else if (key == "windowWidth") {
                windowWidth = std::stoi(value);
            } else if (key == "windowHeight") {
                windowHeight = std::stoi(value);
            } else if (key == "windowTitle") {
                windowTitle = value;
            } else if (key == "fullscreen") {
                fullscreen = (value == "true" || value == "1");
            } else if (key == "borderless") {
                borderless = (value == "true" || value == "1");
            } else if (key == "resizable") {
                resizable = (value == "true" || value == "1");
            } else if (key == "debugMode") {
                debugMode = (value == "true" || value == "1");
            } else if (key == "showFPS") {
                showFPS = (value == "true" || value == "1");
            } else if (key == "maxFrameSkip") {
                maxFrameSkip = std::stoi(value);
            } else if (key == "pauseOnFocusLoss") {
                pauseOnFocusLoss = (value == "true" || value == "1");
            } else if (key == "enableMultithreading") {
                enableMultithreading = (value == "true" || value == "1");
            } else if (key == "workerThreadCount") {
                workerThreadCount = std::stoi(value);
            } else if (key == "useFixedTimeStep") {
                useFixedTimeStep = (value == "true" || value == "1");
            } else if (key == "enableVSync") {
                enableVSync = (value == "true" || value == "1");
            } else if (key == "msaaSamples") {
                msaaSamples = std::stoi(value);
            } else if (key == "enablePostProcessing") {
                enablePostProcessing = (value == "true" || value == "1");
            } else if (key == "masterVolume") {
                masterVolume = std::stof(value);
            } else if (key == "musicVolume") {
                musicVolume = std::stof(value);
            } else if (key == "sfxVolume") {
                sfxVolume = std::stof(value);
            } else if (key == "audioChannels") {
                audioChannels = std::stoi(value);
            } else if (key == "audioSampleRate") {
                audioSampleRate = std::stoi(value);
            } else if (key == "enableLogging") {
                enableLogging = (value == "true" || value == "1");
            } else if (key == "logLevel") {
                logLevel = value;
            } else if (key == "logFile") {
                logFile = value;
            } else if (key == "logToConsole") {
                logToConsole = (value == "true" || value == "1");
            }
        }
        
        return validate();
    }
    
    bool EngineConfig::saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to create config file: " << filename << std::endl;
            return false;
        }
        
        file << "# RPG Engine Configuration File\n";
        file << "# Frame timing settings\n";
        file << "targetFPS=" << targetFPS << "\n";
        file << "vsyncEnabled=" << (vsyncEnabled ? "true" : "false") << "\n";
        file << "frameRateLimitEnabled=" << (frameRateLimitEnabled ? "true" : "false") << "\n";
        file << "fixedTimeStep=" << fixedTimeStep << "\n";
        file << "\n";
        
        file << "# Window settings\n";
        file << "windowWidth=" << windowWidth << "\n";
        file << "windowHeight=" << windowHeight << "\n";
        file << "windowTitle=" << windowTitle << "\n";
        file << "fullscreen=" << (fullscreen ? "true" : "false") << "\n";
        file << "borderless=" << (borderless ? "true" : "false") << "\n";
        file << "resizable=" << (resizable ? "true" : "false") << "\n";
        file << "\n";
        
        file << "# Engine settings\n";
        file << "debugMode=" << (debugMode ? "true" : "false") << "\n";
        file << "showFPS=" << (showFPS ? "true" : "false") << "\n";
        file << "maxFrameSkip=" << maxFrameSkip << "\n";
        file << "pauseOnFocusLoss=" << (pauseOnFocusLoss ? "true" : "false") << "\n";
        file << "\n";
        
        file << "# System settings\n";
        file << "enableMultithreading=" << (enableMultithreading ? "true" : "false") << "\n";
        file << "workerThreadCount=" << workerThreadCount << "\n";
        file << "useFixedTimeStep=" << (useFixedTimeStep ? "true" : "false") << "\n";
        file << "\n";
        
        file << "# Rendering settings\n";
        file << "enableVSync=" << (enableVSync ? "true" : "false") << "\n";
        file << "msaaSamples=" << msaaSamples << "\n";
        file << "enablePostProcessing=" << (enablePostProcessing ? "true" : "false") << "\n";
        file << "\n";
        
        file << "# Audio settings\n";
        file << "masterVolume=" << masterVolume << "\n";
        file << "musicVolume=" << musicVolume << "\n";
        file << "sfxVolume=" << sfxVolume << "\n";
        file << "audioChannels=" << audioChannels << "\n";
        file << "audioSampleRate=" << audioSampleRate << "\n";
        file << "\n";
        
        file << "# Logging settings\n";
        file << "enableLogging=" << (enableLogging ? "true" : "false") << "\n";
        file << "logLevel=" << logLevel << "\n";
        file << "logFile=" << logFile << "\n";
        file << "logToConsole=" << (logToConsole ? "true" : "false") << "\n";
        
        return true;
    }
    
    void EngineConfig::resetToDefaults() {
        *this = EngineConfig{};
    }
    
    bool EngineConfig::validate() const {
        if (targetFPS <= 0.0f || targetFPS > 1000.0f) {
            std::cerr << "Invalid targetFPS: " << targetFPS << std::endl;
            return false;
        }
        
        if (fixedTimeStep <= 0.0f || fixedTimeStep > 1.0f) {
            std::cerr << "Invalid fixedTimeStep: " << fixedTimeStep << std::endl;
            return false;
        }
        
        if (windowWidth <= 0 || windowHeight <= 0) {
            std::cerr << "Invalid window dimensions: " << windowWidth << "x" << windowHeight << std::endl;
            return false;
        }
        
        if (maxFrameSkip < 0) {
            std::cerr << "Invalid maxFrameSkip: " << maxFrameSkip << std::endl;
            return false;
        }
        
        if (workerThreadCount < 0) {
            std::cerr << "Invalid workerThreadCount: " << workerThreadCount << std::endl;
            return false;
        }
        
        if (msaaSamples != 0 && msaaSamples != 2 && msaaSamples != 4 && msaaSamples != 8 && msaaSamples != 16) {
            std::cerr << "Invalid msaaSamples: " << msaaSamples << std::endl;
            return false;
        }
        
        if (masterVolume < 0.0f || masterVolume > 1.0f) {
            std::cerr << "Invalid masterVolume: " << masterVolume << std::endl;
            return false;
        }
        
        if (musicVolume < 0.0f || musicVolume > 1.0f) {
            std::cerr << "Invalid musicVolume: " << musicVolume << std::endl;
            return false;
        }
        
        if (sfxVolume < 0.0f || sfxVolume > 1.0f) {
            std::cerr << "Invalid sfxVolume: " << sfxVolume << std::endl;
            return false;
        }
        
        if (audioChannels <= 0 || audioChannels > 256) {
            std::cerr << "Invalid audioChannels: " << audioChannels << std::endl;
            return false;
        }
        
        if (audioSampleRate != 22050 && audioSampleRate != 44100 && audioSampleRate != 48000 && audioSampleRate != 96000) {
            std::cerr << "Invalid audioSampleRate: " << audioSampleRate << std::endl;
            return false;
        }
        
        return true;
    }
    
} // namespace RPGEngine