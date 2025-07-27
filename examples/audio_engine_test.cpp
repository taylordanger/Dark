#include "../src/audio/AudioManager.h"
#include "../src/audio/MockAudioDevice.h"
#include "../src/resources/ResourceManager.h"
#include "../src/resources/AudioResource.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Audio;
using namespace RPGEngine::Resources;

int main() {
    std::cout << "Audio Engine Foundation Test" << std::endl;
    
    // Create resource manager
    auto resourceManager = std::make_shared<ResourceManager>();
    resourceManager->initialize();
    
    // Create mock audio device
    auto audioDevice = std::make_shared<MockAudioDevice>();
    
    // Create audio manager with custom device
    AudioManager audioManager(resourceManager, audioDevice);
    
    // Set up event callback
    int eventCount = 0;
    audioManager.setEventCallback([&eventCount](const std::string& type, const std::string& details) {
        eventCount++;
        std::cout << "Audio Event [" << type << "]: " << details << std::endl;
    });
    
    // Test device capabilities
    std::cout << "\nTesting audio device capabilities..." << std::endl;
    
    auto availableDevices = audioManager.getAvailableDevices();
    std::cout << "Available devices:" << std::endl;
    for (const auto& device : availableDevices) {
        std::cout << "  - " << device << std::endl;
    }
    
    // Initialize audio manager
    if (!audioManager.initialize("Mock High Quality Device")) {
        std::cerr << "Failed to initialize audio manager" << std::endl;
        return -1;
    }
    
    std::cout << "Current device: " << audioManager.getCurrentDevice() << std::endl;
    
    auto capabilities = audioManager.getDeviceCapabilities();
    std::cout << "Device capabilities:" << std::endl;
    std::cout << "  Max sources: " << capabilities.maxSources << std::endl;
    std::cout << "  Max buffers: " << capabilities.maxBuffers << std::endl;
    std::cout << "  3D support: " << (capabilities.supports3D ? "Yes" : "No") << std::endl;
    std::cout << "  Effects support: " << (capabilities.supportsEffects ? "Yes" : "No") << std::endl;
    std::cout << "  Streaming support: " << (capabilities.supportsStreaming ? "Yes" : "No") << std::endl;
    
    // Test volume controls
    std::cout << "\nTesting volume controls..." << std::endl;
    
    std::cout << "Initial volumes:" << std::endl;
    std::cout << "  Master: " << audioManager.getMasterVolume() << std::endl;
    std::cout << "  Sound: " << audioManager.getSoundVolume() << std::endl;
    std::cout << "  Music: " << audioManager.getMusicVolume() << std::endl;
    
    audioManager.setMasterVolume(0.8f);
    audioManager.setSoundVolume(0.9f);
    audioManager.setMusicVolume(0.7f);
    
    std::cout << "Updated volumes:" << std::endl;
    std::cout << "  Master: " << audioManager.getMasterVolume() << std::endl;
    std::cout << "  Sound: " << audioManager.getSoundVolume() << std::endl;
    std::cout << "  Music: " << audioManager.getMusicVolume() << std::endl;
    
    // Test mute functionality
    std::cout << "\nTesting mute functionality..." << std::endl;
    std::cout << "Muted: " << (audioManager.isMuted() ? "Yes" : "No") << std::endl;
    
    audioManager.setMuted(true);
    std::cout << "After muting: " << (audioManager.isMuted() ? "Yes" : "No") << std::endl;
    
    audioManager.setMuted(false);
    std::cout << "After unmuting: " << (audioManager.isMuted() ? "Yes" : "No") << std::endl;
    
    // Test 3D audio listener
    std::cout << "\nTesting 3D audio listener..." << std::endl;
    
    AudioListener listener;
    listener.x = 10.0f;
    listener.y = 5.0f;
    listener.z = 0.0f;
    listener.forwardX = 1.0f;
    listener.forwardY = 0.0f;
    listener.forwardZ = 0.0f;
    listener.velocityX = 2.0f;
    listener.velocityY = 0.0f;
    listener.velocityZ = 0.0f;
    
    audioManager.setListener(listener);
    
    const auto& currentListener = audioManager.getListener();
    std::cout << "Listener position: (" << currentListener.x << ", " << currentListener.y << ", " << currentListener.z << ")" << std::endl;
    std::cout << "Listener forward: (" << currentListener.forwardX << ", " << currentListener.forwardY << ", " << currentListener.forwardZ << ")" << std::endl;
    std::cout << "Listener velocity: (" << currentListener.velocityX << ", " << currentListener.velocityY << ", " << currentListener.velocityZ << ")" << std::endl;
    
    // Create mock audio resources
    std::cout << "\nCreating mock audio resources..." << std::endl;
    
    auto soundResource = std::make_shared<AudioResource>("test_sound", "assets/sounds/test.wav", false);
    auto musicResource = std::make_shared<AudioResource>("test_music", "assets/music/background.ogg", true);
    
    resourceManager->addResource(soundResource);
    resourceManager->addResource(musicResource);
    
    // Test sound playback
    std::cout << "\nTesting sound playback..." << std::endl;
    
    auto soundSource = audioManager.playSound("test_sound", 0.8f, 1.2f, false);
    if (soundSource) {
        std::cout << "Sound playing: " << (soundSource->isPlaying() ? "Yes" : "No") << std::endl;
        std::cout << "Sound volume: " << soundSource->getVolume() << std::endl;
        std::cout << "Sound pitch: " << soundSource->getPitch() << std::endl;
    }
    
    // Test 3D sound playback
    std::cout << "\nTesting 3D sound playback..." << std::endl;
    
    auto sound3D = audioManager.play3DSound("test_sound", 5.0f, 0.0f, -10.0f, 0.6f, 1.0f, false);
    if (sound3D) {
        std::cout << "3D sound created successfully" << std::endl;
    }
    
    // Test music playback
    std::cout << "\nTesting music playback..." << std::endl;
    
    auto musicSource = audioManager.playMusic("test_music", 0.5f, true);
    if (musicSource) {
        std::cout << "Music playing: " << (musicSource->isPlaying() ? "Yes" : "No") << std::endl;
        std::cout << "Music looping: " << (musicSource->isLooping() ? "Yes" : "No") << std::endl;
    }
    
    // Test audio controls
    std::cout << "\nTesting audio controls..." << std::endl;
    
    // Pause all audio
    audioManager.pauseAll();
    std::cout << "All audio paused" << std::endl;
    
    if (soundSource) {
        std::cout << "Sound paused: " << (soundSource->isPaused() ? "Yes" : "No") << std::endl;
    }
    if (musicSource) {
        std::cout << "Music paused: " << (musicSource->isPaused() ? "Yes" : "No") << std::endl;
    }
    
    // Resume all audio
    audioManager.resumeAll();
    std::cout << "All audio resumed" << std::endl;
    
    // Stop all audio
    audioManager.stopAll();
    std::cout << "All audio stopped" << std::endl;
    
    // Test audio manager update
    std::cout << "\nTesting audio manager update..." << std::endl;
    
    for (int i = 0; i < 5; i++) {
        audioManager.update(0.1f);
        std::cout << "Update " << i + 1 << " completed" << std::endl;
    }
    
    // Cleanup
    audioManager.shutdown();
    resourceManager->shutdown();
    
    std::cout << "\nReceived " << eventCount << " audio events" << std::endl;
    std::cout << "Audio engine foundation test completed successfully!" << std::endl;
    
    return 0;
}