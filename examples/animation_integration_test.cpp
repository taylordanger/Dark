#include "../src/graphics/AnimationComponent.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Graphics;

int main() {
    std::cout << "Animation Integration Test" << std::endl;
    
    // Create animation component
    EntityId testEntityId = 1;
    auto animComponent = std::make_shared<AnimationComponent>(testEntityId);
    
    std::cout << "Created animation component for entity " << testEntityId << std::endl;
    
    // Create test animation sequences
    auto idleAnimation = std::make_shared<AnimationSequence>("idle", true);
    idleAnimation->addFrame(AnimationFrame(0, 0, 32, 32, 1.0f));
    
    auto walkAnimation = std::make_shared<AnimationSequence>("walk", true);
    walkAnimation->addFrame(AnimationFrame(0, 32, 32, 32, 0.2f));
    walkAnimation->addFrame(AnimationFrame(32, 32, 32, 32, 0.2f));
    walkAnimation->addFrame(AnimationFrame(64, 32, 32, 32, 0.2f));
    walkAnimation->addFrame(AnimationFrame(96, 32, 32, 32, 0.2f));
    
    auto runAnimation = std::make_shared<AnimationSequence>("run", true);
    runAnimation->addFrame(AnimationFrame(0, 64, 32, 32, 0.1f));
    runAnimation->addFrame(AnimationFrame(32, 64, 32, 32, 0.1f));
    runAnimation->addFrame(AnimationFrame(64, 64, 32, 32, 0.1f));
    runAnimation->addFrame(AnimationFrame(96, 64, 32, 32, 0.1f));
    
    auto attackAnimation = std::make_shared<AnimationSequence>("attack", false); // Non-looping
    attackAnimation->addFrame(AnimationFrame(0, 96, 32, 32, 0.1f));
    attackAnimation->addFrame(AnimationFrame(32, 96, 32, 32, 0.1f));
    attackAnimation->addFrame(AnimationFrame(64, 96, 32, 32, 0.2f));
    attackAnimation->addFrame(AnimationFrame(96, 96, 32, 32, 0.1f));
    
    // Add animations to component
    animComponent->addAnimation(idleAnimation);
    animComponent->addAnimation(walkAnimation);
    animComponent->addAnimation(runAnimation);
    animComponent->addAnimation(attackAnimation);
    
    std::cout << "Added animations to component" << std::endl;
    
    // Test animation triggers
    std::cout << "\nTesting animation triggers..." << std::endl;
    
    bool isMoving = false;
    bool isRunning = false;
    bool isAttacking = false;
    
    // Add movement trigger
    animComponent->addTrigger(AnimationTrigger("walk", AnimationTriggerType::OnStart, 
        [&isMoving, &isRunning]() { return isMoving && !isRunning; }, 1));
    
    // Add running trigger (higher priority)
    animComponent->addTrigger(AnimationTrigger("run", AnimationTriggerType::OnStart, 
        [&isRunning]() { return isRunning; }, 2));
    
    // Add attack trigger (highest priority)
    animComponent->addTrigger(AnimationTrigger("attack", AnimationTriggerType::OnAttack, 
        [&isAttacking]() { return isAttacking; }, 10));
    
    // Set default animation
    animComponent->setDefaultAnimation("idle");
    
    std::cout << "Set up animation triggers" << std::endl;
    
    // Test trigger system
    std::cout << "\nSimulating movement..." << std::endl;
    
    for (int i = 0; i < 15; i++) {
        // Simulate different states
        if (i == 3) {
            isMoving = true;
            std::cout << "Started moving..." << std::endl;
        }
        if (i == 7) {
            isRunning = true;
            std::cout << "Started running..." << std::endl;
        }
        if (i == 10) {
            isAttacking = true;
            std::cout << "Started attacking..." << std::endl;
        }
        if (i == 12) {
            isAttacking = false;
            std::cout << "Stopped attacking..." << std::endl;
        }
        if (i == 13) {
            isRunning = false;
            isMoving = false;
            std::cout << "Stopped moving..." << std::endl;
        }
        
        animComponent->update(0.1f);
        
        if (animComponent->isPlaying()) {
            std::cout << "Frame " << i << ": Playing '" << animComponent->getCurrentAnimationName() 
                      << "' frame " << animComponent->getCurrentFrameIndex() 
                      << " (priority: " << animComponent->getCurrentAnimationPriority() << ")";
            
            if (animComponent->isBlending()) {
                std::cout << " [Blending: " << (animComponent->getBlendProgress() * 100.0f) << "%]";
            }
            
            std::cout << std::endl;
        }
    }
    
    // Test animation blending
    std::cout << "\nTesting animation blending..." << std::endl;
    
    // Reset state
    isMoving = false;
    isRunning = false;
    isAttacking = false;
    
    // Start with idle
    animComponent->play("idle");
    
    // Update a few frames
    for (int i = 0; i < 3; i++) {
        animComponent->update(0.1f);
    }
    
    std::cout << "Current animation: " << animComponent->getCurrentAnimationName() << std::endl;
    
    // Blend to walk animation
    std::cout << "Blending to walk animation..." << std::endl;
    animComponent->playWithBlend("walk", AnimationBlendMode::Crossfade, 0.5f);
    
    // Update during blend
    for (int i = 0; i < 10; i++) {
        animComponent->update(0.1f);
        
        if (animComponent->isBlending()) {
            std::cout << "Blending progress: " << (animComponent->getBlendProgress() * 100.0f) 
                      << "% - Current: " << animComponent->getCurrentAnimationName() << std::endl;
        } else {
            std::cout << "Blend complete - Playing: " << animComponent->getCurrentAnimationName() << std::endl;
            break;
        }
    }
    
    // Test advanced transitions with blending
    std::cout << "\nTesting transitions with blending..." << std::endl;
    
    // Add transition with blending
    animComponent->addTransition(AnimationTransition("walk", "run", 
        [&isRunning]() { return isRunning; }, false, 
        AnimationBlendMode::Crossfade, 0.3f));
    
    // Add transition back to idle with blending
    animComponent->addTransition(AnimationTransition("run", "idle", 
        [&isRunning]() { return !isRunning; }, false, 
        AnimationBlendMode::Crossfade, 0.4f));
    
    // Simulate state changes
    for (int i = 0; i < 20; i++) {
        if (i == 5) {
            isRunning = true;
            std::cout << "Triggering run transition..." << std::endl;
        }
        if (i == 15) {
            isRunning = false;
            std::cout << "Triggering idle transition..." << std::endl;
        }
        
        animComponent->update(0.1f);
        
        std::cout << "Frame " << i << ": " << animComponent->getCurrentAnimationName();
        if (animComponent->isBlending()) {
            std::cout << " [Blending: " << (animComponent->getBlendProgress() * 100.0f) << "%]";
        }
        std::cout << std::endl;
    }
    
    // Test event callbacks with new features
    std::cout << "\nTesting enhanced event system..." << std::endl;
    
    int eventCount = 0;
    animComponent->registerEventCallback([&eventCount](const AnimationEvent& event) {
        eventCount++;
        const char* eventTypes[] = {"Started", "Finished", "FrameChanged", "LoopCompleted"};
        std::cout << "Event: " << eventTypes[static_cast<int>(event.type)] 
                  << " - Animation: " << event.animationName 
                  << " - Frame: " << event.frameIndex << std::endl;
    });
    
    // Play attack animation (non-looping)
    animComponent->play("attack");
    
    // Let it complete
    for (int i = 0; i < 15; i++) {
        animComponent->update(0.1f);
    }
    
    std::cout << "Received " << eventCount << " events" << std::endl;
    
    std::cout << "\nAnimation integration test completed successfully!" << std::endl;
    return 0;
}