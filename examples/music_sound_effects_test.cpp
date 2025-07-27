#include "../src/audio/MusicManager.h"
#include "../src/audio/SoundEffectManager.h"
#include "../src/audio/MockAudioDevice.h"
#include "../src/resources/ResourceManager.h"
#include "../src/resources/AudioResource.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace RPGEngine;
using namespace RPGEngine::Audio;
using namespace RPGEngine::Resources;

int main() {
    std::cout << "Music and Sound Effects Test" << std::endl;
    
    // Create resource manager
    auto resourceManager = std::make_shared<ResourceManager>();
    resourceManager->initialize();
    
    // Create audio manager with mock device
    auto audioDevice = std::make_shared<MockAudioDevice>();
    AudioManager audioManager(resourceManager, audioDevice);
    
    // Initialize audio manager
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize audio manager" << std::endl;
        return -1;
    }
    
    // Create music and sound effect managers
    auto musicManager = std::make_shared<MusicManager>(std::make_shared<AudioManager>(audioManager));
    auto soundManager = std::make_shared<SoundEffectManager>(std::make_shared<AudioManager>(audioManager));
    
    // Create mock audio resources
    std::cout << "\nCreating mock audio resources..." << std::endl;
    
    auto bgMusic1 = std::make_shared<AudioResource>("bg_music_1", "assets/music/background1.ogg", true);
    auto bgMusic2 = std::make_shared<AudioResource>("bg_music_2", "assets/music/background2.ogg", true);
    auto combatMusic = std::make_shared<AudioResource>("combat_music", "assets/music/combat.ogg", true);
    
    auto footstepSound = std::make_shared<AudioResource>("footstep", "assets/sounds/footstep.wav", false);
    auto swordSound = std::make_shared<AudioResource>("sword_swing", "assets/sounds/sword.wav", false);
    auto ambientSound = std::make_shared<AudioResource>("wind", "assets/sounds/wind.ogg", false);
    auto uiSound = std::make_shared<AudioResource>("button_click", "assets/sounds/click.wav", false);
    
    resourceManager->addResource(bgMusic1);
    resourceManager->addResource(bgMusic2);
    resourceManager->addResource(combatMusic);
    resourceManager->addResource(footstepSound);
    resourceManager->addResource(swordSound);
    resourceManager->addResource(ambientSound);
    resourceManager->addResource(uiSound);
    
    // Test music manager
    std::cout << "\nTesting music manager..." << std::endl;
    
    // Set up 3D listener
    AudioListener listener;
    listener.x = 0.0f;
    listener.y = 0.0f;
    listener.z = 0.0f;
    audioManager.setListener(listener);
    
    // Play background music with crossfade
    std::cout << "Playing background music 1..." << std::endl;
    musicManager->playMusic("bg_music_1", 0.8f, true, MusicTransitionType::FadeIn, 1.0f);
    
    // Simulate some time passing
    for (int i = 0; i < 15; i++) {
        musicManager->update(0.1f);
        soundManager->update(0.1f);
        
        if (i == 5) {
            std::cout << "Crossfading to background music 2..." << std::endl;
            musicManager->playMusic("bg_music_2", 0.7f, true, MusicTransitionType::Crossfade, 2.0f);
        }
        
        if (i == 10) {
            std::cout << "Switching to combat music..." << std::endl;
            musicManager->playMusic("combat_music", 1.0f, true, MusicTransitionType::FadeOut, 1.5f);
        }
        
        std::cout << "Music state: " << static_cast<int>(musicManager->getState()) 
                  << ", Volume: " << musicManager->getCurrentVolume() << std::endl;
    }
    
    // Test music queue
    std::cout << "\nTesting music queue..." << std::endl;
    
    musicManager->queueMusic(MusicTrack("bg_music_1", 0.6f, true), MusicTransitionType::Crossfade, 1.0f);
    musicManager->queueMusic(MusicTrack("bg_music_2", 0.8f, true), MusicTransitionType::Crossfade, 1.5f);
    
    std::cout << "Queue size: " << musicManager->getQueueSize() << std::endl;
    
    // Process queue
    for (int i = 0; i < 20; i++) {
        musicManager->update(0.1f);
        
        if (i == 10) {
            std::cout << "Skipping to next track..." << std::endl;
            musicManager->skipToNext();
        }
    }
    
    // Test sound effect manager
    std::cout << "\nTesting sound effect manager..." << std::endl;
    
    // Test category volumes
    std::cout << "Setting category volumes..." << std::endl;
    soundManager->setCategoryVolume(SoundCategory::UI, 0.8f);
    soundManager->setCategoryVolume(SoundCategory::Combat, 1.0f);
    soundManager->setCategoryVolume(SoundCategory::Ambient, 0.6f);
    
    std::cout << "UI volume: " << soundManager->getCategoryVolume(SoundCategory::UI) << std::endl;
    std::cout << "Combat volume: " << soundManager->getCategoryVolume(SoundCategory::Combat) << std::endl;
    std::cout << "Ambient volume: " << soundManager->getCategoryVolume(SoundCategory::Ambient) << std::endl;
    
    // Play 2D sound effects
    std::cout << "\nPlaying 2D sound effects..." << std::endl;
    
    uint32_t uiSoundId = soundManager->play2DSound("button_click", 0.8f, 1.0f, false, SoundCategory::UI);
    uint32_t combatSoundId = soundManager->play2DSound("sword_swing", 1.0f, 1.2f, false, SoundCategory::Combat);
    
    std::cout << "UI sound ID: " << uiSoundId << std::endl;
    std::cout << "Combat sound ID: " << combatSoundId << std::endl;
    
    // Play 3D sound effects
    std::cout << "\nPlaying 3D sound effects..." << std::endl;
    
    uint32_t footstepId = soundManager->play3DSound("footstep", 5.0f, 0.0f, -2.0f, 0.7f, 1.0f, false, SoundCategory::Action);
    uint32_t windId = soundManager->play3DSound("wind", -10.0f, 0.0f, 5.0f, 0.5f, 0.8f, true, SoundCategory::Ambient);
    
    std::cout << "Footstep sound ID: " << footstepId << std::endl;
    std::cout << "Wind sound ID: " << windId << std::endl;
    
    // Test 3D positioning updates
    std::cout << "\nTesting 3D positioning updates..." << std::endl;
    
    for (int i = 0; i < 10; i++) {
        soundManager->update(0.1f);
        
        // Move footstep sound
        float x = 5.0f - i * 1.0f;
        soundManager->updateSoundPosition(footstepId, x, 0.0f, -2.0f);
        soundManager->updateSoundVelocity(footstepId, -1.0f, 0.0f, 0.0f);
        
        std::cout << "Updated footstep position to (" << x << ", 0, -2)" << std::endl;
        
        // Move listener
        listener.x = i * 0.5f;
        audioManager.setListener(listener);
    }
    
    // Test sound completion callback
    std::cout << "\nTesting sound completion callback..." << std::endl;
    
    int completedSounds = 0;
    soundManager->setSoundCompletionCallback([&completedSounds](uint32_t soundId, const std::string& resourceId) {
        completedSounds++;
        std::cout << "Sound completed: ID=" << soundId << ", Resource=" << resourceId << std::endl;
    });
    
    // Play some short sounds
    for (int i = 0; i < 5; i++) {
        soundManager->play2DSound("button_click", 0.5f, 1.0f + i * 0.1f, false, SoundCategory::UI);
    }
    
    // Update to process completions
    for (int i = 0; i < 10; i++) {
        soundManager->update(0.1f);
    }
    
    std::cout << "Completed sounds: " << completedSounds << std::endl;
    
    // Test category management
    std::cout << "\nTesting category management..." << std::endl;
    
    std::cout << "Active sounds before: " << soundManager->getTotalActiveSounds() << std::endl;
    std::cout << "UI sounds: " << soundManager->getActiveSoundCount(SoundCategory::UI) << std::endl;
    std::cout << "Ambient sounds: " << soundManager->getActiveSoundCount(SoundCategory::Ambient) << std::endl;
    
    // Disable UI category
    soundManager->setCategoryEnabled(SoundCategory::UI, false);
    std::cout << "Disabled UI category" << std::endl;
    
    // Try to play UI sound (should fail)
    uint32_t failedSoundId = soundManager->play2DSound("button_click", 1.0f, 1.0f, false, SoundCategory::UI);
    std::cout << "Attempted UI sound ID (should be 0): " << failedSoundId << std::endl;
    
    // Re-enable UI category
    soundManager->setCategoryEnabled(SoundCategory::UI, true);
    std::cout << "Re-enabled UI category" << std::endl;
    
    // Test max concurrent sounds
    std::cout << "\nTesting max concurrent sounds..." << std::endl;
    
    soundManager->setMaxConcurrentSounds(SoundCategory::UI, 3);
    
    // Play more UI sounds than the limit
    std::vector<uint32_t> uiSounds;
    for (int i = 0; i < 5; i++) {
        uint32_t id = soundManager->play2DSound("button_click", 0.5f, 1.0f, true, SoundCategory::UI);
        uiSounds.push_back(id);
        std::cout << "UI sound " << i << " ID: " << id << std::endl;
    }
    
    std::cout << "UI sounds after limit test: " << soundManager->getActiveSoundCount(SoundCategory::UI) << std::endl;
    
    // Test music controls
    std::cout << "\nTesting music controls..." << std::endl;
    
    std::cout << "Music playing: " << (musicManager->isPlaying() ? "Yes" : "No") << std::endl;
    
    musicManager->pauseMusic(0.5f);
    std::cout << "Paused music" << std::endl;
    
    for (int i = 0; i < 10; i++) {
        musicManager->update(0.1f);
    }
    
    std::cout << "Music paused: " << (musicManager->isPaused() ? "Yes" : "No") << std::endl;
    
    musicManager->resumeMusic(0.5f);
    std::cout << "Resumed music" << std::endl;
    
    for (int i = 0; i < 10; i++) {
        musicManager->update(0.1f);
    }
    
    // Test volume changes
    std::cout << "\nTesting volume changes..." << std::endl;
    
    musicManager->setVolume(0.3f, 1.0f);
    std::cout << "Fading music volume to 0.3" << std::endl;
    
    for (int i = 0; i < 15; i++) {
        musicManager->update(0.1f);
        std::cout << "Music volume: " << musicManager->getCurrentVolume() << std::endl;
    }
    
    // Cleanup
    std::cout << "\nCleaning up..." << std::endl;
    
    soundManager->stopAllSounds(0.0f);
    musicManager->stopMusic(0.0f);
    
    audioManager.shutdown();
    resourceManager->shutdown();
    
    std::cout << "\nMusic and sound effects test completed successfully!" << std::endl;
    
    return 0;
}