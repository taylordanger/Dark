# Dark Engine Framework

A modular, cross-platform 2D RPG game engine built with modern C++ and designed for extensibility and performance.

## Project Structure

```
├── src/                    # Engine source code
│   ├── core/              # Core engine interfaces and types
│   │   ├── IEngine.h      # Main engine interface
│   │   ├── ISystem.h      # Base system interface
│   │   └── Types.h        # Common type definitions
│   ├── systems/           # Engine systems
│   │   ├── SystemManager.h # System lifecycle management
│   │   └── SystemManager.cpp
│   ├── components/        # ECS component definitions
│   │   └── IComponent.h   # Base component interface
│   └── platform/          # Platform abstraction layer
│       └── IPlatform.h    # Platform interface
├── examples/              # Example applications
│   └── main.cpp          # Basic engine test
├── CMakeLists.txt        # Build configuration
└── README.md            # This file
```

## Features

### Core Architecture
- **Modular Design**: Each system is self-contained and independently testable
- **Entity-Component-System**: Flexible game object architecture
- **Cross-Platform**: Supports Windows, macOS, and Linux
- **Modern C++17**: Uses modern C++ features for performance and safety

### System Management
- **Priority-Based Updates**: Systems update in optimal order
- **Lifecycle Management**: Proper initialization and shutdown sequences
- **Type-Safe System Retrieval**: Template-based system access
- **Dependency Resolution**: Automatic system dependency handling

## Building

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Run the test example
./RPGEngineTest  # Linux/macOS
# or
RPGEngineTest.exe  # Windows
```

### Build Types
- **Debug**: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- **Release**: `cmake -DCMAKE_BUILD_TYPE=Release ..` (default)

## Usage

### Basic Engine Setup

```cpp
#include "systems/SystemManager.h"

// Create system manager
SystemManager systemManager;

// Register systems
systemManager.registerSystem(std::make_unique<YourSystem>(), SystemType::YourType);

// Initialize all systems
if (!systemManager.initializeAll()) {
    // Handle initialization failure
}

// Game loop
while (running) {
    systemManager.updateAll(deltaTime);
}

// Cleanup
systemManager.shutdownAll();
```

### Creating Custom Systems

```cpp
class YourSystem : public ISystem {
public:
    bool initialize() override {
        // Initialize your system
        return true;
    }
    
    void update(float deltaTime) override {
        // Update logic here
    }
    
    void shutdown() override {
        // Cleanup resources
    }
    
    const std::string& getName() const override {
        static std::string name = "YourSystem";
        return name;
    }
    
    bool isInitialized() const override {
        return m_initialized;
    }
    
private:
    bool m_initialized = false;
};
```

## Architecture Overview

The engine follows a layered architecture:

1. **Application Layer**: Game-specific code
2. **Engine Core**: Main engine loop and system coordination
3. **Systems Layer**: Individual engine systems (rendering, input, physics, etc.)
4. **Platform Abstraction**: OS-specific implementations

### System Update Order
Systems are updated in priority order:
1. Input System (priority 0)
2. ECS System (priority 1)
3. Physics System (priority 2)
4. Audio System (priority 3)
5. Rendering System (priority 4)
6. Resource System (priority 5)
7. Scene System (priority 6)

## Development Status

This is the initial foundation implementation (Task 1) that includes:
- Project structure and directory organization
- Core interfaces (IEngine, ISystem, IComponent, IPlatform)
- System management with priority-based updates
- Cross-platform CMake build system
- Basic type definitions and common structures
- Example application for testing

## Next Steps

The following systems will be implemented in subsequent tasks:
- Core engine with game loop (Task 2)
- Entity-Component-System (Task 3)
- Rendering system (Task 4)
- Input handling (Task 5)
- Physics and collision (Task 6)
- And more...

## License

This project is part of a game engine development specification and is intended for educational and development purposes.