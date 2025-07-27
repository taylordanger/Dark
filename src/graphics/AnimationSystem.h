#pragma once

#include "../systems/System.h"
#include "../entities/EntityManager.h"
#include "../components/ComponentManager.h"
#include "AnimationComponent.h"
#include <memory>

namespace RPGEngine {
namespace Graphics {

/**
 * Animation system
 * Updates all animation components
 */
class AnimationSystem : public System {
public:
    /**
     * Constructor
     * @param entityManager Entity manager
     * @param componentManager Component manager
     */
    AnimationSystem(std::shared_ptr<EntityManager> entityManager,
                   std::shared_ptr<ComponentManager> componentManager);
    
    /**
     * Destructor
     */
    ~AnimationSystem();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Play an animation on an entity
     * @param entity Entity
     * @param animationName Animation name
     * @param reset Whether to reset the animation if it's already playing
     * @return true if the animation was started
     */
    bool playAnimation(Entity entity, const std::string& animationName, bool reset = true);
    
    /**
     * Stop an animation on an entity
     * @param entity Entity
     * @return true if the animation was stopped
     */
    bool stopAnimation(Entity entity);
    
    /**
     * Pause an animation on an entity
     * @param entity Entity
     * @return true if the animation was paused
     */
    bool pauseAnimation(Entity entity);
    
    /**
     * Resume an animation on an entity
     * @param entity Entity
     * @return true if the animation was resumed
     */
    bool resumeAnimation(Entity entity);
    
    /**
     * Check if an entity has an animation component
     * @param entity Entity
     * @return true if the entity has an animation component
     */
    bool hasAnimationComponent(Entity entity) const;
    
    /**
     * Get an animation component from an entity
     * @param entity Entity
     * @return Animation component, or nullptr if not found
     */
    std::shared_ptr<AnimationComponent> getAnimationComponent(Entity entity) const;
    
    /**
     * Play animation with blending
     * @param entity Entity
     * @param animationName Animation name
     * @param blendMode Blend mode
     * @param blendDuration Blend duration in seconds
     * @param reset Whether to reset the animation if it's already playing
     * @return true if the animation was started
     */
    bool playAnimationWithBlend(Entity entity, const std::string& animationName, 
                               AnimationBlendMode blendMode, float blendDuration = 0.3f, bool reset = true);
    
    /**
     * Add animation trigger to an entity
     * @param entity Entity
     * @param trigger Animation trigger
     * @return true if the trigger was added
     */
    bool addAnimationTrigger(Entity entity, const AnimationTrigger& trigger);
    
    /**
     * Remove animation trigger from an entity
     * @param entity Entity
     * @param animationName Animation name
     * @param triggerType Trigger type
     * @return true if the trigger was removed
     */
    bool removeAnimationTrigger(Entity entity, const std::string& animationName, AnimationTriggerType triggerType);
    
    /**
     * Add animation transition to an entity
     * @param entity Entity
     * @param transition Animation transition
     * @return true if the transition was added
     */
    bool addAnimationTransition(Entity entity, const AnimationTransition& transition);
    
    /**
     * Remove animation transition from an entity
     * @param entity Entity
     * @param fromAnimation Source animation name
     * @param toAnimation Target animation name
     * @return true if the transition was removed
     */
    bool removeAnimationTransition(Entity entity, const std::string& fromAnimation, const std::string& toAnimation);
    
private:
    std::shared_ptr<EntityManager> m_entityManager;        // Entity manager
    std::shared_ptr<ComponentManager> m_componentManager;  // Component manager
};

} // namespace Graphics
} // namespace RPGEngine