#include "SoundEffectManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace RPGEngine {
namespace Audio {

SoundEffectManager::SoundEffectManager(std::shared_ptr<AudioManager> audioManager)
    : m_audioManager(audioManager)
    , m_nextSoundId(1)
    , m_totalTime(0.0f)
{
    // Initialize category volumes
    m_categoryVolumes[SoundCategory::UI] = 1.0f;
    m_categoryVolumes[SoundCategory::Ambient] = 0.8f;
    m_categoryVolumes[SoundCategory::Action] = 1.0f;
    m_categoryVolumes[SoundCategory::Combat] = 1.0f;
    m_categoryVolumes[SoundCategory::Environment] = 0.9f;
    m_categoryVolumes[SoundCategory::Voice] = 1.0f;
    m_categoryVolumes[SoundCategory::Music] = 0.8f;
    
    // Initialize category enabled states
    for (auto& pair : m_categoryVolumes) {
        m_categoryEnabled[pair.first] = true;
    }
    
    // Initialize max concurrent sounds per category
    m_maxConcurrentSounds[SoundCategory::UI] = 10;
    m_maxConcurrentSounds[SoundCategory::Ambient] = 5;
    m_maxConcurrentSounds[SoundCategory::Action] = 15;
    m_maxConcurrentSounds[SoundCategory::Combat] = 20;
    m_maxConcurrentSounds[SoundCategory::Environment] = 10;
    m_maxConcurrentSounds[SoundCategory::Voice] = 3;
    m_maxConcurrentSounds[SoundCategory::Music] = 5;
}

SoundEffectManager::~SoundEffectManager() {
    stopAllSounds(0.0f);
}

void SoundEffectManager::update(float deltaTime) {
    m_totalTime += deltaTime;
    
    // Cleanup finished sounds
    cleanupFinishedSounds();
    
    // Update 3D positioning for active sounds
    const auto& listener = m_audioManager->getListener();
    
    for (auto& pair : m_activeSounds) {
        auto& soundEffect = pair.second;
        
        if (soundEffect.properties.is3D && soundEffect.source) {
            // Update 3D volume based on distance
            float volumeMultiplier = calculate3DVolume(soundEffect.properties, listener);
            float finalVolume = soundEffect.properties.volume * 
                               getCategoryVolume(soundEffect.properties.category) * 
                               volumeMultiplier;
            
            soundEffect.source->setVolume(finalVolume);
        }
    }
}

uint32_t SoundEffectManager::playSound(const std::string& resourceId, const SoundEffectProperties& properties) {
    // Check if category is enabled
    if (!isCategoryEnabled(properties.category)) {
        return 0;
    }
    
    // Check concurrent sound limit for category
    int currentCount = getActiveSoundCount(properties.category);
    int maxCount = m_maxConcurrentSounds[properties.category];
    
    if (currentCount >= maxCount) {
        // Find lowest priority sound in this category to replace
        uint32_t lowestPriorityId = 0;
        float lowestPriority = properties.priority;
        
        for (const auto& pair : m_activeSounds) {
            const auto& sound = pair.second;
            if (sound.properties.category == properties.category && 
                sound.properties.priority < lowestPriority) {
                lowestPriority = sound.properties.priority;
                lowestPriorityId = pair.first;
            }
        }
        
        if (lowestPriorityId != 0) {
            stopSound(lowestPriorityId, 0.0f);
        } else {
            // No lower priority sound found, can't play
            return 0;
        }
    }
    
    // Create audio source
    std::shared_ptr<AudioSource> source;
    
    if (properties.is3D) {
        source = m_audioManager->play3DSound(resourceId, properties.x, properties.y, properties.z,
                                           properties.volume * getCategoryVolume(properties.category),
                                           properties.pitch, properties.loop);
    } else {
        source = m_audioManager->playSound(resourceId, 
                                         properties.volume * getCategoryVolume(properties.category),
                                         properties.pitch, properties.loop);
    }
    
    if (!source) {
        return 0;
    }
    
    // Apply 3D positioning if needed
    if (properties.is3D) {
        apply3DPositioning(source, properties);
    }
    
    // Generate unique ID and store sound
    uint32_t soundId = generateSoundId();
    m_activeSounds.emplace(soundId, ActiveSoundEffect(source, properties, resourceId, m_totalTime, !properties.loop));
    
    return soundId;
}

uint32_t SoundEffectManager::play2DSound(const std::string& resourceId, float volume, float pitch, 
                                        bool loop, SoundCategory category) {
    SoundEffectProperties props(volume, pitch, loop, category);
    return playSound(resourceId, props);
}

uint32_t SoundEffectManager::play3DSound(const std::string& resourceId, float x, float y, float z,
                                        float volume, float pitch, bool loop, SoundCategory category) {
    SoundEffectProperties props(volume, pitch, x, y, z, loop, category);
    return playSound(resourceId, props);
}

void SoundEffectManager::stopSound(uint32_t soundId, float fadeOutDuration) {
    auto it = m_activeSounds.find(soundId);
    if (it == m_activeSounds.end()) {
        return;
    }
    
    if (fadeOutDuration <= 0.0f) {
        // Immediate stop
        if (it->second.source) {
            it->second.source->stop();
        }
        
        // Fire completion callback
        if (m_completionCallback) {
            m_completionCallback(soundId, it->second.resourceId);
        }
        
        m_activeSounds.erase(it);
    } else {
        // TODO: Implement fade out for individual sounds
        // For now, just stop immediately
        stopSound(soundId, 0.0f);
    }
}

void SoundEffectManager::stopCategory(SoundCategory category, float fadeOutDuration) {
    std::vector<uint32_t> soundsToStop;
    
    // Collect sounds in category
    for (const auto& pair : m_activeSounds) {
        if (pair.second.properties.category == category) {
            soundsToStop.push_back(pair.first);
        }
    }
    
    // Stop collected sounds
    for (uint32_t soundId : soundsToStop) {
        stopSound(soundId, fadeOutDuration);
    }
}

void SoundEffectManager::stopAllSounds(float fadeOutDuration) {
    std::vector<uint32_t> soundsToStop;
    
    // Collect all sound IDs
    for (const auto& pair : m_activeSounds) {
        soundsToStop.push_back(pair.first);
    }
    
    // Stop all sounds
    for (uint32_t soundId : soundsToStop) {
        stopSound(soundId, fadeOutDuration);
    }
}

void SoundEffectManager::updateSoundPosition(uint32_t soundId, float x, float y, float z) {
    auto it = m_activeSounds.find(soundId);
    if (it == m_activeSounds.end() || !it->second.properties.is3D) {
        return;
    }
    
    // Update properties
    it->second.properties.x = x;
    it->second.properties.y = y;
    it->second.properties.z = z;
    
    // Update audio source position
    if (it->second.source && m_audioManager->getAudioDevice()) {
        m_audioManager->getAudioDevice()->setSourcePosition(it->second.source->getSourceHandle(), x, y, z);
    }
}

void SoundEffectManager::updateSoundVelocity(uint32_t soundId, float velocityX, float velocityY, float velocityZ) {
    auto it = m_activeSounds.find(soundId);
    if (it == m_activeSounds.end() || !it->second.properties.is3D) {
        return;
    }
    
    // Update properties
    it->second.properties.velocityX = velocityX;
    it->second.properties.velocityY = velocityY;
    it->second.properties.velocityZ = velocityZ;
    
    // Update audio source velocity
    if (it->second.source && m_audioManager->getAudioDevice()) {
        m_audioManager->getAudioDevice()->setSourceVelocity(it->second.source->getSourceHandle(), 
                                                           velocityX, velocityY, velocityZ);
    }
}

void SoundEffectManager::setCategoryVolume(SoundCategory category, float volume) {
    volume = std::max(0.0f, std::min(1.0f, volume));
    m_categoryVolumes[category] = volume;
    
    // Update all active sounds in this category
    for (auto& pair : m_activeSounds) {
        auto& soundEffect = pair.second;
        if (soundEffect.properties.category == category && soundEffect.source) {
            float finalVolume = soundEffect.properties.volume * volume;
            
            // Apply 3D volume if needed
            if (soundEffect.properties.is3D) {
                const auto& listener = m_audioManager->getListener();
                float volumeMultiplier = calculate3DVolume(soundEffect.properties, listener);
                finalVolume *= volumeMultiplier;
            }
            
            soundEffect.source->setVolume(finalVolume);
        }
    }
}

float SoundEffectManager::getCategoryVolume(SoundCategory category) const {
    auto it = m_categoryVolumes.find(category);
    return (it != m_categoryVolumes.end()) ? it->second : 1.0f;
}

void SoundEffectManager::setCategoryEnabled(SoundCategory category, bool enabled) {
    m_categoryEnabled[category] = enabled;
    
    if (!enabled) {
        // Stop all sounds in this category
        stopCategory(category, 0.0f);
    }
}

bool SoundEffectManager::isCategoryEnabled(SoundCategory category) const {
    auto it = m_categoryEnabled.find(category);
    return (it != m_categoryEnabled.end()) ? it->second : true;
}

void SoundEffectManager::setMaxConcurrentSounds(SoundCategory category, int maxSounds) {
    m_maxConcurrentSounds[category] = std::max(0, maxSounds);
    
    // If we're over the limit, stop lowest priority sounds
    int currentCount = getActiveSoundCount(category);
    if (currentCount > maxSounds) {
        std::vector<std::pair<uint32_t, float>> soundPriorities;
        
        // Collect sounds in this category with their priorities
        for (const auto& pair : m_activeSounds) {
            if (pair.second.properties.category == category) {
                soundPriorities.emplace_back(pair.first, pair.second.properties.priority);
            }
        }
        
        // Sort by priority (lowest first)
        std::sort(soundPriorities.begin(), soundPriorities.end(),
                 [](const auto& a, const auto& b) { return a.second < b.second; });
        
        // Stop excess sounds
        int soundsToStop = currentCount - maxSounds;
        for (int i = 0; i < soundsToStop && i < static_cast<int>(soundPriorities.size()); ++i) {
            stopSound(soundPriorities[i].first, 0.0f);
        }
    }
}

int SoundEffectManager::getActiveSoundCount(SoundCategory category) const {
    int count = 0;
    for (const auto& pair : m_activeSounds) {
        if (pair.second.properties.category == category) {
            count++;
        }
    }
    return count;
}

bool SoundEffectManager::isSoundPlaying(uint32_t soundId) const {
    auto it = m_activeSounds.find(soundId);
    return (it != m_activeSounds.end()) && it->second.source && it->second.source->isPlaying();
}

uint32_t SoundEffectManager::generateSoundId() {
    return m_nextSoundId++;
}

void SoundEffectManager::cleanupFinishedSounds() {
    std::vector<uint32_t> finishedSounds;
    
    // Find finished sounds
    for (const auto& pair : m_activeSounds) {
        const auto& soundEffect = pair.second;
        
        if (!soundEffect.source || soundEffect.source->isStopped()) {
            finishedSounds.push_back(pair.first);
        }
    }
    
    // Remove finished sounds
    for (uint32_t soundId : finishedSounds) {
        auto it = m_activeSounds.find(soundId);
        if (it != m_activeSounds.end()) {
            // Fire completion callback
            if (m_completionCallback) {
                m_completionCallback(soundId, it->second.resourceId);
            }
            
            m_activeSounds.erase(it);
        }
    }
}

void SoundEffectManager::apply3DPositioning(std::shared_ptr<AudioSource> source, const SoundEffectProperties& properties) {
    if (!source || !m_audioManager->getAudioDevice()) {
        return;
    }
    
    auto device = m_audioManager->getAudioDevice();
    unsigned int sourceHandle = source->getSourceHandle();
    
    // Set 3D position
    device->setSourcePosition(sourceHandle, properties.x, properties.y, properties.z);
    
    // Set velocity for Doppler effect
    device->setSourceVelocity(sourceHandle, properties.velocityX, properties.velocityY, properties.velocityZ);
    
    // TODO: Set distance model parameters (minDistance, maxDistance)
    // This would require extending the IAudioDevice interface
}

float SoundEffectManager::calculate3DVolume(const SoundEffectProperties& properties, const AudioListener& listener) {
    // Calculate distance from listener
    float dx = properties.x - listener.x;
    float dy = properties.y - listener.y;
    float dz = properties.z - listener.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // Apply distance attenuation
    if (distance <= properties.minDistance) {
        return 1.0f; // Full volume within minimum distance
    } else if (distance >= properties.maxDistance) {
        return 0.0f; // No volume beyond maximum distance
    } else {
        // Linear falloff between min and max distance
        float falloff = 1.0f - (distance - properties.minDistance) / (properties.maxDistance - properties.minDistance);
        return std::max(0.0f, falloff);
    }
}

} // namespace Audio
} // namespace RPGEngine