#include "../src/graphics/AnimationComponent.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Graphics;

int main() {
    std::cout << "Animation Framework Test" << std::endl;
    
    // Create animation component directly
    EntityId testEntityId = 1;
    auto animComponent = std::make_shared<AnimationComponent>(testEntityId);
    
    std::cout << "Created animation component for entity " << testEntityId << std::endl;
    
    // Create test animation sequence
    auto walkAnimation = std::make_shared<AnimationSequence>("walk", true);
    walkAnimation->addFrame(AnimationFrame(0, 0, 32, 32, 0.2f));
    walkAnimation->addFrame(AnimationFrame(32, 0, 32, 32, 0.2f));
    walkAnimation->addFrame(AnimationFrame(64, 0, 32, 32, 0.2f));
    walkAnimation->addFrame(AnimationFrame(96, 0, 32, 32, 0.2f));
    
    auto idleAnimation = std::make_shared<AnimationSequence>("idle", true);
    idleAnimation->addFrame(AnimationFrame(0, 32, 32, 32, 1.0f));
    
    // Add animations to component
    if (!animComponent->addAnimation(walkAnimation)) {
        std::cerr << "Failed to add walk animation" << std::endl;
        return -1;
    }
    
    if (!animComponent->addAnimation(idleAnimation)) {
        std::cerr << "Failed to add idle animation" << std::endl;
        return -1;
    }
    
    std::cout << "Added animations to component" << std::endl;
    
    // Test animation state machine
    // Add transition from idle to walk (manual trigger)
    bool shouldWalk = false;
    animComponent->addTransition(AnimationTransition("idle", "walk", 
        [&shouldWalk]() { return shouldWalk; }));
    
    // Add transition from walk to idle (auto transition when walk finishes)
    animComponent->addTransition(AnimationTransition("walk", "idle", 
        nullptr, true));
    
    // Set default animation
    animComponent->setDefaultAnimation("idle");
    
    std::cout << "Set up animation state machine" << std::endl;
    
    // Test animation playback
    std::cout << "\nTesting animation playback..." << std::endl;
    
    // Simulate some updates
    for (int i = 0; i < 10; i++) {
        animComponent->update(0.1f);
        
        if (animComponent->isPlaying()) {
            std::cout << "Frame " << i << ": Playing '" << animComponent->getCurrentAnimationName() 
                      << "' frame " << animComponent->getCurrentFrameIndex() 
                      << " (state: " << static_cast<int>(animComponent->getAnimationState()) << ")" << std::endl;
        }
        
        // Trigger walk animation after 3 frames
        if (i == 3) {
            shouldWalk = true;
            std::cout << "Triggering walk animation..." << std::endl;
        }
        
        // Stop walking after 6 frames
        if (i == 6) {
            shouldWalk = false;
            std::cout << "Stopping walk animation..." << std::endl;
        }
    }
    
    // Test animation controls
    std::cout << "\nTesting animation controls..." << std::endl;
    
    // Play walk animation directly
    if (animComponent->play("walk")) {
        std::cout << "Started walk animation" << std::endl;
    }
    
    // Update a few frames
    for (int i = 0; i < 3; i++) {
        animComponent->update(0.2f);
        std::cout << "Frame " << animComponent->getCurrentFrameIndex() << std::endl;
    }
    
    // Pause animation
    animComponent->pause();
    std::cout << "Paused animation" << std::endl;
    
    // Update while paused (should not advance)
    int pausedFrame = animComponent->getCurrentFrameIndex();
    animComponent->update(0.2f);
    if (animComponent->getCurrentFrameIndex() == pausedFrame) {
        std::cout << "Animation correctly paused" << std::endl;
    }
    
    // Resume animation
    animComponent->resume();
    std::cout << "Resumed animation" << std::endl;
    
    // Update after resume
    animComponent->update(0.2f);
    if (animComponent->getCurrentFrameIndex() != pausedFrame) {
        std::cout << "Animation correctly resumed" << std::endl;
    }
    
    // Stop animation
    animComponent->stop();
    std::cout << "Stopped animation" << std::endl;
    
    // Test animation events
    std::cout << "\nTesting animation events..." << std::endl;
    
    int eventCount = 0;
    animComponent->registerEventCallback([&eventCount](const AnimationEvent& event) {
        eventCount++;
        std::cout << "Animation event: " << static_cast<int>(event.type) 
                  << " for animation '" << event.animationName 
                  << "' frame " << event.frameIndex << std::endl;
    });
    
    // Play animation and let it complete
    animComponent->play("walk");
    for (int i = 0; i < 20; i++) {
        animComponent->update(0.1f);
    }
    
    std::cout << "Received " << eventCount << " animation events" << std::endl;
    
    std::cout << "\nAnimation framework test completed successfully!" << std::endl;
    return 0;
}