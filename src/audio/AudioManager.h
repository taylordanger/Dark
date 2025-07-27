#pragma once

#include "../resources/ResourceManager.h"
#include "../resources/AudioResource.h"
#include "IAudioDevice.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <functional>

namespace RPGEngine {
namespace Audio {

/**
 * Audio source state
 */
enum class AudioSourceState {
    Initial,    // Initial state
    Playing,    // Currently playing
    Paused,     // Paused
    Stopped     // Stopped
};

/**
 * Audio source
 * Represents a playable audio source
 */
class AudioSource {
public:
    /**
     * Constructor
     * @param resource Audio resource
     */
    AudioSource(std::shared_ptr<Resources::AudioResource> resource);
    
    /**
     * Destructor
     */
    ~AudioSource();
    
    /**
     * Play the audio
     * @param loop Whether to loop the audio
     */
    void play(bool loop = false);
    
    /**
     * Pause the audio
     */
    void pause();
    
    /**
     * Stop the audio
     */
    void stop();
    
    /**
     * Get the audio state
     * @return Audio state
     */
    AudioSourceState getState() const { return m_state; }
    
    /**
     * Check if the audio is playing
     * @return true if the audio is playing
     */
    bool isPlaying() const { return m_state == AudioSourceState::Playing; }
    
    /**
     * Check if the audio is paused
     * @return true if the audio is paused
     */
    bool isPaused() const { return m_state == AudioSourceState::Paused; }
    
    /**
     * Check if the audio is stopped
     * @return true if the audio is stopped
     */
    bool isStopped() const { return m_state == AudioSourceState::Stopped; }
    
    /**
     * Check if the audio is looping
     * @return true if the audio is looping
     */
    bool isLooping() const { return m_looping; }
    
    /**
     * Set whether the audio should loop
     * @param loop Whether to loop the audio
     */
    void setLooping(bool loop) { m_looping = loop; }
    
    /**
     * Get the audio volume
     * @return Volume (0.0 to 1.0)
     */
    float getVolume() const { return m_volume; }
    
    /**
     * Set the audio volume
     * @param volume Volume (0.0 to 1.0)
     */
    void setVolume(float volume);
    
    /**
     * Get the audio pitch
     * @return Pitch (0.5 to 2.0)
     */
    float getPitch() const { return m_pitch; }
    
    /**
     * Set the audio pitch
     * @param pitch Pitch (0.5 to 2.0)
     */
    void setPitch(float pitch);
    
    /**
     * Get the audio position
     * @return Position in seconds
     */
    float getPosition() const;
    
    /**
     * Set the audio position
     * @param position Position in seconds
     */
    void setPosition(float position);
    
    /**
     * Get the audio resource
     * @return Audio resource
     */
    std::shared_ptr<Resources::AudioResource> getResource() const { return m_resource; }
    
    /**
     * Get the source handle
     * @return Source handle
     */
    unsigned int getSourceHandle() const { return m_sourceHandle; }
    
private:
    std::shared_ptr<Resources::AudioResource> m_resource; // Audio resource
    unsigned int m_sourceHandle;                          // OpenAL source handle
    AudioSourceState m_state;                             // Audio state
    bool m_looping;                                       // Whether the audio is looping
    float m_volume;                                       // Volume (0.0 to 1.0)
    float m_pitch;                                        // Pitch (0.5 to 2.0)
};

/**
 * Audio listener
 * Represents the audio listener (usually the player/camera)
 */
struct AudioListener {
    float x = 0.0f, y = 0.0f, z = 0.0f;           // Position
    float forwardX = 0.0f, forwardY = 0.0f, forwardZ = -1.0f; // Forward vector
    float upX = 0.0f, upY = 1.0f, upZ = 0.0f;     // Up vector
    float velocityX = 0.0f, velocityY = 0.0f, velocityZ = 0.0f; // Velocity
};

/**
 * Audio manager
 * Manages audio playback with platform abstraction
 */
class AudioManager {
public:
    /**
     * Constructor
     * @param resourceManager Resource manager
     * @param audioDevice Audio device (optional, will create default if null)
     */
    AudioManager(std::shared_ptr<Resources::ResourceManager> resourceManager,
                std::shared_ptr<IAudioDevice> audioDevice = nullptr);
    
    /**
     * Destructor
     */
    ~AudioManager();
    
    /**
     * Initialize the audio manager
     * @param deviceName Audio device name (nullptr for default)
     * @return true if initialization was successful
     */
    bool initialize(const char* deviceName = nullptr);
    
    /**
     * Shutdown the audio manager
     */
    void shutdown();
    
    /**
     * Update the audio manager
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
    
    /**
     * Play a sound effect
     * @param resourceId Resource ID
     * @param volume Volume (0.0 to 1.0)
     * @param pitch Pitch (0.5 to 2.0)
     * @param loop Whether to loop the sound
     * @return Audio source, or nullptr if the resource was not found
     */
    std::shared_ptr<AudioSource> playSound(const std::string& resourceId, float volume = 1.0f, float pitch = 1.0f, bool loop = false);
    
    /**
     * Play music
     * @param resourceId Resource ID
     * @param volume Volume (0.0 to 1.0)
     * @param loop Whether to loop the music
     * @return Audio source, or nullptr if the resource was not found
     */
    std::shared_ptr<AudioSource> playMusic(const std::string& resourceId, float volume = 1.0f, bool loop = true);
    
    /**
     * Stop all sounds
     */
    void stopAllSounds();
    
    /**
     * Stop all music
     */
    void stopAllMusic();
    
    /**
     * Stop all audio
     */
    void stopAll();
    
    /**
     * Pause all sounds
     */
    void pauseAllSounds();
    
    /**
     * Pause all music
     */
    void pauseAllMusic();
    
    /**
     * Pause all audio
     */
    void pauseAll();
    
    /**
     * Resume all sounds
     */
    void resumeAllSounds();
    
    /**
     * Resume all music
     */
    void resumeAllMusic();
    
    /**
     * Resume all audio
     */
    void resumeAll();
    
    /**
     * Get the master volume
     * @return Master volume (0.0 to 1.0)
     */
    float getMasterVolume() const { return m_masterVolume; }
    
    /**
     * Set the master volume
     * @param volume Master volume (0.0 to 1.0)
     */
    void setMasterVolume(float volume);
    
    /**
     * Get the sound volume
     * @return Sound volume (0.0 to 1.0)
     */
    float getSoundVolume() const { return m_soundVolume; }
    
    /**
     * Set the sound volume
     * @param volume Sound volume (0.0 to 1.0)
     */
    void setSoundVolume(float volume);
    
    /**
     * Get the music volume
     * @return Music volume (0.0 to 1.0)
     */
    float getMusicVolume() const { return m_musicVolume; }
    
    /**
     * Set the music volume
     * @param volume Music volume (0.0 to 1.0)
     */
    void setMusicVolume(float volume);
    
    /**
     * Check if audio is muted
     * @return true if audio is muted
     */
    bool isMuted() const { return m_muted; }
    
    /**
     * Set whether audio is muted
     * @param muted Whether audio is muted
     */
    void setMuted(bool muted);
    
    /**
     * Get the resource manager
     * @return Resource manager
     */
    std::shared_ptr<Resources::ResourceManager> getResourceManager() const { return m_resourceManager; }
    
    /**
     * Get the audio device
     * @return Audio device
     */
    std::shared_ptr<IAudioDevice> getAudioDevice() const { return m_audioDevice; }
    
    /**
     * Get available audio devices
     * @return List of available device names
     */
    std::vector<std::string> getAvailableDevices() const;
    
    /**
     * Get current audio device name
     * @return Current device name
     */
    std::string getCurrentDevice() const;
    
    /**
     * Get audio device capabilities
     * @return Device capabilities
     */
    AudioDeviceCapabilities getDeviceCapabilities() const;
    
    /**
     * Set audio listener
     * @param listener Audio listener
     */
    void setListener(const AudioListener& listener);
    
    /**
     * Get audio listener
     * @return Audio listener
     */
    const AudioListener& getListener() const { return m_listener; }
    
    /**
     * Play 3D positioned sound
     * @param resourceId Resource ID
     * @param x X position
     * @param y Y position
     * @param z Z position
     * @param volume Volume (0.0 to 1.0)
     * @param pitch Pitch (0.5 to 2.0)
     * @param loop Whether to loop the sound
     * @return Audio source, or nullptr if the resource was not found
     */
    std::shared_ptr<AudioSource> play3DSound(const std::string& resourceId, 
                                            float x, float y, float z,
                                            float volume = 1.0f, float pitch = 1.0f, bool loop = false);
    
    /**
     * Set audio event callback
     * @param callback Callback function for audio events
     */
    void setEventCallback(std::function<void(const std::string&, const std::string&)> callback) {
        m_eventCallback = callback;
    }
    
private:
    /**
     * Clean up unused audio sources
     */
    void cleanupSources();
    
    /**
     * Fire audio event
     * @param eventType Event type
     * @param details Event details
     */
    void fireEvent(const std::string& eventType, const std::string& details);
    
    // Resource manager
    std::shared_ptr<Resources::ResourceManager> m_resourceManager;
    
    // Audio device
    std::shared_ptr<IAudioDevice> m_audioDevice;
    bool m_ownsDevice;
    
    // Audio sources
    std::vector<std::shared_ptr<AudioSource>> m_soundSources;
    std::vector<std::shared_ptr<AudioSource>> m_musicSources;
    
    // Volume settings
    float m_masterVolume;
    float m_soundVolume;
    float m_musicVolume;
    bool m_muted;
    
    // Audio listener
    AudioListener m_listener;
    
    // Event callback
    std::function<void(const std::string&, const std::string&)> m_eventCallback;
    
    // Mutex for thread safety
    mutable std::mutex m_mutex;
};

} // namespace Audio
} // namespace RPGEngine