#pragma once

#include "Resource.h"
#include <string>
#include <memory>
#include <vector>

namespace RPGEngine {
namespace Resources {

/**
 * Audio format enumeration
 */
enum class AudioFormat {
    Unknown,
    WAV,
    OGG,
    MP3
};

/**
 * Audio resource
 * Represents a loadable audio file
 */
class AudioResource : public Resource {
public:
    /**
     * Constructor
     * @param id Resource ID
     * @param path Resource path
     * @param streaming Whether to stream the audio
     */
    AudioResource(const std::string& id, const std::string& path, bool streaming = false);
    
    /**
     * Destructor
     */
    ~AudioResource();
    
    /**
     * Load the resource
     * @return true if the resource was loaded successfully
     */
    bool load() override;
    
    /**
     * Unload the resource
     */
    void unload() override;
    
    /**
     * Get the audio format
     * @return Audio format
     */
    AudioFormat getFormat() const { return m_format; }
    
    /**
     * Get the audio duration in seconds
     * @return Audio duration
     */
    float getDuration() const { return m_duration; }
    
    /**
     * Get the audio sample rate
     * @return Sample rate in Hz
     */
    int getSampleRate() const { return m_sampleRate; }
    
    /**
     * Get the number of channels
     * @return Number of channels
     */
    int getChannels() const { return m_channels; }
    
    /**
     * Get the audio data
     * @return Audio data
     */
    const std::vector<uint8_t>& getData() const { return m_data; }
    
    /**
     * Check if the audio is streaming
     * @return true if the audio is streaming
     */
    bool isStreaming() const { return m_streaming; }
    
    /**
     * Set whether the audio is streaming
     * @param streaming Whether to stream the audio
     */
    void setStreaming(bool streaming) { m_streaming = streaming; }
    
    /**
     * Get the audio buffer handle
     * @return Audio buffer handle
     */
    unsigned int getBufferHandle() const { return m_bufferHandle; }
    
private:
    /**
     * Determine the audio format from the file extension
     * @param path File path
     * @return Audio format
     */
    AudioFormat determineFormat(const std::string& path);
    
    /**
     * Load WAV file
     * @return true if the file was loaded successfully
     */
    bool loadWAV();
    
    /**
     * Load OGG file
     * @return true if the file was loaded successfully
     */
    bool loadOGG();
    
    /**
     * Load MP3 file
     * @return true if the file was loaded successfully
     */
    bool loadMP3();
    
    AudioFormat m_format;           // Audio format
    float m_duration;               // Audio duration in seconds
    int m_sampleRate;               // Sample rate in Hz
    int m_channels;                 // Number of channels
    bool m_streaming;               // Whether to stream the audio
    std::vector<uint8_t> m_data;    // Audio data
    unsigned int m_bufferHandle;    // Audio buffer handle
};

} // namespace Resources
} // namespace RPGEngine