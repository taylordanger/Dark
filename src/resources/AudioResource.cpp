#include "AudioResource.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

namespace RPGEngine {
namespace Resources {

// Mock OpenAL functions
namespace {
    void alGenBuffers(int n, unsigned int* buffers) {
        static unsigned int nextBuffer = 1;
        for (int i = 0; i < n; ++i) {
            buffers[i] = nextBuffer++;
        }
    }
    
    void alDeleteBuffers(int n, const unsigned int* buffers) {
        // Mock implementation
    }
    
    void alBufferData(unsigned int buffer, int format, const void* data, int size, int freq) {
        // Mock implementation
    }
}

AudioResource::AudioResource(const std::string& id, const std::string& path, bool streaming)
    : Resource(id, path)
    , m_format(AudioFormat::Unknown)
    , m_duration(0.0f)
    , m_sampleRate(0)
    , m_channels(0)
    , m_streaming(streaming)
    , m_bufferHandle(0)
{
    m_format = determineFormat(path);
}

AudioResource::~AudioResource() {
    if (isLoaded()) {
        unload();
    }
}

bool AudioResource::load() {
    // Check if already loaded
    if (isLoaded()) {
        return true;
    }
    
    // Set state to loading
    setState(ResourceState::Loading);
    
    // Load based on format
    bool success = false;
    
    switch (m_format) {
        case AudioFormat::WAV:
            success = loadWAV();
            break;
        case AudioFormat::OGG:
            success = loadOGG();
            break;
        case AudioFormat::MP3:
            success = loadMP3();
            break;
        default:
            std::cerr << "Unsupported audio format: " << getPath() << std::endl;
            setState(ResourceState::Failed);
            return false;
    }
    
    if (!success) {
        setState(ResourceState::Failed);
        return false;
    }
    
    // Generate OpenAL buffer if not streaming
    if (!m_streaming) {
        alGenBuffers(1, &m_bufferHandle);
        
        // Determine format based on channels
        int format = (m_channels == 1) ? 0x1100 : 0x1102; // AL_FORMAT_MONO16 or AL_FORMAT_STEREO16
        
        // Upload audio data
        alBufferData(m_bufferHandle, format, m_data.data(), static_cast<int>(m_data.size()), m_sampleRate);
    }
    
    // Set state to loaded
    setState(ResourceState::Loaded);
    
    std::cout << "Loaded audio: " << getPath() << " (" << m_duration << "s, " << m_channels << " channels, " 
              << m_sampleRate << " Hz, " << (m_streaming ? "streaming" : "static") << ")" << std::endl;
    
    return true;
}

void AudioResource::unload() {
    // Check if loaded
    if (!isLoaded()) {
        return;
    }
    
    // Delete OpenAL buffer if not streaming
    if (!m_streaming && m_bufferHandle != 0) {
        alDeleteBuffers(1, &m_bufferHandle);
        m_bufferHandle = 0;
    }
    
    // Clear audio data
    m_data.clear();
    m_data.shrink_to_fit();
    
    // Reset audio information
    m_duration = 0.0f;
    m_sampleRate = 0;
    m_channels = 0;
    
    // Set state to unloaded
    setState(ResourceState::Unloaded);
    
    std::cout << "Unloaded audio: " << getPath() << std::endl;
}

AudioFormat AudioResource::determineFormat(const std::string& path) {
    // Get file extension
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return AudioFormat::Unknown;
    }
    
    std::string ext = path.substr(dotPos + 1);
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    
    // Determine format based on extension
    if (ext == "wav") {
        return AudioFormat::WAV;
    } else if (ext == "ogg") {
        return AudioFormat::OGG;
    } else if (ext == "mp3") {
        return AudioFormat::MP3;
    }
    
    return AudioFormat::Unknown;
}

bool AudioResource::loadWAV() {
    // Open file
    std::ifstream file(getPath(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open WAV file: " << getPath() << std::endl;
        return false;
    }
    
    // Read WAV header
    char header[44];
    file.read(header, 44);
    
    // Check RIFF header
    if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F') {
        std::cerr << "Invalid WAV file (RIFF header): " << getPath() << std::endl;
        return false;
    }
    
    // Check WAVE format
    if (header[8] != 'W' || header[9] != 'A' || header[10] != 'V' || header[11] != 'E') {
        std::cerr << "Invalid WAV file (WAVE format): " << getPath() << std::endl;
        return false;
    }
    
    // Check fmt chunk
    if (header[12] != 'f' || header[13] != 'm' || header[14] != 't' || header[15] != ' ') {
        std::cerr << "Invalid WAV file (fmt chunk): " << getPath() << std::endl;
        return false;
    }
    
    // Get audio format (PCM = 1)
    int audioFormat = header[20] | (header[21] << 8);
    if (audioFormat != 1) {
        std::cerr << "Unsupported WAV format (not PCM): " << getPath() << std::endl;
        return false;
    }
    
    // Get channels
    m_channels = header[22] | (header[23] << 8);
    
    // Get sample rate
    m_sampleRate = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    
    // Get bits per sample
    int bitsPerSample = header[34] | (header[35] << 8);
    
    // Check data chunk
    if (header[36] != 'd' || header[37] != 'a' || header[38] != 't' || header[39] != 'a') {
        std::cerr << "Invalid WAV file (data chunk): " << getPath() << std::endl;
        return false;
    }
    
    // Get data size
    int dataSize = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);
    
    // Calculate duration
    int bytesPerSample = bitsPerSample / 8;
    int bytesPerSecond = m_sampleRate * m_channels * bytesPerSample;
    m_duration = static_cast<float>(dataSize) / bytesPerSecond;
    
    // Read audio data
    m_data.resize(dataSize);
    file.read(reinterpret_cast<char*>(m_data.data()), dataSize);
    
    // Check if read was successful
    if (file.gcount() != dataSize) {
        std::cerr << "Failed to read WAV data: " << getPath() << std::endl;
        return false;
    }
    
    return true;
}

bool AudioResource::loadOGG() {
    // In a real implementation, this would use a library like libvorbis
    // For this example, we'll just simulate loading an OGG file
    
    std::cout << "Simulating OGG loading: " << getPath() << std::endl;
    
    // Simulate audio properties
    m_channels = 2;
    m_sampleRate = 44100;
    m_duration = 30.0f; // 30 seconds
    
    // Simulate audio data (1 second of silence)
    int bytesPerSample = 2; // 16-bit
    int dataSize = m_sampleRate * m_channels * bytesPerSample;
    m_data.resize(dataSize, 0);
    
    return true;
}

bool AudioResource::loadMP3() {
    // In a real implementation, this would use a library like libmpg123
    // For this example, we'll just simulate loading an MP3 file
    
    std::cout << "Simulating MP3 loading: " << getPath() << std::endl;
    
    // Simulate audio properties
    m_channels = 2;
    m_sampleRate = 44100;
    m_duration = 60.0f; // 60 seconds
    
    // Simulate audio data (1 second of silence)
    int bytesPerSample = 2; // 16-bit
    int dataSize = m_sampleRate * m_channels * bytesPerSample;
    m_data.resize(dataSize, 0);
    
    return true;
}

} // namespace Resources
} // namespace RPGEngine