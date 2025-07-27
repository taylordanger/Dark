#include "../src/resources/ResourceManager.h"
#include "../src/resources/AudioResource.h"
#include "../src/audio/AudioManager.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace RPGEngine;
using namespace RPGEngine::Resources;
using namespace RPGEngine::Audio;

int main() {
    // Create resource manager
    auto resourceManager = std::make_shared<ResourceManager>();
    
    // Initialize resource manager
    if (!resourceManager->initialize()) {
        std::cerr << "Failed to initialize resource manager" << std::endl;
        return 1;
    }
    
    // Set base path for resources
    resourceManager->setBasePath("assets/");
    
    // Create audio manager
    AudioManager audioManager(resourceManager);
    
    // Initialize audio manager
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize audio manager" << std::endl;
        return 1;
    }
    
    // Create audio resources
    auto sound1 = std::make_shared<AudioResource>("sound1", "assets/audio/sound1.wav");
    auto sound2 = std::make_shared<AudioResource>("sound2", "assets/audio/sound2.wav");
    auto music1 = std::make_shared<AudioResource>("music1", "assets/audio/music1.ogg");
    auto music2 = std::make_shared<AudioResource>("music2", "assets/audio/music2.mp3");
    
    // Add resources to manager
    resourceManager->addResource(sound1);
    resourceManager->addResource(sound2);
    resourceManager->addResource(music1);
    resourceManager->addResource(music2);
    
    // Load resources
    resourceManager->loadResource("sound1");
    resourceManager->loadResource("sound2");
    resourceManager->loadResource("music1");
    resourceManager->loadResource("music2");
    
    // Print instructions
    std::cout << "Audio Test" << std::endl;
    std::cout << "Press 1 to play sound1" << std::endl;
    std::cout << "Press 2 to play sound2" << std::endl;
    std::cout << "Press 3 to play music1" << std::endl;
    std::cout << "Press 4 to play music2" << std::endl;
    std::cout << "Press 5 to stop all sounds" << std::endl;
    std::cout << "Press 6 to stop all music" << std::endl;
    std::cout << "Press 7 to pause all" << std::endl;
    std::cout << "Press 8 to resume all" << std::endl;
    std::cout << "Press 9 to toggle mute" << std::endl;
    std::cout << "Press 0 to quit" << std::endl;
    
    // Main loop
    bool running = true;
    while (running) {
        // Update audio manager
        audioManager.update(0.016f); // ~60 FPS
        
        // Process input
        if (std::cin.peek() != EOF) {
            char input = std::cin.get();
            
            switch (input) {
                case '1':
                    std::cout << "Playing sound1" << std::endl;
                    audioManager.playSound("sound1", 1.0f, 1.0f, false);
                    break;
                    
                case '2':
                    std::cout << "Playing sound2" << std::endl;
                    audioManager.playSound("sound2", 0.8f, 1.2f, false);
                    break;
                    
                case '3':
                    std::cout << "Playing music1" << std::endl;
                    audioManager.playMusic("music1", 0.7f, true);
                    break;
                    
                case '4':
                    std::cout << "Playing music2" << std::endl;
                    audioManager.playMusic("music2", 0.7f, true);
                    break;
                    
                case '5':
                    std::cout << "Stopping all sounds" << std::endl;
                    audioManager.stopAllSounds();
                    break;
                    
                case '6':
                    std::cout << "Stopping all music" << std::endl;
                    audioManager.stopAllMusic();
                    break;
                    
                case '7':
                    std::cout << "Pausing all audio" << std::endl;
                    audioManager.pauseAll();
                    break;
                    
                case '8':
                    std::cout << "Resuming all audio" << std::endl;
                    audioManager.resumeAll();
                    break;
                    
                case '9':
                    audioManager.setMuted(!audioManager.isMuted());
                    std::cout << "Mute: " << (audioManager.isMuted() ? "ON" : "OFF") << std::endl;
                    break;
                    
                case '0':
                    std::cout << "Quitting" << std::endl;
                    running = false;
                    break;
                    
                default:
                    break;
            }
        }
        
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Shutdown audio manager
    audioManager.shutdown();
    
    // Shutdown resource manager
    resourceManager->shutdown();
    
    return 0;
}