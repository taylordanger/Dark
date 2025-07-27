#include "../src/graphics/AnimationComponent.h"
#include "../src/graphics/AnimationSystem.h"
#include "../src/graphics/Sprite.h"
#include "../src/entities/EntityManager.h"
#include "../src/components/ComponentManager.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;
using namespace RPGEngine::Graphics;

/**
 * Mock sprite renderer integration example
 * This demonstrates how the animation system would integrate with sprite rendering
 */
class MockSpriteRenderer {
public:
    void render(Entity entity, std::shared_ptr<Sprite> sprite, std::shared_ptr<AnimationComponent> animComponent) {
        if (!sprite || !animComponent) {
            return;
        }
        
        const AnimationFrame* frame = animComponent->getCurrentFrame();
        if (!frame) {
            return;
        }
        
        // In a real implementation, this would update the sprite's texture coordinates
        // and render it to the screen
        std::cout << "Rendering entity " << entity.getID() 
                  << " with animation '" << animComponent->getCurrentAnimationName() 
                  << "' frame " << animComponent->getCurrentFrameIndex()
                  << " at texture coords (" << frame->textureX << "," << frame->textureY 
                  << ") size " << frame->width << "x" << frame->height;
        
        if (animComponent->isBlending()) {
            std::cout << " [Blend: " << (animComponent->getBlendProgress() * 100.0f) << "%]";
        }
        
        std::cout << std::endl;
    }
};

int main() {
    std::cout << "Sprite Animation Integration Example" << std::endl;
    
    // Create managers
    auto entityManager = std::make_shared<EntityManager>();
    auto componentManager = std::make_shared<ComponentManager>();
    
    // Initialize managers
    entityManager->initialize();
    componentManager->initialize();
    
    // Create animation system
    AnimationSystem animSystem(entityManager, componentManager);
    animSystem.initialize();
    
    // Create mock renderer
    MockSpriteRenderer renderer;
    
    // Create test entity
    Entity player = entityManager->createEntity("Player");
    
    // Create animation component
    auto animComponent = componentManager->createComponent<AnimationComponent>(player);
    
    // Create sprite component (mock)
    auto sprite = std::make_shared<Sprite>();
    animComponent->setSprite(sprite);
    
    // Create animation sequences for a character
    auto idleAnimation = std::make_shared<AnimationSequence>("idle", true);
    idleAnimation->addFrame(AnimationFrame(0, 0, 64, 64, 0.5f));
    idleAnimation->addFrame(AnimationFrame(64, 0, 64, 64, 0.5f));
    
    auto walkAnimation = std::make_shared<AnimationSequence>("walk", true);
    walkAnimation->addFrame(AnimationFrame(0, 64, 64, 64, 0.15f));
    walkAnimation->addFrame(AnimationFrame(64, 64, 64, 64, 0.15f));
    walkAnimation->addFrame(AnimationFrame(128, 64, 64, 64, 0.15f));
    walkAnimation->addFrame(AnimationFrame(192, 64, 64, 64, 0.15f));
    
    auto jumpAnimation = std::make_shared<AnimationSequence>("jump", false);
    jumpAnimation->addFrame(AnimationFrame(0, 128, 64, 64, 0.1f));
    jumpAnimation->addFrame(AnimationFrame(64, 128, 64, 64, 0.3f));
    jumpAnimation->addFrame(AnimationFrame(128, 128, 64, 64, 0.1f));
    
    // Add animations
    animComponent->addAnimation(idleAnimation);
    animComponent->addAnimation(walkAnimation);
    animComponent->addAnimation(jumpAnimation);
    
    // Set up game state variables
    bool isMoving = false;
    bool isJumping = false;
    
    // Add animation triggers using the system
    animSystem.addAnimationTrigger(player, 
        AnimationTrigger("walk", AnimationTriggerType::OnStart, 
            [&isMoving, &isJumping]() { return isMoving && !isJumping; }, 1));
    
    animSystem.addAnimationTrigger(player, 
        AnimationTrigger("jump", AnimationTriggerType::Custom, 
            [&isJumping]() { return isJumping; }, 10));
    
    // Add transitions with blending
    animSystem.addAnimationTransition(player, 
        AnimationTransition("idle", "walk", nullptr, false, 
            AnimationBlendMode::Crossfade, 0.2f));
    
    animSystem.addAnimationTransition(player, 
        AnimationTransition("walk", "idle", nullptr, false, 
            AnimationBlendMode::Crossfade, 0.3f));
    
    animSystem.addAnimationTransition(player, 
        AnimationTransition("jump", "idle", nullptr, true, 
            AnimationBlendMode::Crossfade, 0.1f));
    
    // Set default animation
    animComponent->setDefaultAnimation("idle");
    
    std::cout << "\nSimulating game loop with sprite rendering..." << std::endl;
    
    // Simulate game loop
    for (int frame = 0; frame < 30; frame++) {
        // Simulate input/game state changes
        if (frame == 5) {
            isMoving = true;
            std::cout << "Player starts moving..." << std::endl;
        }
        if (frame == 15) {
            isJumping = true;
            std::cout << "Player jumps!" << std::endl;
        }
        if (frame == 18) {
            isJumping = false;
            std::cout << "Player lands..." << std::endl;
        }
        if (frame == 25) {
            isMoving = false;
            std::cout << "Player stops moving..." << std::endl;
        }
        
        // Update animation system
        animSystem.update(0.1f);
        
        // Render sprite with current animation frame
        renderer.render(player, sprite, animComponent);
    }
    
    // Demonstrate manual animation control with blending
    std::cout << "\nDemonstrating manual animation control..." << std::endl;
    
    // Play specific animation with blending
    animSystem.playAnimationWithBlend(player, "walk", AnimationBlendMode::Crossfade, 0.5f);
    
    for (int i = 0; i < 10; i++) {
        animSystem.update(0.1f);
        renderer.render(player, sprite, animComponent);
    }
    
    // Cleanup
    animSystem.shutdown();
    componentManager->shutdown();
    entityManager->shutdown();
    
    std::cout << "\nSprite animation integration example completed!" << std::endl;
    return 0;
}