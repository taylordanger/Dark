#pragma once

#include <string>
#include <vector>
#include <memory>

namespace RPGEngine {
namespace Audio {

/**
 * Audio device capabilities
 */
struct AudioDeviceCapabilities {
    std::string name;                    // Device name
    int maxSources;                      // Maximum number of audio sources
    int maxBuffers;                      // Maximum number of audio buffers
    std::vector<int> supportedSampleRates; // Supported sample rates
    bool supports3D;                     // 3D audio support
    bool supportsEffects;                // Audio effects support
    bool supportsStreaming;              // Streaming support
};

/**
 * Audio device interface
 * Platform abstraction for audio devices
 */
class IAudioDevice {
public:
    virtual ~IAudioDevice() = default;
    
    /**
     * Initialize the audio device
     * @param deviceName Device name (nullptr for default)
     * @return true if initialization was successful
     */
    virtual bool initialize(const char* deviceName = nullptr) = 0;
    
    /**
     * Shutdown the audio device
     */
    virtual void shutdown() = 0;
    
    /**
     * Check if the device is initialized
     * @return true if the device is initialized
     */
    virtual bool isInitialized() const = 0;
    
    /**
     * Get device capabilities
     * @return Device capabilities
     */
    virtual AudioDeviceCapabilities getCapabilities() const = 0;
    
    /**
     * Get available audio devices
     * @return List of available device names
     */
    virtual std::vector<std::string> getAvailableDevices() const = 0;
    
    /**
     * Get current device name
     * @return Current device name
     */
    virtual std::string getCurrentDevice() const = 0;
    
    /**
     * Set master volume
     * @param volume Volume (0.0 to 1.0)
     */
    virtual void setMasterVolume(float volume) = 0;
    
    /**
     * Get master volume
     * @return Master volume (0.0 to 1.0)
     */
    virtual float getMasterVolume() const = 0;
    
    /**
     * Set listener position
     * @param x X position
     * @param y Y position
     * @param z Z position
     */
    virtual void setListenerPosition(float x, float y, float z) = 0;
    
    /**
     * Set listener orientation
     * @param forwardX Forward vector X
     * @param forwardY Forward vector Y
     * @param forwardZ Forward vector Z
     * @param upX Up vector X
     * @param upY Up vector Y
     * @param upZ Up vector Z
     */
    virtual void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                       float upX, float upY, float upZ) = 0;
    
    /**
     * Set listener velocity
     * @param x X velocity
     * @param y Y velocity
     * @param z Z velocity
     */
    virtual void setListenerVelocity(float x, float y, float z) = 0;
    
    /**
     * Create audio source
     * @return Audio source handle
     */
    virtual unsigned int createSource() = 0;
    
    /**
     * Destroy audio source
     * @param source Source handle
     */
    virtual void destroySource(unsigned int source) = 0;
    
    /**
     * Create audio buffer
     * @return Audio buffer handle
     */
    virtual unsigned int createBuffer() = 0;
    
    /**
     * Destroy audio buffer
     * @param buffer Buffer handle
     */
    virtual void destroyBuffer(unsigned int buffer) = 0;
    
    /**
     * Upload audio data to buffer
     * @param buffer Buffer handle
     * @param format Audio format
     * @param data Audio data
     * @param size Data size in bytes
     * @param sampleRate Sample rate
     */
    virtual void bufferData(unsigned int buffer, int format, const void* data, int size, int sampleRate) = 0;
    
    /**
     * Set source buffer
     * @param source Source handle
     * @param buffer Buffer handle
     */
    virtual void setSourceBuffer(unsigned int source, unsigned int buffer) = 0;
    
    /**
     * Play audio source
     * @param source Source handle
     */
    virtual void playSource(unsigned int source) = 0;
    
    /**
     * Pause audio source
     * @param source Source handle
     */
    virtual void pauseSource(unsigned int source) = 0;
    
    /**
     * Stop audio source
     * @param source Source handle
     */
    virtual void stopSource(unsigned int source) = 0;
    
    /**
     * Set source volume
     * @param source Source handle
     * @param volume Volume (0.0 to 1.0)
     */
    virtual void setSourceVolume(unsigned int source, float volume) = 0;
    
    /**
     * Set source pitch
     * @param source Source handle
     * @param pitch Pitch (0.5 to 2.0)
     */
    virtual void setSourcePitch(unsigned int source, float pitch) = 0;
    
    /**
     * Set source position
     * @param source Source handle
     * @param x X position
     * @param y Y position
     * @param z Z position
     */
    virtual void setSourcePosition(unsigned int source, float x, float y, float z) = 0;
    
    /**
     * Set source velocity
     * @param source Source handle
     * @param x X velocity
     * @param y Y velocity
     * @param z Z velocity
     */
    virtual void setSourceVelocity(unsigned int source, float x, float y, float z) = 0;
    
    /**
     * Set source looping
     * @param source Source handle
     * @param looping Whether to loop
     */
    virtual void setSourceLooping(unsigned int source, bool looping) = 0;
    
    /**
     * Get source state
     * @param source Source handle
     * @return Source state (playing, paused, stopped)
     */
    virtual int getSourceState(unsigned int source) const = 0;
    
    /**
     * Get source playback position
     * @param source Source handle
     * @return Playback position in seconds
     */
    virtual float getSourcePosition(unsigned int source) const = 0;
    
    /**
     * Set source playback position
     * @param source Source handle
     * @param position Position in seconds
     */
    virtual void setSourcePlaybackPosition(unsigned int source, float position) = 0;
};

} // namespace Audio
} // namespace RPGEngine