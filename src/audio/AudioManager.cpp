#include "AudioManager.h"
#include "MockAudioDevice.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Audio {

// Mock OpenAL functions
namespace {
    void alGenSources(int n, unsigned int* sources) {
        static unsigned int nextSource = 1;
        for (int i = 0; i < n; ++i) {
            sources[i] = nextSource++;
        }
    }
    
    void alDeleteSources(int n, const unsigned int* sources) {
        // Mock implementation
    }
    
    void alSourcef(unsigned int source, int param, float value) {
        // Mock implementation
    }
    
    void alSourcei(unsigned int source, int param, int value) {
        // Mock implementation
    }
    
    void alSource3f(unsigned int source, int param, float value1, float value2, float value3) {
        // Mock implementation
    }
    
    void alGetSourcef(unsigned int source, int param, float* value) {
        // Mock implementation
        *value = 0.0f;
    }
    
    void alGetSourcei(unsigned int source, int param, int* value) {
        // Mock implementation
        *value = 0;
    }
    
    void alSourcePlay(unsigned int source) {
        // Mock implementation
    }
    
    void alSourcePause(unsigned int source) {
        // Mock implementation
    }
    
    void alSourceStop(unsigned int source) {
        // Mock implementation
    }
    
    void alSourceRewind(unsigned int source) {
        // Mock implementation
    }
    
    void alListenerf(int param, float value) {
        // Mock implementation
    }
    
    void* alcOpenDevice(const char* devicename) {
        // Mock implementation
        return reinterpret_cast<void*>(1);
    }
    
    void* alcCreateContext(void* device, const int* attrlist) {
        // Mock implementation
        return reinterpret_cast<void*>(1);
    }
    
    void alcMakeContextCurrent(void* context) {
        // Mock implementation
    }
    
    void alcDestroyContext(void* context) {
        // Mock implementation
    }
    
    void alcCloseDevice(void* device) {
        // Mock implementation
    }
}

// AudioSource implementation
AudioSource::AudioSource(std::shared_ptr<Resources::AudioResource> resource)
    : m_resource(resource)
    , m_sourceHandle(0)
    , m_state(AudioSourceState::Initial)
    , m_looping(false)
    , m_volume(1.0f)
    , m_pitch(1.0f)
{
    // Generate OpenAL source
    alGenSources(1, &m_sourceHandle);
    
    // Set initial source properties
    alSourcef(m_sourceHandle, 0x1001, m_volume); // AL_GAIN
    alSourcef(m_sourceHandle, 0x1003, m_pitch); // AL_PITCH
    alSourcei(m_sourceHandle, 0x1007, m_looping ? 1 : 0); // AL_LOOPING
    
    // Attach buffer if resource is loaded and not streaming
    if (resource && resource->isLoaded() && !resource->isStreaming()) {
        alSourcei(m_sourceHandle, 0x1009, resource->getBufferHandle()); // AL_BUFFER
    }
}

AudioSource::~AudioSource() {
    // Stop playback
    stop();
    
    // Delete OpenAL source
    if (m_sourceHandle != 0) {
        alDeleteSources(1, &m_sourceHandle);
        m_sourceHandle = 0;
    }
}

void AudioSource::play(bool loop) {
    // Set looping
    m_looping = loop;
    alSourcei(m_sourceHandle, 0x1007, m_looping ? 1 : 0); // AL_LOOPING
    
    // Start playback
    alSourcePlay(m_sourceHandle);
    m_state = AudioSourceState::Playing;
}

void AudioSource::pause() {
    // Pause playback
    alSourcePause(m_sourceHandle);
    m_state = AudioSourceState::Paused;
}

void AudioSource::stop() {
    // Stop playback
    alSourceStop(m_sourceHandle);
    m_state = AudioSourceState::Stopped;
}

void AudioSource::setVolume(float volume) {
    // Clamp volume to valid range
    m_volume = std::max(0.0f, std::min(1.0f, volume));
    
    // Set source volume
    alSourcef(m_sourceHandle, 0x1001, m_volume); // AL_GAIN
}

void AudioSource::setPitch(float pitch) {
    // Clamp pitch to valid range
    m_pitch = std::max(0.5f, std::min(2.0f, pitch));
    
    // Set source pitch
    alSourcef(m_sourceHandle, 0x1003, m_pitch); // AL_PITCH
}

float AudioSource::getPosition() const {
    float position;
    alGetSourcef(m_sourceHandle, 0x1001, &position); // AL_SEC_OFFSET
    return position;
}

void AudioSource::setPosition(float position) {
    alSourcef(m_sourceHandle, 0x1001, position); // AL_SEC_OFFSET
}

// AudioManager implementation
AudioManager::AudioManager(std::shared_ptr<Resources::ResourceManager> resourceManager,
                          std::shared_ptr<IAudioDevice> audioDevice)
    : m_resourceManager(resourceManager)
    , m_audioDevice(audioDevice)
    , m_ownsDevice(false)
    , m_masterVolume(1.0f)
    , m_soundVolume(1.0f)
    , m_musicVolume(1.0f)
    , m_muted(false)
{
    // Create default audio device if none provided
    if (!m_audioDevice) {
        m_audioDevice = std::make_shared<MockAudioDevice>();
        m_ownsDevice = true;
    }
}

AudioManager::~AudioManager() {
    if (m_audioDevice && m_audioDevice->isInitialized()) {
        shutdown();
    }
}

bool AudioManager::initialize(const char* deviceName) {
    // Initialize audio device
    if (!m_audioDevice->initialize(deviceName)) {
        std::cerr << "Failed to initialize audio device" << std::endl;
        return false;
    }
    
    // Set initial audio properties
    m_audioDevice->setMasterVolume(m_masterVolume);
    m_audioDevice->setListenerPosition(m_listener.x, m_listener.y, m_listener.z);
    m_audioDevice->setListenerOrientation(m_listener.forwardX, m_listener.forwardY, m_listener.forwardZ,
                                         m_listener.upX, m_listener.upY, m_listener.upZ);
    m_audioDevice->setListenerVelocity(m_listener.velocityX, m_listener.velocityY, m_listener.velocityZ);
    
    // Fire initialization event
    fireEvent("AudioManager", "Initialized with device: " + m_audioDevice->getCurrentDevice());
    
    std::cout << "AudioManager initialized with device: " << m_audioDevice->getCurrentDevice() << std::endl;
    return true;
}

void AudioManager::shutdown() {
    // Stop all audio
    stopAll();
    
    // Clear audio sources
    m_soundSources.clear();
    m_musicSources.clear();
    
    // Shutdown audio device
    if (m_audioDevice) {
        m_audioDevice->shutdown();
    }
    
    // Fire shutdown event
    fireEvent("AudioManager", "Shutdown");
    
    std::cout << "AudioManager shutdown" << std::endl;
}

void AudioManager::update(float deltaTime) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Update sound sources
    for (auto it = m_soundSources.begin(); it != m_soundSources.end();) {
        auto& source = *it;
        
        // Check if source is still playing
        int state;
        alGetSourcei(source->getSourceHandle(), 0x1010, &state); // AL_SOURCE_STATE
        
        if (state != 0x1012 && state != 0x1013) { // AL_PLAYING or AL_PAUSED
            // Source is no longer playing, remove it
            it = m_soundSources.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update music sources
    for (auto it = m_musicSources.begin(); it != m_musicSources.end();) {
        auto& source = *it;
        
        // Check if source is still playing
        int state;
        alGetSourcei(source->getSourceHandle(), 0x1010, &state); // AL_SOURCE_STATE
        
        if (state != 0x1012 && state != 0x1013) { // AL_PLAYING or AL_PAUSED
            // Source is no longer playing, remove it
            it = m_musicSources.erase(it);
        } else {
            ++it;
        }
    }
    
    // Clean up unused sources
    cleanupSources();
}

std::shared_ptr<AudioSource> AudioManager::playSound(const std::string& resourceId, float volume, float pitch, bool loop) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get audio resource
    auto resource = m_resourceManager->getResourceOfType<Resources::AudioResource>(resourceId);
    if (!resource) {
        std::cerr << "Audio resource not found: " << resourceId << std::endl;
        return nullptr;
    }
    
    // Load resource if not already loaded
    if (!resource->isLoaded()) {
        if (!resource->load()) {
            std::cerr << "Failed to load audio resource: " << resourceId << std::endl;
            return nullptr;
        }
    }
    
    // Create audio source
    auto source = std::make_shared<AudioSource>(resource);
    
    // Set source properties
    source->setVolume(volume * m_soundVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    source->setPitch(pitch);
    
    // Play source
    source->play(loop);
    
    // Add to sound sources
    m_soundSources.push_back(source);
    
    return source;
}

std::shared_ptr<AudioSource> AudioManager::playMusic(const std::string& resourceId, float volume, bool loop) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Stop all music
    stopAllMusic();
    
    // Get audio resource
    auto resource = m_resourceManager->getResourceOfType<Resources::AudioResource>(resourceId);
    if (!resource) {
        std::cerr << "Audio resource not found: " << resourceId << std::endl;
        return nullptr;
    }
    
    // Set resource to streaming
    resource->setStreaming(true);
    
    // Load resource if not already loaded
    if (!resource->isLoaded()) {
        if (!resource->load()) {
            std::cerr << "Failed to load audio resource: " << resourceId << std::endl;
            return nullptr;
        }
    }
    
    // Create audio source
    auto source = std::make_shared<AudioSource>(resource);
    
    // Set source properties
    source->setVolume(volume * m_musicVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    source->setPitch(1.0f);
    
    // Play source
    source->play(loop);
    
    // Add to music sources
    m_musicSources.push_back(source);
    
    return source;
}

void AudioManager::stopAllSounds() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Stop all sound sources
    for (auto& source : m_soundSources) {
        source->stop();
    }
    
    // Clear sound sources
    m_soundSources.clear();
}

void AudioManager::stopAllMusic() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Stop all music sources
    for (auto& source : m_musicSources) {
        source->stop();
    }
    
    // Clear music sources
    m_musicSources.clear();
}

void AudioManager::stopAll() {
    stopAllSounds();
    stopAllMusic();
}

void AudioManager::pauseAllSounds() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Pause all sound sources
    for (auto& source : m_soundSources) {
        source->pause();
    }
}

void AudioManager::pauseAllMusic() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Pause all music sources
    for (auto& source : m_musicSources) {
        source->pause();
    }
}

void AudioManager::pauseAll() {
    pauseAllSounds();
    pauseAllMusic();
}

void AudioManager::resumeAllSounds() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Resume all sound sources
    for (auto& source : m_soundSources) {
        if (source->isPaused()) {
            source->play(source->isLooping());
        }
    }
}

void AudioManager::resumeAllMusic() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Resume all music sources
    for (auto& source : m_musicSources) {
        if (source->isPaused()) {
            source->play(source->isLooping());
        }
    }
}

void AudioManager::resumeAll() {
    resumeAllSounds();
    resumeAllMusic();
}

void AudioManager::setMasterVolume(float volume) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clamp volume to valid range
    m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
    
    // Update device master volume
    if (m_audioDevice) {
        m_audioDevice->setMasterVolume(m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
    
    // Update all sound sources
    for (auto& source : m_soundSources) {
        source->setVolume(source->getVolume() * m_soundVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
    
    // Update all music sources
    for (auto& source : m_musicSources) {
        source->setVolume(source->getVolume() * m_musicVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
    
    fireEvent("AudioManager", "Master volume set to " + std::to_string(m_masterVolume));
}

void AudioManager::setSoundVolume(float volume) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clamp volume to valid range
    m_soundVolume = std::max(0.0f, std::min(1.0f, volume));
    
    // Update all sound sources
    for (auto& source : m_soundSources) {
        source->setVolume(source->getVolume() * m_soundVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
}

void AudioManager::setMusicVolume(float volume) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clamp volume to valid range
    m_musicVolume = std::max(0.0f, std::min(1.0f, volume));
    
    // Update all music sources
    for (auto& source : m_musicSources) {
        source->setVolume(source->getVolume() * m_musicVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
}

void AudioManager::setMuted(bool muted) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_muted = muted;
    
    // Update all sound sources
    for (auto& source : m_soundSources) {
        source->setVolume(source->getVolume() * m_soundVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
    
    // Update all music sources
    for (auto& source : m_musicSources) {
        source->setVolume(source->getVolume() * m_musicVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    }
    
    // Set listener gain
    alListenerf(0x1003, m_masterVolume * (m_muted ? 0.0f : 1.0f)); // AL_GAIN
}

void AudioManager::cleanupSources() {
    // Remove stopped sound sources
    m_soundSources.erase(
        std::remove_if(m_soundSources.begin(), m_soundSources.end(),
            [](const std::shared_ptr<AudioSource>& source) {
                return source->isStopped();
            }),
        m_soundSources.end()
    );
    
    // Remove stopped music sources
    m_musicSources.erase(
        std::remove_if(m_musicSources.begin(), m_musicSources.end(),
            [](const std::shared_ptr<AudioSource>& source) {
                return source->isStopped();
            }),
        m_musicSources.end()
    );
}

std::vector<std::string> AudioManager::getAvailableDevices() const {
    if (m_audioDevice) {
        return m_audioDevice->getAvailableDevices();
    }
    return {};
}

std::string AudioManager::getCurrentDevice() const {
    if (m_audioDevice) {
        return m_audioDevice->getCurrentDevice();
    }
    return "";
}

AudioDeviceCapabilities AudioManager::getDeviceCapabilities() const {
    if (m_audioDevice) {
        return m_audioDevice->getCapabilities();
    }
    return {};
}

void AudioManager::setListener(const AudioListener& listener) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_listener = listener;
    
    if (m_audioDevice) {
        m_audioDevice->setListenerPosition(listener.x, listener.y, listener.z);
        m_audioDevice->setListenerOrientation(listener.forwardX, listener.forwardY, listener.forwardZ,
                                             listener.upX, listener.upY, listener.upZ);
        m_audioDevice->setListenerVelocity(listener.velocityX, listener.velocityY, listener.velocityZ);
    }
    
    fireEvent("AudioManager", "Listener updated");
}

std::shared_ptr<AudioSource> AudioManager::play3DSound(const std::string& resourceId, 
                                                       float x, float y, float z,
                                                       float volume, float pitch, bool loop) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get audio resource
    auto resource = m_resourceManager->getResourceOfType<Resources::AudioResource>(resourceId);
    if (!resource) {
        std::cerr << "Audio resource not found: " << resourceId << std::endl;
        return nullptr;
    }
    
    // Load resource if not already loaded
    if (!resource->isLoaded()) {
        if (!resource->load()) {
            std::cerr << "Failed to load audio resource: " << resourceId << std::endl;
            return nullptr;
        }
    }
    
    // Create audio source
    auto source = std::make_shared<AudioSource>(resource);
    
    // Set 3D properties
    if (m_audioDevice) {
        m_audioDevice->setSourcePosition(source->getSourceHandle(), x, y, z);
    }
    
    // Set source properties
    source->setVolume(volume * m_soundVolume * m_masterVolume * (m_muted ? 0.0f : 1.0f));
    source->setPitch(pitch);
    
    // Play source
    source->play(loop);
    
    // Add to sound sources
    m_soundSources.push_back(source);
    
    fireEvent("AudioManager", "3D sound played: " + resourceId);
    
    return source;
}

void AudioManager::fireEvent(const std::string& eventType, const std::string& details) {
    if (m_eventCallback) {
        m_eventCallback(eventType, details);
    }
}

} // namespace Audio
} // namespace RPGEngine