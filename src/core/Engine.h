#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace RPGEngine {
    namespace Core {
        
        // Forward declarations
        class ISystem;
        enum class SystemType;
        
        /**
         * Core engine interface defining the main engine contract
         */
        class IEngine {
        public:
            virtual ~IEngine() = default;
            
            virtual bool initialize() = 0;
            virtual void run() = 0;
            virtual void shutdown() = 0;
            
            template<typename T>
            T* getSystem();
            
        protected:
            virtual void registerSystem(SystemType type, std::unique_ptr<ISystem> system) = 0;
        };
        
        /**
         * Base system interface that all engine systems must implement
         */
        class ISystem {
        public:
            virtual ~ISystem() = default;
            
            virtual bool initialize() = 0;
            virtual void update(float deltaTime) = 0;
            virtual void shutdown() = 0;
            virtual SystemType getType() const = 0;
        };
        
        /**
         * System type enumeration for system identification
         */
        enum class SystemType {
            Rendering,
            Input,
            Physics,
            Audio,
            ECS,
            Resource,
            Scene,
            Animation,
            UI,
            Script
        };
        
    } // namespace Core
} // namespace RPGEngine