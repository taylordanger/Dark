#include "AnimationSystem.h"
#include <iostream>

namespace RPGEngine {
namespace Graphics {

AnimationSystem::AnimationSystem(std::shared_ptr<EntityManager> entityManager,
                               std::shared_ptr<ComponentManager> componentManager)
    : System("AnimationSystem")
    , m_entityManager(entityManager)
    , m_componentManager(componentManager)
{
}

AnimationSystem::~AnimationSystem() {
    if (isInitialized()) {
        shutdown();
    }
}

bool AnimationSystem::onInitialize() {
    if (!m_entityManager) {
        std::cerr << "Entity manager not provided to AnimationSystem" << std::endl;
        return false;
    }
    
    if (!m_componentManager) {
        std::cerr << "Component manager not provided to AnimationSystem" << std::endl;
        return false;
    }
    
    std::cout << "AnimationSystem initialized" << std::endl;
    return true;
}

void AnimationSystem::onUpdate(float deltaTime) {
    // Get all entities with animation components
    auto entities = m_componentManager->getEntitiesWithComponent<AnimationComponent>();
    
    // Update each animation component
    for (const auto& entity : entities) {
        auto animComponent = getAnimationComponent(entity);
        if (animComponent) {
            animComponent->update(deltaTime);
        }
    }
}

void AnimationSystem::onShutdown() {
    std::cout << "AnimationSystem shutdown" << std::endl;
}

bool AnimationSystem::playAnimation(Entity entity, const std::string& animationName, bool reset) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    return animComponent->play(animationName, reset);
}

bool AnimationSystem::stopAnimation(Entity entity) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    animComponent->stop();
    return true;
}

bool AnimationSystem::pauseAnimation(Entity entity) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    animComponent->pause();
    return true;
}

bool AnimationSystem::resumeAnimation(Entity entity) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    animComponent->resume();
    return true;
}

bool AnimationSystem::hasAnimationComponent(Entity entity) const {
    return m_componentManager->hasComponent<AnimationComponent>(entity);
}

std::shared_ptr<AnimationComponent> AnimationSystem::getAnimationComponent(Entity entity) const {
    return m_componentManager->getComponent<AnimationComponent>(entity);
}

bool AnimationSystem::playAnimationWithBlend(Entity entity, const std::string& animationName, 
                                            AnimationBlendMode blendMode, float blendDuration, bool reset) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    return animComponent->playWithBlend(animationName, blendMode, blendDuration, reset);
}

bool AnimationSystem::addAnimationTrigger(Entity entity, const AnimationTrigger& trigger) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    animComponent->addTrigger(trigger);
    return true;
}

bool AnimationSystem::removeAnimationTrigger(Entity entity, const std::string& animationName, AnimationTriggerType triggerType) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    return animComponent->removeTrigger(animationName, triggerType);
}

bool AnimationSystem::addAnimationTransition(Entity entity, const AnimationTransition& transition) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    animComponent->addTransition(transition);
    return true;
}

bool AnimationSystem::removeAnimationTransition(Entity entity, const std::string& fromAnimation, const std::string& toAnimation) {
    auto animComponent = getAnimationComponent(entity);
    if (!animComponent) {
        return false;
    }
    
    return animComponent->removeTransition(fromAnimation, toAnimation);
}

} // namespace Graphics
} // namespace RPGEngine