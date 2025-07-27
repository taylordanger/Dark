#pragma once

#include "AudioManager.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace RPGEngine {
namespace Audio {

/**
 * Sound effect category
 */
enum class SoundCategory {
    UI,             // User interface sounds
    Ambient,        // Ambient environment sounds
    Action,         // Player action sounds
    Combat,         // Combat sounds
    Environment,    // Environmental sounds
    Voice,          // Voice/dialogue sounds
    Music           // Musical sound effects
};

/**
 * Sound effect properties
 */
struct SoundEffectProperties {
    float volume = 1.0f;
    float pitch = 1.0f;
    float x = 0.0f, y = 0.0f, z = 0.0f;        // 3D position
    float velocityX = 0.0f, velocityY = 0.0f, velocityZ = 0.0f; // 3D velocity
    float minDistance = 1.0f;                   // Minimum distance for 3D falloff
    float maxDistance = 100.0f;                 // Maximum distance for 3D falloff
    bool loop = false;
    bool is3D = false;                          // Whether to use 3D positioning
    SoundCategory category = SoundCategory::Action;
    float priority = 1.0f;                      // Higher priority sounds can interrupt lower priority
    
    SoundEffectProperties() = default;
    
    // 2D sound constructor
    SoundEffectProperties(float vol, float p = 1.0f, bool looping = false, 
                         SoundCategory cat = SoundCategory::Action, float prio = 1.0f)
        : volume(vol), pitch(p), loop(looping), is3D(false), category(cat), priority(prio) {}
    
    // 3D sound constructor
    SoundEffectProperties(float vol, float p, float posX, float posY, float posZ,
                         bool looping = false, SoundCategory cat = SoundCategory::Action, float prio = 1.0f)
        : volume(vol), pitch(p), x(posX), y(posY), z(posZ), loop(looping), is3D(true), category(cat), priority(prio) {}
};

/**
 * Active sound effect
 */
struct ActiveSoundEffect {
    std::shared_ptr<AudioSource> source;
    SoundEffectProperties properties;
    std::string resourceId;
    float startTime;
    bool isOneShot;
    
    ActiveSoundEffect(std::shared_ptr<AudioSource> src, const SoundEffectProperties& props, 
                     const std::string& id, float time, bool oneshot = true)
        : source(src), properties(props), resourceId(id), startTime(time), isOneShot(oneshot) {}
};

/**
 * Sound effect manager
 * Manages sound effects with 3D positioning and categories
 */
class SoundEffectManager {
public:
    /**
     * Constructor
     * @param audioManager Audio manager
     */
    explicit SoundEffectManager(std::shared_ptr<AudioManager> audioManager);
    
    /**
     * Destructor
     */
    ~SoundEffectManager();
    
    /**
     * Update the sound effect manager
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
    
    /**
     * Play sound effect
     * @param resourceId Resource ID
     * @param properties Sound properties
     * @return Sound effect ID for tracking, or 0 if failed
     */
    uint32_t playSound(const std::string& resourceId, const SoundEffectProperties& properties = SoundEffectProperties());
    
    /**
     * Play 2D sound effect
     * @param resourceId Resource ID
     * @param volume Volume (0.0 to 1.0)
     * @param pitch Pitch (0.5 to 2.0)
     * @param loop Whether to loop
     * @param category Sound category
     * @return Sound effect ID for tracking, or 0 if failed
     */
    uint32_t play2DSound(const std::string& resourceId, float volume = 1.0f, float pitch = 1.0f, 
                        bool loop = false, SoundCategory category = SoundCategory::Action);
    
    /**
     * Play 3D sound effect
     * @param resourceId Resource ID
     * @param x X position
     * @param y Y position
     * @param z Z position
     * @param volume Volume (0.0 to 1.0)
     * @param pitch Pitch (0.5 to 2.0)
     * @param loop Whether to loop
     * @param category Sound category
     * @return Sound effect ID for tracking, or 0 if failed
     */
    uint32_t play3DSound(const std::string& resourceId, float x, float y, float z,
                        float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                        SoundCategory category = SoundCategory::Action);
    
    /**
     * Stop sound effect
     * @param soundId Sound effect ID
     * @param fadeOutDuration Fade out duration in seconds (0 for immediate)
     */
    void stopSound(uint32_t soundId, float fadeOutDuration = 0.0f);
    
    /**
     * Stop all sounds in category
     * @param category Sound category
     * @param fadeOutDuration Fade out duration in seconds (0 for immediate)
     */
    void stopCategory(SoundCategory category, float fadeOutDuration = 0.0f);
    
    /**
     * Stop all sounds
     * @param fadeOutDuration Fade out duration in seconds (0 for immediate)
     */
    void stopAllSounds(float fadeOutDuration = 0.0f);
    
    /**
     * Update sound position (for 3D sounds)
     * @param soundId Sound effect ID
     * @param x X position
     * @param y Y position
     * @param z Z position
     */
    void updateSoundPosition(uint32_t soundId, float x, float y, float z);
    
    /**
     * Update sound velocity (for 3D sounds with Doppler effect)
     * @param soundId Sound effect ID
     * @param velocityX X velocity
     * @param velocityY Y velocity
     * @param velocityZ Z velocity
     */
    void updateSoundVelocity(uint32_t soundId, float velocityX, float velocityY, float velocityZ);
    
    /**
     * Set category volume
     * @param category Sound category
     * @param volume Volume (0.0 to 1.0)
     */
    void setCategoryVolume(SoundCategory category, float volume);
    
    /**
     * Get category volume
     * @param category Sound category
     * @return Volume (0.0 to 1.0)
     */
    float getCategoryVolume(SoundCategory category) const;
    
    /**
     * Set category enabled state
     * @param category Sound category
     * @param enabled Whether category is enabled
     */
    void setCategoryEnabled(SoundCategory category, bool enabled);
    
    /**
     * Check if category is enabled
     * @param category Sound category
     * @return true if category is enabled
     */
    bool isCategoryEnabled(SoundCategory category) const;
    
    /**
     * Set maximum concurrent sounds per category
     * @param category Sound category
     * @param maxSounds Maximum concurrent sounds
     */
    void setMaxConcurrentSounds(SoundCategory category, int maxSounds);
    
    /**
     * Get number of active sounds in category
     * @param category Sound category
     * @return Number of active sounds
     */
    int getActiveSoundCount(SoundCategory category) const;
    
    /**
     * Get total number of active sounds
     * @return Total number of active sounds
     */
    int getTotalActiveSounds() const { return static_cast<int>(m_activeSounds.size()); }
    
    /**
     * Set sound completion callback
     * @param callback Callback function (soundId, resourceId)
     */
    void setSoundCompletionCallback(std::function<void(uint32_t, const std::string&)> callback) {
        m_completionCallback = callback;
    }
    
    /**
     * Check if sound is playing
     * @param soundId Sound effect ID
     * @return true if sound is playing
     */
    bool isSoundPlaying(uint32_t soundId) const;
    
private:
    /**
     * Generate unique sound ID
     * @return Unique sound ID
     */
    uint32_t generateSoundId();
    
    /**
     * Cleanup finished sounds
     */
    void cleanupFinishedSounds();
    
    /**
     * Apply 3D positioning to sound
     * @param source Audio source
     * @param properties Sound properties
     */
    void apply3DPositioning(std::shared_ptr<AudioSource> source, const SoundEffectProperties& properties);
    
    /**
     * Calculate 3D volume based on distance
     * @param properties Sound properties
     * @param listenerPos Listener position
     * @return Calculated volume multiplier
     */
    float calculate3DVolume(const SoundEffectProperties& properties, const AudioListener& listenerPos);
    
    std::shared_ptr<AudioManager> m_audioManager;
    
    // Active sounds
    std::unordered_map<uint32_t, ActiveSoundEffect> m_activeSounds;
    uint32_t m_nextSoundId;
    
    // Category settings
    std::unordered_map<SoundCategory, float> m_categoryVolumes;
    std::unordered_map<SoundCategory, bool> m_categoryEnabled;
    std::unordered_map<SoundCategory, int> m_maxConcurrentSounds;
    
    // Callbacks
    std::function<void(uint32_t, const std::string&)> m_completionCallback;
    
    // Timing
    float m_totalTime;
};

} // namespace Audio
} // namespace RPGEngine