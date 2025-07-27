#pragma once

#include "../components/Component.h"
#include "Sprite.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace RPGEngine {
namespace Graphics {

// Forward declarations
class AnimationComponent;

/**
 * Animation frame
 * Represents a single frame in an animation
 */
struct AnimationFrame {
    int textureX;          // X position in texture
    int textureY;          // Y position in texture
    int width;             // Frame width
    int height;            // Frame height
    float duration;        // Frame duration in seconds
    
    AnimationFrame(int x, int y, int w, int h, float d)
        : textureX(x), textureY(y), width(w), height(h), duration(d) {}
};

/**
 * Animation sequence
 * Represents a sequence of animation frames
 */
class AnimationSequence {
public:
    /**
     * Constructor
     * @param name Animation name
     * @param loop Whether the animation should loop
     */
    AnimationSequence(const std::string& name, bool loop = true)
        : m_name(name), m_loop(loop), m_totalDuration(0.0f) {}
    
    /**
     * Add a frame to the animation
     * @param frame Animation frame
     */
    void addFrame(const AnimationFrame& frame) {
        m_frames.push_back(frame);
        m_totalDuration += frame.duration;
    }
    
    /**
     * Get the animation name
     * @return Animation name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * Get whether the animation should loop
     * @return true if the animation should loop
     */
    bool isLooping() const { return m_loop; }
    
    /**
     * Set whether the animation should loop
     * @param loop Whether the animation should loop
     */
    void setLooping(bool loop) { m_loop = loop; }
    
    /**
     * Get the total duration of the animation
     * @return Total duration in seconds
     */
    float getTotalDuration() const { return m_totalDuration; }
    
    /**
     * Get the number of frames
     * @return Number of frames
     */
    size_t getFrameCount() const { return m_frames.size(); }
    
    /**
     * Get a frame by index
     * @param index Frame index
     * @return Animation frame
     */
    const AnimationFrame& getFrame(size_t index) const { return m_frames[index]; }
    
    /**
     * Get all frames
     * @return Vector of animation frames
     */
    const std::vector<AnimationFrame>& getFrames() const { return m_frames; }
    
private:
    std::string m_name;                // Animation name
    bool m_loop;                       // Whether the animation should loop
    float m_totalDuration;             // Total duration in seconds
    std::vector<AnimationFrame> m_frames; // Animation frames
};

/**
 * Animation event type
 */
enum class AnimationEventType {
    Started,        // Animation started
    Finished,       // Animation finished
    FrameChanged,   // Frame changed
    LoopCompleted   // Loop completed
};

/**
 * Animation event
 */
struct AnimationEvent {
    AnimationEventType type;           // Event type
    std::string animationName;         // Animation name
    int frameIndex;                    // Frame index
    
    AnimationEvent(AnimationEventType t, const std::string& name, int index = 0)
        : type(t), animationName(name), frameIndex(index) {}
};

/**
 * Animation state for state machine
 */
enum class AnimationState {
    Idle,
    Playing,
    Paused,
    Finished
};

/**
 * Animation blend mode
 */
enum class AnimationBlendMode {
    Replace,    // Replace current animation immediately
    Crossfade,  // Crossfade between animations
    Additive    // Add animation on top of current
};

/**
 * Animation trigger type
 */
enum class AnimationTriggerType {
    Manual,     // Manually triggered
    OnStart,    // Triggered when entity starts moving/acting
    OnStop,     // Triggered when entity stops
    OnCollision,// Triggered on collision
    OnDamage,   // Triggered when taking damage
    OnAttack,   // Triggered when attacking
    Custom      // Custom trigger condition
};

/**
 * Animation trigger
 */
struct AnimationTrigger {
    std::string animationName;
    AnimationTriggerType triggerType;
    std::function<bool()> condition;
    int priority;
    
    AnimationTrigger(const std::string& name, AnimationTriggerType type, 
                    std::function<bool()> cond = nullptr, int prio = 0)
        : animationName(name), triggerType(type), condition(cond), priority(prio) {}
};

/**
 * Animation transition condition
 */
struct AnimationTransition {
    std::string fromAnimation;
    std::string toAnimation;
    std::function<bool()> condition;
    bool autoTransition;
    AnimationBlendMode blendMode;
    float blendDuration;
    
    AnimationTransition(const std::string& from, const std::string& to, 
                       std::function<bool()> cond, bool autoTrans = false,
                       AnimationBlendMode blend = AnimationBlendMode::Replace,
                       float blendTime = 0.0f)
        : fromAnimation(from), toAnimation(to), condition(cond), autoTransition(autoTrans),
          blendMode(blend), blendDuration(blendTime) {}
};

/**
 * Animation component
 * Handles sprite animations with state machine support
 */
class AnimationComponent : public RPGEngine::Components::Component<AnimationComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     */
    AnimationComponent(EntityId entityId);
    
    /**
     * Destructor
     */
    ~AnimationComponent();
    
    /**
     * Update the animation
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
    
    /**
     * Add an animation sequence
     * @param animation Animation sequence
     * @return true if the animation was added
     */
    bool addAnimation(std::shared_ptr<AnimationSequence> animation);
    
    /**
     * Get an animation by name
     * @param name Animation name
     * @return Animation sequence, or nullptr if not found
     */
    std::shared_ptr<AnimationSequence> getAnimation(const std::string& name) const;
    
    /**
     * Remove an animation
     * @param name Animation name
     * @return true if the animation was removed
     */
    bool removeAnimation(const std::string& name);
    
    /**
     * Play an animation
     * @param name Animation name
     * @param reset Whether to reset the animation if it's already playing
     * @return true if the animation was started
     */
    bool play(const std::string& name, bool reset = true);
    
    /**
     * Stop the current animation
     */
    void stop();
    
    /**
     * Pause the current animation
     */
    void pause();
    
    /**
     * Resume the current animation
     */
    void resume();
    
    /**
     * Check if an animation is playing
     * @return true if an animation is playing
     */
    bool isPlaying() const { return m_playing; }
    
    /**
     * Check if an animation is paused
     * @return true if an animation is paused
     */
    bool isPaused() const { return m_paused; }
    
    /**
     * Get the current animation name
     * @return Current animation name, or empty string if no animation is playing
     */
    const std::string& getCurrentAnimationName() const { return m_currentAnimationName; }
    
    /**
     * Get the current animation
     * @return Current animation, or nullptr if no animation is playing
     */
    std::shared_ptr<AnimationSequence> getCurrentAnimation() const;
    
    /**
     * Get the current frame index
     * @return Current frame index
     */
    int getCurrentFrameIndex() const { return m_currentFrameIndex; }
    
    /**
     * Get the current frame
     * @return Current frame, or nullptr if no animation is playing
     */
    const AnimationFrame* getCurrentFrame() const;
    
    /**
     * Set the sprite to animate
     * @param sprite Sprite to animate
     */
    void setSprite(std::shared_ptr<Sprite> sprite) { m_sprite = sprite; }
    
    /**
     * Get the sprite being animated
     * @return Sprite being animated
     */
    std::shared_ptr<Sprite> getSprite() const { return m_sprite; }
    
    /**
     * Register an event callback
     * @param callback Function to call when an animation event occurs
     * @return Callback ID
     */
    int registerEventCallback(const std::function<void(const AnimationEvent&)>& callback);
    
    /**
     * Unregister an event callback
     * @param callbackId Callback ID
     * @return true if the callback was unregistered
     */
    bool unregisterEventCallback(int callbackId);
    
    /**
     * Set the playback speed
     * @param speed Playback speed (1.0 = normal speed)
     */
    void setSpeed(float speed) { m_speed = speed; }
    
    /**
     * Get the playback speed
     * @return Playback speed
     */
    float getSpeed() const { return m_speed; }
    
    /**
     * Add an animation transition
     * @param transition Animation transition
     */
    void addTransition(const AnimationTransition& transition);
    
    /**
     * Remove an animation transition
     * @param fromAnimation Source animation name
     * @param toAnimation Target animation name
     * @return true if the transition was removed
     */
    bool removeTransition(const std::string& fromAnimation, const std::string& toAnimation);
    
    /**
     * Check for and execute animation transitions
     */
    void checkTransitions();
    
    /**
     * Get the current animation state
     * @return Current animation state
     */
    AnimationState getAnimationState() const { return m_animationState; }
    
    /**
     * Set default animation to play when no other animation is active
     * @param animationName Default animation name
     */
    void setDefaultAnimation(const std::string& animationName) { m_defaultAnimation = animationName; }
    
    /**
     * Get the default animation name
     * @return Default animation name
     */
    const std::string& getDefaultAnimation() const { return m_defaultAnimation; }
    
    /**
     * Add an animation trigger
     * @param trigger Animation trigger
     */
    void addTrigger(const AnimationTrigger& trigger);
    
    /**
     * Remove an animation trigger
     * @param animationName Animation name
     * @param triggerType Trigger type
     * @return true if the trigger was removed
     */
    bool removeTrigger(const std::string& animationName, AnimationTriggerType triggerType);
    
    /**
     * Check and execute animation triggers
     */
    void checkTriggers();
    
    /**
     * Play animation with blending
     * @param name Animation name
     * @param blendMode Blend mode
     * @param blendDuration Blend duration in seconds
     * @param reset Whether to reset the animation if it's already playing
     * @return true if the animation was started
     */
    bool playWithBlend(const std::string& name, AnimationBlendMode blendMode, 
                      float blendDuration = 0.3f, bool reset = true);
    
    /**
     * Get current blend state
     * @return true if currently blending between animations
     */
    bool isBlending() const { return m_isBlending; }
    
    /**
     * Get blend progress (0.0 to 1.0)
     * @return Blend progress
     */
    float getBlendProgress() const { return m_blendProgress; }
    
    /**
     * Set animation priority (higher priority animations can interrupt lower priority ones)
     * @param priority Priority value
     */
    void setCurrentAnimationPriority(int priority) { m_currentAnimationPriority = priority; }
    
    /**
     * Get current animation priority
     * @return Current animation priority
     */
    int getCurrentAnimationPriority() const { return m_currentAnimationPriority; }
    
    // Inherited from Component<AnimationComponent>
    
private:
    /**
     * Fire an animation event
     * @param type Event type
     * @param animationName Animation name
     * @param frameIndex Frame index
     */
    void fireEvent(AnimationEventType type, const std::string& animationName, int frameIndex = 0);
    
    /**
     * Update the sprite with the current frame
     */
    void updateSprite();
    
    std::unordered_map<std::string, std::shared_ptr<AnimationSequence>> m_animations; // Animations
    std::string m_currentAnimationName;                                // Current animation name
    int m_currentFrameIndex;                                           // Current frame index
    float m_currentFrameTime;                                          // Current frame time
    bool m_playing;                                                    // Whether an animation is playing
    bool m_paused;                                                     // Whether the animation is paused
    float m_speed;                                                     // Playback speed
    std::shared_ptr<Sprite> m_sprite;                                  // Sprite to animate
    std::unordered_map<int, std::function<void(const AnimationEvent&)>> m_eventCallbacks; // Event callbacks
    int m_nextCallbackId;                                              // Next callback ID
    
    // Animation state machine
    AnimationState m_animationState;                                   // Current animation state
    std::vector<AnimationTransition> m_transitions;                    // Animation transitions
    std::string m_defaultAnimation;                                    // Default animation to play
    
    // Animation triggers
    std::vector<AnimationTrigger> m_triggers;                          // Animation triggers
    
    // Animation blending
    bool m_isBlending;                                                 // Whether currently blending
    float m_blendProgress;                                             // Blend progress (0.0 to 1.0)
    float m_blendDuration;                                             // Total blend duration
    float m_blendTime;                                                 // Current blend time
    AnimationBlendMode m_blendMode;                                    // Current blend mode
    std::string m_previousAnimationName;                               // Previous animation for blending
    int m_previousFrameIndex;                                          // Previous frame index for blending
    float m_previousFrameTime;                                         // Previous frame time for blending
    
    // Animation priority
    int m_currentAnimationPriority;                                    // Current animation priority
};

} // namespace Graphics
} // namespace RPGEngine