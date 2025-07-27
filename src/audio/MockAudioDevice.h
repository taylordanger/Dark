#pragma once

#include "IAudioDevice.h"
#include <unordered_map>
#include <iostream>

namespace RPGEngine {
namespace Audio {

/**
 * Mock audio device for testing
 * Simulates audio device functionality without actual audio output
 */
class MockAudioDevice : public IAudioDevice {
public:
    MockAudioDevice();
    ~MockAudioDevice();
    
    // IAudioDevice implementation
    bool initialize(const char* deviceName = nullptr) override;
    void shutdown() override;
    bool isInitialized() const override { return m_initialized; }
    
    AudioDeviceCapabilities getCapabilities() const override;
    std::vector<std::string> getAvailableDevices() const override;
    std::string getCurrentDevice() const override { return m_currentDevice; }
    
    void setMasterVolume(float volume) override;
    float getMasterVolume() const override { return m_masterVolume; }
    
    void setListenerPosition(float x, float y, float z) override;
    void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                               float upX, float upY, float upZ) override;
    void setListenerVelocity(float x, float y, float z) override;
    
    unsigned int createSource() override;
    void destroySource(unsigned int source) override;
    
    unsigned int createBuffer() override;
    void destroyBuffer(unsigned int buffer) override;
    
    void bufferData(unsigned int buffer, int format, const void* data, int size, int sampleRate) override;
    void setSourceBuffer(unsigned int source, unsigned int buffer) override;
    
    void playSource(unsigned int source) override;
    void pauseSource(unsigned int source) override;
    void stopSource(unsigned int source) override;
    
    void setSourceVolume(unsigned int source, float volume) override;
    void setSourcePitch(unsigned int source, float pitch) override;
    void setSourcePosition(unsigned int source, float x, float y, float z) override;
    void setSourceVelocity(unsigned int source, float x, float y, float z) override;
    void setSourceLooping(unsigned int source, bool looping) override;
    
    int getSourceState(unsigned int source) const override;
    float getSourcePosition(unsigned int source) const override;
    void setSourcePlaybackPosition(unsigned int source, float position) override;
    
private:
    struct MockSource {
        unsigned int buffer = 0;
        float volume = 1.0f;
        float pitch = 1.0f;
        float x = 0.0f, y = 0.0f, z = 0.0f;
        float vx = 0.0f, vy = 0.0f, vz = 0.0f;
        bool looping = false;
        int state = 0; // 0 = stopped, 1 = playing, 2 = paused
        float position = 0.0f;
    };
    
    struct MockBuffer {
        int format = 0;
        int size = 0;
        int sampleRate = 0;
    };
    
    bool m_initialized;
    std::string m_currentDevice;
    float m_masterVolume;
    
    // Listener properties
    float m_listenerX, m_listenerY, m_listenerZ;
    float m_listenerForwardX, m_listenerForwardY, m_listenerForwardZ;
    float m_listenerUpX, m_listenerUpY, m_listenerUpZ;
    float m_listenerVX, m_listenerVY, m_listenerVZ;
    
    // Mock sources and buffers
    std::unordered_map<unsigned int, MockSource> m_sources;
    std::unordered_map<unsigned int, MockBuffer> m_buffers;
    
    unsigned int m_nextSourceId;
    unsigned int m_nextBufferId;
};

} // namespace Audio
} // namespace RPGEngine