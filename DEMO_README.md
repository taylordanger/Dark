# RPG Engine Comprehensive Demo Game

This demo showcases ALL major features of the 2D RPG Game Engine in a single, playable example.

## Features Demonstrated

### 🎮 Core Engine Features
- **Entity-Component-System Architecture** - Flexible game object management
- **Cross-Platform Support** - Runs on Windows, macOS, Linux, and Web
- **Performance Optimization** - Memory pooling, frustum culling, multi-threading
- **Configuration System** - JSON-based game settings

### 🎨 Graphics & Rendering
- **OpenGL Rendering** - Hardware-accelerated 2D graphics
- **Sprite Rendering** - Batch rendering with texture management
- **Animation System** - Frame-based sprite animations
- **Tilemap Rendering** - Efficient tile-based world rendering
- **Camera System** - Smooth camera movement and following
- **Debug Rendering** - Visual debugging tools

### Audio System
- **Music Manager** - Background music with looping
- **Sound Effects** - 3D positioned sound effects
- **Audio Resource Management** - Efficient audio loading and caching

### Input Handling
- **Multi-Device Support** - Keyboard, mouse, and gamepad input
- **Configurable Controls** - Customizable key bindings
- **Input Events** - Event-driven input processing

### World & Physics
- **Tilemap Loading** - TMX (Tiled) format support
- **Collision Detection** - AABB collision with spatial partitioning
- **Movement System** - Physics-based character movement
- **Trigger Zones** - Event-based area interactions
- **World Management** - Multiple map support with transitions

### RPG Systems
- **Character Stats** - HP, MP, level, experience progression
- **Inventory System** - Item management with capacity limits
- **Dialogue System** - Branching conversation trees
- **Quest System** - Objective tracking and completion
- **Combat System** - Turn-based combat mechanics
- **Save/Load System** - Game state persistence

### User Interface
- **Immediate Mode GUI** - Flexible UI rendering
- **Game HUD** - Health, mana, and status displays
- **Menu System** - Main menu, pause menu, settings
- **Dialogue UI** - Interactive conversation interface
- **Quest UI** - Quest tracking and progress display
- **Combat UI** - Turn-based combat interface

### 🎬 Scene Management
- **Scene System** - Game state management
- **Scene Transitions** - Smooth transitions between game states
- **Multiple Scene Types** - Game, menu, and battle scenes

### Development Tools
- **Debug Systems** - Performance profiler, entity inspector
- **Content Creation Tools** - Map editor, animation editor, dialogue editor
- **Scripting Support** - Lua integration for game logic
- **Hot Reloading** - Development workflow optimization

## Building the Demo

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler
- OpenGL support
- Lua development libraries
- GLFW3 (for desktop platforms)

### Build Instructions

1. **Create build directory:**
   ```bash
   mkdir build-demo
   cd build-demo
   ```

2. **Configure with CMake:**
   ```bash
   cmake ..
   ```

3. **Build the demo:**
   ```bash
   make RPGDemoGame -j4
   ```

4. **Run the demo:**
   ```bash
   ./RPGDemoGame
   ```

### Cross-Platform Building

#### Windows (Visual Studio)
```bash
cmake .. -G "Visual Studio 16 2019"
cmake --build . --target RPGDemoGame --config Release
```

#### macOS
```bash
cmake ..
make RPGDemoGame -j4
```

#### Linux
```bash
cmake ..
make RPGDemoGame -j4
```

#### Web (Emscripten)
```bash
emcmake cmake ..
emmake make RPGDemoGame
```

## Controls

### Movement
- **WASD** or **Arrow Keys** - Move player character
- **Mouse** - UI interaction

### Interaction
- **SPACE** - Interact with NPCs, items, or attack enemies
- **E** - Open/close inventory
- **Q** - Open/close quest log
- **ESC** - Pause menu / Scene switching

### Debug & Development
- **TAB** - Toggle debug information display
- **F5** - Quick save
- **F9** - Quick load

## Demo Gameplay

### 1. Village Exploration
- Start in a peaceful village with NPCs and buildings
- Explore the tilemap-based world
- Interact with the village elder to receive quests

### 2. Character Progression
- View character stats (HP, MP, Level, Experience)
- Manage inventory with item pickup and usage
- Level up through combat and quest completion

### 3. Quest System
- Accept quests from NPCs
- Track quest objectives in the quest log
- Complete objectives to earn rewards

### 4. Combat System
- Encounter enemies in the world
- Engage in turn-based combat
- Use items and abilities strategically

### 5. Dialogue System
- Engage in conversations with NPCs
- Make choices that affect quest outcomes
- Experience branching dialogue trees

### 6. Save/Load System
- Save game progress at any time
- Load previous saves to continue gameplay
- Automatic save system for progress preservation

## Technical Architecture

### Entity-Component-System
The demo uses a pure ECS architecture where:
- **Entities** are unique identifiers
- **Components** store data (Stats, Inventory, Physics, etc.)
- **Systems** process entities with specific component combinations

### Rendering Pipeline
1. **World Rendering** - Tilemaps and background elements
2. **Entity Rendering** - Characters, NPCs, items, enemies
3. **UI Rendering** - HUD, menus, dialogue boxes
4. **Debug Rendering** - Collision shapes, performance info

### System Update Order
1. **Input System** - Process user input
2. **Script System** - Execute game logic scripts
3. **Movement System** - Update entity positions
4. **Physics System** - Handle collisions and triggers
5. **Combat System** - Process combat interactions
6. **Quest System** - Update quest progress
7. **Audio System** - Update sound effects and music
8. **Camera System** - Update camera position
9. **Animation System** - Update sprite animations

## Asset Requirements

### Graphics Assets
- Character sprites (32x32 pixels recommended)
- Animation frames for idle and walking
- Tileset for world rendering
- UI elements and icons

### Audio Assets
- Background music (OGG format recommended)
- Sound effects (WAV format recommended)
- Ambient sounds for atmosphere

### Data Assets
- Dialogue trees (JSON format)
- Quest definitions (JSON format)
- Map files (TMX format from Tiled editor)
- Configuration files (JSON format)

## Performance Features

### Optimization Techniques
- **Batch Rendering** - Minimize draw calls
- **Frustum Culling** - Only render visible objects
- **Spatial Partitioning** - Efficient collision detection
- **Memory Pooling** - Reduce memory allocations
- **Multi-threading** - Parallel system updates

### Debug Information
When debug mode is enabled (TAB key), you can see:
- Frame rate (FPS)
- Frame time in milliseconds
- Entity count
- Memory usage
- System performance metrics

## Extending the Demo

The demo is designed to be easily extensible:

1. **Add New Components** - Create new data types for entities
2. **Add New Systems** - Implement new game logic processors
3. **Add New Scenes** - Create different game states
4. **Add New Assets** - Include graphics, audio, and data files
5. **Modify Configuration** - Adjust game settings via JSON files

## Troubleshooting

### Common Issues

1. **Graphics not displaying:**
   - Ensure OpenGL drivers are installed
   - Check that GLFW is properly linked

2. **Audio not playing:**
   - Verify audio files exist in assets/audio/
   - Check system audio settings

3. **Build errors:**
   - Ensure all dependencies are installed
   - Check CMake configuration output

4. **Performance issues:**
   - Enable debug mode to check performance metrics
   - Reduce window size or disable debug rendering

### Debug Mode
Enable debug mode with TAB to see:
- Collision shapes
- Entity information
- Performance statistics
- System status

## License

This demo is part of the RPG Engine project and demonstrates the capabilities of a modern 2D game engine built with C++17 and OpenGL.