cmake_minimum_required(VERSION 3.16)

project(RPGEngineQuestTest VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include platform-specific configuration
include(cmake/Platform.cmake)

# Compiler-specific options (moved to Platform.cmake)
if(NOT PLATFORM_WINDOWS AND NOT PLATFORM_MACOS AND NOT PLATFORM_LINUX AND NOT EMSCRIPTEN)
    # Fallback for unknown platforms
    if(MSVC)
        add_compile_options(/W4)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
endif()

# Include directories
include_directories(src)
include_directories(external/include)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Find Lua
pkg_check_modules(LUA REQUIRED lua)
include_directories(${LUA_INCLUDE_DIRS})
link_directories(${LUA_LIBRARY_DIRS})

# Find OpenGL
find_package(OpenGL REQUIRED)

# Platform-specific graphics dependencies
if(NOT EMSCRIPTEN)
    # For native platforms, we need GLFW
    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        # Try to find GLFW via pkg-config
        pkg_check_modules(GLFW glfw3)
        if(NOT GLFW_FOUND)
            message(WARNING "GLFW not found. Graphics functionality will be limited.")
            add_compile_definitions(NO_GRAPHICS=1)
        else()
            include_directories(${GLFW_INCLUDE_DIRS})
            link_directories(${GLFW_LIBRARY_DIRS})
        endif()
    endif()
endif()

# Engine source files
set(ENGINE_SOURCES
    # Core
    src/core/Event.cpp
    src/core/EngineCore.cpp
    src/core/EngineConfig.cpp
    src/core/ConfigurationManager.cpp
    src/core/MemoryPool.cpp
    src/core/ThreadPool.cpp
    
    # Debug
    src/debug/DebugRenderer.cpp
    src/debug/EntityInspector.cpp
    src/debug/PerformanceProfiler.cpp
    
    # Systems
    src/systems/SystemManager.cpp
    src/systems/System.cpp
    src/systems/QuestSystem.cpp
    src/systems/QuestDialogueIntegration.cpp
    src/systems/QuestWorldIntegration.cpp
    src/systems/CombatSystem.cpp
    src/systems/ScriptSystem.cpp
    
    # Entities
    src/entities/EntityManager.cpp
    src/entities/EntityFactory.cpp
    
    # Components
    src/components/ComponentManager.cpp
    src/components/QuestComponent.cpp
    src/components/DialogueComponent.cpp
    src/components/StatsComponent.cpp
    src/components/InventoryComponent.cpp
    src/components/CombatComponent.cpp
    src/components/AnimationComponent.cpp
    src/components/ScriptComponent.cpp
    
    # Graphics
    src/graphics/ShaderManager.cpp
    src/graphics/SpriteRenderer.cpp
    src/graphics/Sprite.cpp
    src/graphics/Texture.cpp
    src/graphics/Camera.cpp
    src/graphics/CameraSystem.cpp
    src/graphics/AnimationSystem.cpp
    src/graphics/FrustumCuller.cpp
    

    
    # Physics
    src/physics/CollisionDetection.cpp
    src/physics/CollisionSystem.cpp
    src/physics/MovementSystem.cpp
    src/physics/TriggerSystem.cpp
    src/physics/SpatialPartitioning.cpp
    
    # Resources
    src/resources/ResourceManager.cpp
    src/resources/TextureResource.cpp
    src/resources/AudioResource.cpp
    
    # Audio
    src/audio/AudioManager.cpp
    src/audio/MusicManager.cpp
    src/audio/SoundEffectManager.cpp
    src/audio/MockAudioDevice.cpp
    
    # Tilemap
    src/tilemap/Tilemap.cpp
    src/tilemap/TileLayer.cpp
    src/tilemap/Tileset.cpp
    src/tilemap/TilemapRenderer.cpp
    src/tilemap/MapLoader.cpp
    
    # World
    src/world/Map.cpp
    src/world/MapObject.cpp
    src/world/WorldManager.cpp
    
    # Scene
    src/scene/Scene.cpp
    src/scene/SceneManager.cpp
    src/scene/GameScene.cpp
    src/scene/MenuScene.cpp
    src/scene/BattleScene.cpp
    
    # Save
    src/save/SaveManager.cpp
    src/save/JsonSaveSerializer.cpp
    src/save/SaveIntegration.cpp
    src/save/SaveLoadManager.cpp
    src/save/SystemStateSerializer.cpp
    
    # UI
    src/ui/UIRenderer.cpp
    src/ui/MainMenuUI.cpp
    src/ui/GameHUD.cpp
    src/ui/DialogueUI.cpp
    src/ui/CombatUI.cpp
    src/ui/QuestUI.cpp
    src/ui/SaveLoadUI.cpp
    
    # Scripting
    src/scripting/LuaScriptEngine.cpp
)

# Add graphics and input sources if graphics are enabled
if(NOT NO_GRAPHICS)
    list(APPEND ENGINE_SOURCES 
        src/graphics/OpenGLAPI.cpp
        src/graphics/GraphicsFactory.cpp
        src/input/InputManager.cpp
        src/input/KeyboardDevice.cpp
        src/input/MouseDevice.cpp
        src/input/GamepadDevice.cpp
    )
endif()

# Create minimal engine library
add_library(RPGEngineMinimal STATIC ${ENGINE_SOURCES})

# Link dependencies to the engine library
target_link_libraries(RPGEngineMinimal ${LUA_LIBRARIES})
target_compile_options(RPGEngineMinimal PRIVATE ${LUA_CFLAGS_OTHER})

# Link graphics dependencies if available
if(NOT EMSCRIPTEN AND NOT NO_GRAPHICS)
    if(glfw3_FOUND)
        target_link_libraries(RPGEngineMinimal glfw)
    elseif(GLFW_FOUND)
        target_link_libraries(RPGEngineMinimal ${GLFW_LIBRARIES})
        target_compile_options(RPGEngineMinimal PRIVATE ${GLFW_CFLAGS_OTHER})
    endif()
    
    target_link_libraries(RPGEngineMinimal ${OPENGL_LIBRARIES})
endif()

# Create simple test executable
add_executable(QuestIntegrationSimpleTest
    examples/quest_integration_simple_test.cpp
)

target_link_libraries(QuestIntegrationSimpleTest RPGEngineMinimal)

# Create combat test executable
add_executable(CombatTest
    examples/combat_test.cpp
)

target_link_libraries(CombatTest RPGEngineMinimal)

# Create combat integration test executable
add_executable(CombatIntegrationTest
    examples/combat_integration_test.cpp
)

target_link_libraries(CombatIntegrationTest RPGEngineMinimal)

# Create save system test executable
add_executable(SaveSystemTest
    examples/save_system_test.cpp
)

target_link_libraries(SaveSystemTest RPGEngineMinimal)

# Create save system demo executable
add_executable(SaveSystemDemo
    examples/save_system_demo.cpp
)

target_link_libraries(SaveSystemDemo RPGEngineMinimal)

# Create save/load integration test executable
add_executable(SaveLoadIntegrationTest
    examples/save_load_integration_test.cpp
)

target_link_libraries(SaveLoadIntegrationTest RPGEngineMinimal)

# Create UI framework simple test executable (header-only test)
add_executable(UIFrameworkSimpleTest
    examples/ui_framework_simple_test.cpp
)

# This test only needs the UI headers, not the full engine
target_include_directories(UIFrameworkSimpleTest PRIVATE src)

# Create game-specific UI test executable (header-only test)
add_executable(UIGameSpecificTest
    examples/ui_game_specific_test.cpp
)

# This test only needs the UI headers, not the full engine
target_include_directories(UIGameSpecificTest PRIVATE src)

# Create scripting test executable
add_executable(ScriptingTest
    examples/scripting_test.cpp
)

target_link_libraries(ScriptingTest RPGEngineMinimal)

# Create minimal scripting test executable (only scripting components)
add_executable(ScriptingMinimalTest
    examples/scripting_minimal_test.cpp
    src/scripting/LuaScriptEngine.cpp
)

target_include_directories(ScriptingMinimalTest PRIVATE src)
target_link_libraries(ScriptingMinimalTest ${LUA_LIBRARIES})
target_compile_options(ScriptingMinimalTest PRIVATE ${LUA_CFLAGS_OTHER})

# Create simple configuration test executable
add_executable(ConfigSimpleTest
    examples/config_simple_test.cpp
    src/core/ConfigurationManager.cpp
)

target_include_directories(ConfigSimpleTest PRIVATE src)

# Create debug systems minimal test executable (only performance profiler)
add_executable(DebugSystemsMinimalTest
    examples/debug_systems_minimal_test.cpp
    src/debug/PerformanceProfiler.cpp
)

target_include_directories(DebugSystemsMinimalTest PRIVATE src)

# Create content creation tools test executable
add_executable(ContentCreationToolsTest
    examples/content_creation_tools_test.cpp
    src/tools/MapEditor.cpp
    src/tools/AnimationEditor.cpp
    src/tools/DialogueEditor.cpp
)

target_include_directories(ContentCreationToolsTest PRIVATE src)

# Create debug systems test executable
add_executable(DebugSystemsTest
    examples/debug_systems_test.cpp
)

target_link_libraries(DebugSystemsTest RPGEngineMinimal)

# Create performance optimization test executable
add_executable(PerformanceOptimizationTest
    examples/performance_optimization_test.cpp
)

target_link_libraries(PerformanceOptimizationTest RPGEngineMinimal)

# Create simple performance optimization test executable
add_executable(PerformanceOptimizationSimpleTest
    examples/performance_optimization_simple_test.cpp
    src/graphics/FrustumCuller.cpp
    src/graphics/Camera.cpp
    src/graphics/Sprite.cpp
    src/core/ThreadPool.cpp
    src/core/MemoryPool.cpp
)

target_include_directories(PerformanceOptimizationSimpleTest PRIVATE src)

# Create cross-platform test executable
add_executable(CrossPlatformTest
    examples/cross_platform_test.cpp
)

target_include_directories(CrossPlatformTest PRIVATE src)

# Create graphics structure test executable (no OpenGL dependencies)
add_executable(GraphicsStructureTest
    examples/graphics_structure_test.cpp
    src/graphics/Camera.cpp
    src/graphics/Sprite.cpp
)

target_include_directories(GraphicsStructureTest PRIVATE src)

# Create graphics core test executable
add_executable(GraphicsCoreTest
    examples/graphics_core_test.cpp
    src/graphics/OpenGLAPI.cpp
    src/graphics/ShaderManager.cpp
    src/graphics/Texture.cpp
    src/graphics/Camera.cpp
)

target_include_directories(GraphicsCoreTest PRIVATE src)
target_link_libraries(GraphicsCoreTest ${LUA_LIBRARIES})
target_compile_options(GraphicsCoreTest PRIVATE ${LUA_CFLAGS_OTHER})

# Link graphics dependencies if available
if(NOT EMSCRIPTEN AND NOT NO_GRAPHICS)
    if(glfw3_FOUND)
        target_link_libraries(GraphicsCoreTest glfw)
    elseif(GLFW_FOUND)
        target_link_libraries(GraphicsCoreTest ${GLFW_LIBRARIES})
        target_compile_options(GraphicsCoreTest PRIVATE ${GLFW_CFLAGS_OTHER})
    endif()
    
    target_link_libraries(GraphicsCoreTest ${OPENGL_LIBRARIES})
endif()

# Configure platform-specific settings for all targets
configure_platform_target(RPGEngineMinimal)
configure_platform_target(QuestIntegrationSimpleTest)
configure_platform_target(CombatTest)
configure_platform_target(CombatIntegrationTest)
configure_platform_target(SaveSystemTest)
configure_platform_target(SaveSystemDemo)
configure_platform_target(SaveLoadIntegrationTest)
configure_platform_target(UIFrameworkSimpleTest)
configure_platform_target(UIGameSpecificTest)
configure_platform_target(ScriptingTest)
configure_platform_target(ScriptingMinimalTest)
configure_platform_target(ConfigSimpleTest)
configure_platform_target(DebugSystemsMinimalTest)
configure_platform_target(ContentCreationToolsTest)
configure_platform_target(DebugSystemsTest)
configure_platform_target(PerformanceOptimizationTest)
configure_platform_target(PerformanceOptimizationSimpleTest)
configure_platform_target(CrossPlatformTest)
configure_platform_target(GraphicsStructureTest)
configure_platform_target(GraphicsCoreTest)

# Create game integration test executable
add_executable(GameIntegrationTest
    examples/game_integration_test.cpp
    src/graphics/OpenGLAPI.cpp
    src/graphics/ShaderManager.cpp
    src/graphics/Camera.cpp
    src/graphics/Sprite.cpp
    src/entities/EntityManager.cpp
    src/components/ComponentManager.cpp
    src/components/StatsComponent.cpp
    src/components/InventoryComponent.cpp
    src/core/EngineCore.cpp
    src/core/EngineConfig.cpp
    src/core/Event.cpp
)

target_include_directories(GameIntegrationTest PRIVATE src)
target_link_libraries(GameIntegrationTest ${LUA_LIBRARIES})
target_compile_options(GameIntegrationTest PRIVATE ${LUA_CFLAGS_OTHER})

# Link graphics dependencies if available
if(NOT EMSCRIPTEN AND NOT NO_GRAPHICS)
    if(glfw3_FOUND)
        target_link_libraries(GameIntegrationTest glfw)
    elseif(GLFW_FOUND)
        target_link_libraries(GameIntegrationTest ${GLFW_LIBRARIES})
        target_compile_options(GameIntegrationTest PRIVATE ${GLFW_CFLAGS_OTHER})
    endif()
    
    target_link_libraries(GameIntegrationTest ${OPENGL_LIBRARIES})
endif()

configure_platform_target(GameIntegrationTest)

# Create comprehensive RPG demo game executable
add_executable(RPGDemoGame
    examples/rpg_demo_game.cpp
)

target_link_libraries(RPGDemoGame RPGEngineMinimal)
configure_platform_target(RPGDemoGame)

# Create simple RPG demo game executable (working version)
add_executable(RPGDemoSimple
    examples/rpg_demo_simple.cpp
    
    # Core Engine (working components only)
    src/core/EngineCore.cpp
    src/core/ConfigurationManager.cpp
    src/core/EngineConfig.cpp
    src/core/Event.cpp
    
    # Systems (working ones only)
    src/systems/SystemManager.cpp
    src/systems/System.cpp
    
    # Entities
    src/entities/EntityManager.cpp
    
    # Components (working ones only)
    src/components/ComponentManager.cpp
    src/components/StatsComponent.cpp
    src/components/InventoryComponent.cpp
    src/components/DialogueComponent.cpp
    src/components/QuestComponent.cpp
    
    # Graphics (basic)
    src/graphics/OpenGLAPI.cpp
    src/graphics/Camera.cpp
    src/graphics/ShaderManager.cpp
    
    # Scene Management
    src/scene/SceneManager.cpp
    src/scene/Scene.cpp
    src/scene/GameScene.cpp
    src/scene/MenuScene.cpp
    
    # Save System
    src/save/SaveManager.cpp
    src/save/JsonSaveSerializer.cpp
    
    # Resources
    src/resources/ResourceManager.cpp
    
    # Debug Tools
    src/debug/PerformanceProfiler.cpp
)

target_include_directories(RPGDemoSimple PRIVATE src)
target_link_libraries(RPGDemoSimple ${LUA_LIBRARIES})
target_compile_options(RPGDemoSimple PRIVATE ${LUA_CFLAGS_OTHER})

# Link graphics dependencies if available
if(NOT EMSCRIPTEN AND NOT NO_GRAPHICS)
    if(glfw3_FOUND)
        target_link_libraries(RPGDemoSimple glfw)
    elseif(GLFW_FOUND)
        target_link_libraries(RPGDemoSimple ${GLFW_LIBRARIES})
        target_compile_options(RPGDemoSimple PRIVATE ${GLFW_CFLAGS_OTHER})
    endif()
    
    target_link_libraries(RPGDemoSimple ${OPENGL_LIBRARIES})
endif()

configure_platform_target(RPGDemoSimple)

# Create minimal RPG demo game executable (basic working version)
add_executable(RPGDemoMinimal
    examples/rpg_demo_minimal.cpp
    
    # Core Engine (minimal working set)
    src/core/EngineCore.cpp
    src/core/EngineConfig.cpp
    src/core/Event.cpp
    
    # Entities
    src/entities/EntityManager.cpp
    
    # Components (basic working ones)
    src/components/ComponentManager.cpp
    src/components/StatsComponent.cpp
    src/components/InventoryComponent.cpp
    
    # Graphics (basic)
    src/graphics/OpenGLAPI.cpp
    src/graphics/Camera.cpp
    src/graphics/ShaderManager.cpp
)

target_include_directories(RPGDemoMinimal PRIVATE src)
target_link_libraries(RPGDemoMinimal ${LUA_LIBRARIES})
target_compile_options(RPGDemoMinimal PRIVATE ${LUA_CFLAGS_OTHER})

# Link graphics dependencies if available
if(NOT EMSCRIPTEN AND NOT NO_GRAPHICS)
    if(glfw3_FOUND)
        target_link_libraries(RPGDemoMinimal glfw)
    elseif(GLFW_FOUND)
        target_link_libraries(RPGDemoMinimal ${GLFW_LIBRARIES})
        target_compile_options(RPGDemoMinimal PRIVATE ${GLFW_CFLAGS_OTHER})
    endif()
    
    target_link_libraries(RPGDemoMinimal ${OPENGL_LIBRARIES})
endif()

configure_platform_target(RPGDemoMinimal)

# Web-specific configuration
if(EMSCRIPTEN)
    configure_web_target(CrossPlatformTest)
    configure_web_target(PerformanceOptimizationSimpleTest)
    configure_web_target(RPGDemoGame)
    configure_web_target(RPGDemoSimple)
    configure_web_target(RPGDemoMinimal)
endif()

message(STATUS "Minimal RPG Engine Quest Test configuration complete for ${PLATFORM_NAME}")