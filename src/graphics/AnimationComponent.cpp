#include "AnimationComponent.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace RPGEngine {
namespace Graphics {

AnimationComponent::AnimationComponent(EntityId entityId)
    : RPGEngine::Components::Component<AnimationComponent>(entityId)
    , m_currentFrameIndex(0)
    , m_currentFrameTime(0.0f)
    , m_playing(false)
    , m_paused(false)
    , m_speed(1.0f)
    , m_nextCallbackId(1)
    , m_animationState(AnimationState::Idle)
    , m_isBlending(false)
    , m_blendProgress(0.0f)
    , m_blendDuration(0.0f)
    , m_blendTime(0.0f)
    , m_blendMode(AnimationBlendMode::Replace)
    , m_previousFrameIndex(0)
    , m_previousFrameTime(0.0f)
    , m_currentAnimationPriority(0)
{
}

AnimationComponent::~AnimationComponent() {
    // Nothing to do here
}

void AnimationComponent::update(float deltaTime) {
    // Check for animation triggers
    checkTriggers();
    
    // Check for animation transitions
    checkTransitions();
    
    // Update blending if active
    if (m_isBlending) {
        m_blendTime += deltaTime;
        m_blendProgress = std::min(1.0f, m_blendTime / m_blendDuration);
        
        if (m_blendProgress >= 1.0f) {
            // Blending complete
            m_isBlending = false;
            m_blendProgress = 0.0f;
            m_blendTime = 0.0f;
            m_previousAnimationName = "";
        }
    }
    
    // Check if we're playing and not paused
    if (!m_playing || m_paused) {
        // If we're idle and have a default animation, play it
        if (m_animationState == AnimationState::Idle && !m_defaultAnimation.empty()) {
            play(m_defaultAnimation, false);
        }
        return;
    }
    
    // Update animation state
    m_animationState = AnimationState::Playing;
    
    // Get current animation
    auto animation = getCurrentAnimation();
    if (!animation || animation->getFrameCount() == 0) {
        return;
    }
    
    // Update frame time
    m_currentFrameTime += deltaTime * m_speed;
    
    // Get current frame
    const AnimationFrame* frame = getCurrentFrame();
    if (!frame) {
        return;
    }
    
    // Check if we need to advance to the next frame
    if (m_currentFrameTime >= frame->duration) {
        // Reset frame time
        m_currentFrameTime -= frame->duration;
        
        // Move to next frame
        int prevFrameIndex = m_currentFrameIndex;
        m_currentFrameIndex++;
        
        // Check if we've reached the end of the animation
        if (m_currentFrameIndex >= static_cast<int>(animation->getFrameCount())) {
            // Check if we should loop
            if (animation->isLooping()) {
                // Loop back to the beginning
                m_currentFrameIndex = 0;
                
                // Fire loop completed event
                fireEvent(AnimationEventType::LoopCompleted, m_currentAnimationName);
            } else {
                // Stop at the last frame
                m_currentFrameIndex = static_cast<int>(animation->getFrameCount()) - 1;
                m_playing = false;
                m_animationState = AnimationState::Finished;
                
                // Fire finished event
                fireEvent(AnimationEventType::Finished, m_currentAnimationName);
            }
        }
        
        // Fire frame changed event if the frame actually changed
        if (prevFrameIndex != m_currentFrameIndex) {
            fireEvent(AnimationEventType::FrameChanged, m_currentAnimationName, m_currentFrameIndex);
            
            // Update sprite
            updateSprite();
        }
    }
}

bool AnimationComponent::addAnimation(std::shared_ptr<AnimationSequence> animation) {
    if (!animation) {
        return false;
    }
    
    const std::string& name = animation->getName();
    if (name.empty()) {
        return false;
    }
    
    // Check if animation already exists
    if (m_animations.find(name) != m_animations.end()) {
        return false;
    }
    
    m_animations[name] = animation;
    return true;
}

std::shared_ptr<AnimationSequence> AnimationComponent::getAnimation(const std::string& name) const {
    auto it = m_animations.find(name);
    if (it != m_animations.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool AnimationComponent::removeAnimation(const std::string& name) {
    auto it = m_animations.find(name);
    if (it != m_animations.end()) {
        // Check if this is the current animation
        if (m_currentAnimationName == name) {
            stop();
        }
        
        m_animations.erase(it);
        return true;
    }
    
    return false;
}

bool AnimationComponent::play(const std::string& name, bool reset) {
    // Check if animation exists
    auto animation = getAnimation(name);
    if (!animation) {
        return false;
    }
    
    // Check if we're already playing this animation
    if (m_playing && m_currentAnimationName == name && !reset) {
        // Just unpause if paused
        if (m_paused) {
            resume();
        }
        return true;
    }
    
    // Stop current animation
    stop();
    
    // Set new animation
    m_currentAnimationName = name;
    m_currentFrameIndex = 0;
    m_currentFrameTime = 0.0f;
    m_playing = true;
    m_paused = false;
    m_animationState = AnimationState::Playing;
    
    // Update sprite
    updateSprite();
    
    // Fire started event
    fireEvent(AnimationEventType::Started, m_currentAnimationName);
    
    return true;
}

void AnimationComponent::stop() {
    if (!m_playing) {
        return;
    }
    
    // Reset state
    m_playing = false;
    m_paused = false;
    m_animationState = AnimationState::Idle;
    
    // Fire finished event
    fireEvent(AnimationEventType::Finished, m_currentAnimationName);
    
    // Clear current animation
    m_currentAnimationName = "";
    m_currentFrameIndex = 0;
    m_currentFrameTime = 0.0f;
}

void AnimationComponent::pause() {
    if (m_playing && !m_paused) {
        m_paused = true;
        m_animationState = AnimationState::Paused;
    }
}

void AnimationComponent::resume() {
    if (m_playing && m_paused) {
        m_paused = false;
        m_animationState = AnimationState::Playing;
    }
}

std::shared_ptr<AnimationSequence> AnimationComponent::getCurrentAnimation() const {
    if (m_currentAnimationName.empty()) {
        return nullptr;
    }
    
    return getAnimation(m_currentAnimationName);
}

const AnimationFrame* AnimationComponent::getCurrentFrame() const {
    auto animation = getCurrentAnimation();
    if (!animation || m_currentFrameIndex < 0 || m_currentFrameIndex >= static_cast<int>(animation->getFrameCount())) {
        return nullptr;
    }
    
    return &animation->getFrame(m_currentFrameIndex);
}

int AnimationComponent::registerEventCallback(const std::function<void(const AnimationEvent&)>& callback) {
    if (!callback) {
        return -1;
    }
    
    int callbackId = m_nextCallbackId++;
    m_eventCallbacks[callbackId] = callback;
    return callbackId;
}

bool AnimationComponent::unregisterEventCallback(int callbackId) {
    auto it = m_eventCallbacks.find(callbackId);
    if (it != m_eventCallbacks.end()) {
        m_eventCallbacks.erase(it);
        return true;
    }
    
    return false;
}

void AnimationComponent::fireEvent(AnimationEventType type, const std::string& animationName, int frameIndex) {
    AnimationEvent event(type, animationName, frameIndex);
    
    for (const auto& pair : m_eventCallbacks) {
        pair.second(event);
    }
}

void AnimationComponent::updateSprite() {
    if (!m_sprite) {
        return;
    }
    
    const AnimationFrame* frame = getCurrentFrame();
    if (!frame) {
        return;
    }
    
    // Update sprite texture coordinates
    // TODO: Re-enable when Sprite class linking is fixed
    // Rect textureRect(static_cast<float>(frame->textureX), static_cast<float>(frame->textureY), 
    //                  static_cast<float>(frame->width), static_cast<float>(frame->height));
    // m_sprite->setTextureRect(textureRect);
    
    // For now, just log the frame update
    std::cout << "Updating sprite to frame: " << frame->textureX << "," << frame->textureY 
              << " size: " << frame->width << "x" << frame->height << std::endl;
}

void AnimationComponent::addTransition(const AnimationTransition& transition) {
    m_transitions.push_back(transition);
}

bool AnimationComponent::removeTransition(const std::string& fromAnimation, const std::string& toAnimation) {
    auto it = std::find_if(m_transitions.begin(), m_transitions.end(),
        [&](const AnimationTransition& transition) {
            return transition.fromAnimation == fromAnimation && transition.toAnimation == toAnimation;
        });
    
    if (it != m_transitions.end()) {
        m_transitions.erase(it);
        return true;
    }
    
    return false;
}

void AnimationComponent::checkTransitions() {
    if (m_currentAnimationName.empty()) {
        return;
    }
    
    for (const auto& transition : m_transitions) {
        // Check if this transition applies to the current animation
        if (transition.fromAnimation == m_currentAnimationName) {
            // Check if the condition is met
            if (transition.condition && transition.condition()) {
                // Execute the transition with blending if specified
                if (transition.blendMode != AnimationBlendMode::Replace && transition.blendDuration > 0.0f) {
                    playWithBlend(transition.toAnimation, transition.blendMode, transition.blendDuration, true);
                } else {
                    play(transition.toAnimation, true);
                }
                break;
            }
            // Check for auto-transitions when animation finishes
            else if (transition.autoTransition && m_animationState == AnimationState::Finished) {
                if (transition.blendMode != AnimationBlendMode::Replace && transition.blendDuration > 0.0f) {
                    playWithBlend(transition.toAnimation, transition.blendMode, transition.blendDuration, true);
                } else {
                    play(transition.toAnimation, true);
                }
                break;
            }
        }
    }
}

void AnimationComponent::addTrigger(const AnimationTrigger& trigger) {
    m_triggers.push_back(trigger);
}

bool AnimationComponent::removeTrigger(const std::string& animationName, AnimationTriggerType triggerType) {
    auto it = std::find_if(m_triggers.begin(), m_triggers.end(),
        [&](const AnimationTrigger& trigger) {
            return trigger.animationName == animationName && trigger.triggerType == triggerType;
        });
    
    if (it != m_triggers.end()) {
        m_triggers.erase(it);
        return true;
    }
    
    return false;
}

void AnimationComponent::checkTriggers() {
    // Sort triggers by priority (higher priority first)
    std::vector<AnimationTrigger*> activeTriggers;
    
    for (auto& trigger : m_triggers) {
        // Check if trigger condition is met
        if (trigger.condition && trigger.condition()) {
            activeTriggers.push_back(&trigger);
        }
    }
    
    // Sort by priority
    std::sort(activeTriggers.begin(), activeTriggers.end(),
        [](const AnimationTrigger* a, const AnimationTrigger* b) {
            return a->priority > b->priority;
        });
    
    // Execute highest priority trigger
    for (const auto* trigger : activeTriggers) {
        // Only interrupt if the new animation has higher or equal priority
        if (trigger->priority >= m_currentAnimationPriority) {
            play(trigger->animationName, true);
            m_currentAnimationPriority = trigger->priority;
            break;
        }
    }
}

bool AnimationComponent::playWithBlend(const std::string& name, AnimationBlendMode blendMode, 
                                      float blendDuration, bool reset) {
    // Check if animation exists
    auto animation = getAnimation(name);
    if (!animation) {
        return false;
    }
    
    // If we're already playing this animation and not resetting, just continue
    if (m_playing && m_currentAnimationName == name && !reset) {
        return true;
    }
    
    // Store previous animation state for blending
    if (m_playing && blendDuration > 0.0f && blendMode != AnimationBlendMode::Replace) {
        m_previousAnimationName = m_currentAnimationName;
        m_previousFrameIndex = m_currentFrameIndex;
        m_previousFrameTime = m_currentFrameTime;
        
        // Start blending
        m_isBlending = true;
        m_blendProgress = 0.0f;
        m_blendTime = 0.0f;
        m_blendDuration = blendDuration;
        m_blendMode = blendMode;
    }
    
    // Set new animation
    m_currentAnimationName = name;
    m_currentFrameIndex = 0;
    m_currentFrameTime = 0.0f;
    m_playing = true;
    m_paused = false;
    m_animationState = AnimationState::Playing;
    
    // Update sprite
    updateSprite();
    
    // Fire started event
    fireEvent(AnimationEventType::Started, m_currentAnimationName);
    
    return true;
}

} // namespace Graphics
} // namespace RPGEngine