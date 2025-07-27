#include "MockAudioDevice.h"
#include <algorithm>

namespace RPGEngine {
namespace Audio {

MockAudioDevice::MockAudioDevice()
    : m_initialized(false)
    , m_currentDevice("Mock Audio Device")
    , m_masterVolume(1.0f)
    , m_listenerX(0.0f), m_listenerY(0.0f), m_listenerZ(0.0f)
    , m_listenerForwardX(0.0f), m_listenerForwardY(0.0f), m_listenerForwardZ(-1.0f)
    , m_listenerUpX(0.0f), m_listenerUpY(1.0f), m_listenerUpZ(0.0f)
    , m_listenerVX(0.0f), m_listenerVY(0.0f), m_listenerVZ(0.0f)
    , m_nextSourceId(1)
    , m_nextBufferId(1)
{
}

MockAudioDevice::~MockAudioDevice() {
    if (m_initialized) {
        shutdown();
    }
}

bool MockAudioDevice::initialize(const char* deviceName) {
    if (m_initialized) {
        return true;
    }
    
    if (deviceName) {
        m_currentDevice = deviceName;
    }
    
    m_initialized = true;
    
    std::cout << "MockAudioDevice initialized: " << m_currentDevice << std::endl;
    return true;
}

void MockAudioDevice::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Clear all sources and buffers
    m_sources.clear();
    m_buffers.clear();
    
    m_initialized = false;
    
    std::cout << "MockAudioDevice shutdown" << std::endl;
}

AudioDeviceCapabilities MockAudioDevice::getCapabilities() const {
    AudioDeviceCapabilities caps;
    caps.name = m_currentDevice;
    caps.maxSources = 256;
    caps.maxBuffers = 1024;
    caps.supportedSampleRates = {8000, 11025, 22050, 44100, 48000, 96000};
    caps.supports3D = true;
    caps.supportsEffects = true;
    caps.supportsStreaming = true;
    
    return caps;
}

std::vector<std::string> MockAudioDevice::getAvailableDevices() const {
    return {
        "Mock Audio Device",
        "Mock Audio Device 2",
        "Mock High Quality Device",
        "Mock Low Latency Device"
    };
}

void MockAudioDevice::setMasterVolume(float volume) {
    m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
    std::cout << "MockAudioDevice: Master volume set to " << m_masterVolume << std::endl;
}

void MockAudioDevice::setListenerPosition(float x, float y, float z) {
    m_listenerX = x;
    m_listenerY = y;
    m_listenerZ = z;
    std::cout << "MockAudioDevice: Listener position set to (" << x << ", " << y << ", " << z << ")" << std::endl;
}

void MockAudioDevice::setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                            float upX, float upY, float upZ) {
    m_listenerForwardX = forwardX;
    m_listenerForwardY = forwardY;
    m_listenerForwardZ = forwardZ;
    m_listenerUpX = upX;
    m_listenerUpY = upY;
    m_listenerUpZ = upZ;
    std::cout << "MockAudioDevice: Listener orientation set" << std::endl;
}

void MockAudioDevice::setListenerVelocity(float x, float y, float z) {
    m_listenerVX = x;
    m_listenerVY = y;
    m_listenerVZ = z;
    std::cout << "MockAudioDevice: Listener velocity set to (" << x << ", " << y << ", " << z << ")" << std::endl;
}

unsigned int MockAudioDevice::createSource() {
    unsigned int sourceId = m_nextSourceId++;
    m_sources[sourceId] = MockSource();
    std::cout << "MockAudioDevice: Created source " << sourceId << std::endl;
    return sourceId;
}

void MockAudioDevice::destroySource(unsigned int source) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        m_sources.erase(it);
        std::cout << "MockAudioDevice: Destroyed source " << source << std::endl;
    }
}

unsigned int MockAudioDevice::createBuffer() {
    unsigned int bufferId = m_nextBufferId++;
    m_buffers[bufferId] = MockBuffer();
    std::cout << "MockAudioDevice: Created buffer " << bufferId << std::endl;
    return bufferId;
}

void MockAudioDevice::destroyBuffer(unsigned int buffer) {
    auto it = m_buffers.find(buffer);
    if (it != m_buffers.end()) {
        m_buffers.erase(it);
        std::cout << "MockAudioDevice: Destroyed buffer " << buffer << std::endl;
    }
}

void MockAudioDevice::bufferData(unsigned int buffer, int format, const void* data, int size, int sampleRate) {
    auto it = m_buffers.find(buffer);
    if (it != m_buffers.end()) {
        it->second.format = format;
        it->second.size = size;
        it->second.sampleRate = sampleRate;
        std::cout << "MockAudioDevice: Buffer " << buffer << " loaded with " << size 
                  << " bytes at " << sampleRate << " Hz" << std::endl;
    }
}

void MockAudioDevice::setSourceBuffer(unsigned int source, unsigned int buffer) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.buffer = buffer;
        std::cout << "MockAudioDevice: Source " << source << " attached to buffer " << buffer << std::endl;
    }
}

void MockAudioDevice::playSource(unsigned int source) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.state = 1; // Playing
        std::cout << "MockAudioDevice: Playing source " << source << std::endl;
    }
}

void MockAudioDevice::pauseSource(unsigned int source) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.state = 2; // Paused
        std::cout << "MockAudioDevice: Paused source " << source << std::endl;
    }
}

void MockAudioDevice::stopSource(unsigned int source) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.state = 0; // Stopped
        it->second.position = 0.0f;
        std::cout << "MockAudioDevice: Stopped source " << source << std::endl;
    }
}

void MockAudioDevice::setSourceVolume(unsigned int source, float volume) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.volume = std::max(0.0f, std::min(1.0f, volume));
        std::cout << "MockAudioDevice: Source " << source << " volume set to " << it->second.volume << std::endl;
    }
}

void MockAudioDevice::setSourcePitch(unsigned int source, float pitch) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.pitch = std::max(0.5f, std::min(2.0f, pitch));
        std::cout << "MockAudioDevice: Source " << source << " pitch set to " << it->second.pitch << std::endl;
    }
}

void MockAudioDevice::setSourcePosition(unsigned int source, float x, float y, float z) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.x = x;
        it->second.y = y;
        it->second.z = z;
        std::cout << "MockAudioDevice: Source " << source << " position set to (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
}

void MockAudioDevice::setSourceVelocity(unsigned int source, float x, float y, float z) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.vx = x;
        it->second.vy = y;
        it->second.vz = z;
        std::cout << "MockAudioDevice: Source " << source << " velocity set to (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
}

void MockAudioDevice::setSourceLooping(unsigned int source, bool looping) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.looping = looping;
        std::cout << "MockAudioDevice: Source " << source << " looping set to " << (looping ? "true" : "false") << std::endl;
    }
}

int MockAudioDevice::getSourceState(unsigned int source) const {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        return it->second.state;
    }
    return 0; // Stopped
}

float MockAudioDevice::getSourcePosition(unsigned int source) const {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        return it->second.position;
    }
    return 0.0f;
}

void MockAudioDevice::setSourcePlaybackPosition(unsigned int source, float position) {
    auto it = m_sources.find(source);
    if (it != m_sources.end()) {
        it->second.position = std::max(0.0f, position);
        std::cout << "MockAudioDevice: Source " << source << " playback position set to " << position << std::endl;
    }
}

} // namespace Audio
} // namespace RPGEngine