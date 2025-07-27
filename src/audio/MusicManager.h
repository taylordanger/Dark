#pragma once

#include "AudioManager.h"
#include <string>
#include <memory>
#include <queue>
#include <functional>

namespace RPGEngine {
namespace Audio {

/**
 * Music transition type
 */
enum class MusicTransitionType {
    Immediate,      // Immediate switch
    Crossfade,      // Crossfade between tracks
    FadeOut,        // Fade out current, then play new
    FadeIn          // Stop current, fade in new
};

/**
 * Music state
 */
enum class MusicState {
    Stopped,
    Playing,
    Paused,
    FadingIn,
    FadingOut,
    Crossfading
};

/**
 * Music track info
 */
struct MusicTrack {
    std::string resourceId;
    float volume = 1.0f;
    bool loop = true;
    float fadeInDuration = 0.0f;
    float fadeOutDuration = 0.0f;
    
    MusicTrack() = default;
    MusicTrack(const std::string& id, float vol = 1.0f, bool looping = true)
        : resourceId(id), volume(vol), loop(looping) {}
};

/**
 * Music queue entry
 */
struct MusicQueueEntry {
    MusicTrack track;
    MusicTransitionType transitionType;
    float transitionDuration;
    std::function<void()> onComplete;
    
    MusicQueueEntry(const MusicTrack& t, MusicTransitionType type = MusicTransitionType::Crossfade, 
                   float duration = 2.0f, std::function<void()> callback = nullptr)
        : track(t), transitionType(type), transitionDuration(duration), onComplete(callback) {}
};

/**
 * Music manager
 * Handles background music with advanced features like crossfading
 */
class MusicManager {
public:
    /**
     * Constructor
     * @param audioManager Audio manager
     */
    explicit MusicManager(std::shared_ptr<AudioManager> audioManager);
    
    /**
     * Destructor
     */
    ~MusicManager();
    
    /**
     * Update the music manager
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
    
    /**
     * Play music track
     * @param track Music track
     * @param transitionType Transition type
     * @param transitionDuration Transition duration in seconds
     * @param onComplete Callback when transition completes
     */
    void playMusic(const MusicTrack& track, 
                  MusicTransitionType transitionType = MusicTransitionType::Crossfade,
                  float transitionDuration = 2.0f,
                  std::function<void()> onComplete = nullptr);
    
    /**
     * Play music by resource ID
     * @param resourceId Resource ID
     * @param volume Volume (0.0 to 1.0)
     * @param loop Whether to loop
     * @param transitionType Transition type
     * @param transitionDuration Transition duration in seconds
     */
    void playMusic(const std::string& resourceId, float volume = 1.0f, bool loop = true,
                  MusicTransitionType transitionType = MusicTransitionType::Crossfade,
                  float transitionDuration = 2.0f);
    
    /**
     * Stop music
     * @param fadeOutDuration Fade out duration in seconds
     */
    void stopMusic(float fadeOutDuration = 1.0f);
    
    /**
     * Pause music
     * @param fadeOutDuration Fade out duration in seconds (0 for immediate)
     */
    void pauseMusic(float fadeOutDuration = 0.0f);
    
    /**
     * Resume music
     * @param fadeInDuration Fade in duration in seconds (0 for immediate)
     */
    void resumeMusic(float fadeInDuration = 0.0f);
    
    /**
     * Queue music track
     * @param track Music track
     * @param transitionType Transition type
     * @param transitionDuration Transition duration in seconds
     * @param onComplete Callback when transition completes
     */
    void queueMusic(const MusicTrack& track,
                   MusicTransitionType transitionType = MusicTransitionType::Crossfade,
                   float transitionDuration = 2.0f,
                   std::function<void()> onComplete = nullptr);
    
    /**
     * Clear music queue
     */
    void clearQueue();
    
    /**
     * Skip to next queued track
     */
    void skipToNext();
    
    /**
     * Get current music state
     * @return Music state
     */
    MusicState getState() const { return m_state; }
    
    /**
     * Get current track info
     * @return Current track, or nullptr if no track is playing
     */
    const MusicTrack* getCurrentTrack() const;
    
    /**
     * Get current volume
     * @return Current volume (0.0 to 1.0)
     */
    float getCurrentVolume() const { return m_currentVolume; }
    
    /**
     * Set music volume
     * @param volume Volume (0.0 to 1.0)
     * @param fadeDuration Fade duration in seconds (0 for immediate)
     */
    void setVolume(float volume, float fadeDuration = 0.0f);
    
    /**
     * Get queue size
     * @return Number of tracks in queue
     */
    size_t getQueueSize() const { return m_musicQueue.size(); }
    
    /**
     * Check if music is playing
     * @return true if music is playing
     */
    bool isPlaying() const { return m_state == MusicState::Playing || m_state == MusicState::Crossfading; }
    
    /**
     * Check if music is paused
     * @return true if music is paused
     */
    bool isPaused() const { return m_state == MusicState::Paused; }
    
    /**
     * Set crossfade curve
     * @param curve Crossfade curve function (0.0 to 1.0 input, 0.0 to 1.0 output)
     */
    void setCrossfadeCurve(std::function<float(float)> curve) { m_crossfadeCurve = curve; }
    
private:
    /**
     * Process next item in music queue
     */
    void processQueue();
    
    /**
     * Update crossfading
     * @param deltaTime Time since last update
     */
    void updateCrossfading(float deltaTime);
    
    /**
     * Update fading
     * @param deltaTime Time since last update
     */
    void updateFading(float deltaTime);
    
    /**
     * Apply crossfade curve
     * @param t Normalized time (0.0 to 1.0)
     * @return Curved value (0.0 to 1.0)
     */
    float applyCrossfadeCurve(float t);
    
    std::shared_ptr<AudioManager> m_audioManager;
    
    // Current music state
    MusicState m_state;
    MusicTrack m_currentTrack;
    std::shared_ptr<AudioSource> m_currentSource;
    float m_currentVolume;
    
    // Crossfading state
    MusicTrack m_nextTrack;
    std::shared_ptr<AudioSource> m_nextSource;
    float m_crossfadeTime;
    float m_crossfadeDuration;
    std::function<void()> m_crossfadeCallback;
    
    // Fading state
    float m_fadeTime;
    float m_fadeDuration;
    float m_fadeStartVolume;
    float m_fadeTargetVolume;
    bool m_fadeToStop;
    bool m_fadeToResume;
    
    // Music queue
    std::queue<MusicQueueEntry> m_musicQueue;
    
    // Crossfade curve function
    std::function<float(float)> m_crossfadeCurve;
};

} // namespace Audio
} // namespace RPGEngine