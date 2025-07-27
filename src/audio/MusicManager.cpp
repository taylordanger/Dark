#include "MusicManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace RPGEngine {
namespace Audio {

MusicManager::MusicManager(std::shared_ptr<AudioManager> audioManager)
    : m_audioManager(audioManager)
    , m_state(MusicState::Stopped)
    , m_currentVolume(1.0f)
    , m_crossfadeTime(0.0f)
    , m_crossfadeDuration(0.0f)
    , m_fadeTime(0.0f)
    , m_fadeDuration(0.0f)
    , m_fadeStartVolume(0.0f)
    , m_fadeTargetVolume(0.0f)
    , m_fadeToStop(false)
    , m_fadeToResume(false)
{
    // Default crossfade curve (smooth S-curve)
    m_crossfadeCurve = [](float t) {
        return t * t * (3.0f - 2.0f * t); // Smoothstep function
    };
}

MusicManager::~MusicManager() {
    stopMusic(0.0f);
}

void MusicManager::update(float deltaTime) {
    // Update crossfading
    if (m_state == MusicState::Crossfading) {
        updateCrossfading(deltaTime);
    }
    
    // Update fading
    if (m_state == MusicState::FadingIn || m_state == MusicState::FadingOut) {
        updateFading(deltaTime);
    }
    
    // Process queue if not busy with transitions
    if (m_state == MusicState::Stopped || m_state == MusicState::Playing) {
        processQueue();
    }
    
    // Check if current music has finished (for non-looping tracks)
    if (m_currentSource && m_state == MusicState::Playing) {
        if (m_currentSource->isStopped() && !m_currentTrack.loop) {
            m_state = MusicState::Stopped;
            m_currentSource = nullptr;
            processQueue(); // Try to play next track in queue
        }
    }
}

void MusicManager::playMusic(const MusicTrack& track, MusicTransitionType transitionType, 
                            float transitionDuration, std::function<void()> onComplete) {
    // Add to queue for processing
    queueMusic(track, transitionType, transitionDuration, onComplete);
}

void MusicManager::playMusic(const std::string& resourceId, float volume, bool loop,
                            MusicTransitionType transitionType, float transitionDuration) {
    MusicTrack track(resourceId, volume, loop);
    playMusic(track, transitionType, transitionDuration);
}

void MusicManager::stopMusic(float fadeOutDuration) {
    if (m_state == MusicState::Stopped) {
        return;
    }
    
    // Clear queue
    clearQueue();
    
    if (fadeOutDuration <= 0.0f) {
        // Immediate stop
        if (m_currentSource) {
            m_currentSource->stop();
            m_currentSource = nullptr;
        }
        if (m_nextSource) {
            m_nextSource->stop();
            m_nextSource = nullptr;
        }
        m_state = MusicState::Stopped;
    } else {
        // Fade out
        m_fadeTime = 0.0f;
        m_fadeDuration = fadeOutDuration;
        m_fadeStartVolume = m_currentVolume;
        m_fadeTargetVolume = 0.0f;
        m_fadeToStop = true;
        m_fadeToResume = false;
        m_state = MusicState::FadingOut;
    }
}

void MusicManager::pauseMusic(float fadeOutDuration) {
    if (m_state != MusicState::Playing && m_state != MusicState::Crossfading) {
        return;
    }
    
    if (fadeOutDuration <= 0.0f) {
        // Immediate pause
        if (m_currentSource) {
            m_currentSource->pause();
        }
        if (m_nextSource) {
            m_nextSource->pause();
        }
        m_state = MusicState::Paused;
    } else {
        // Fade out to pause
        m_fadeTime = 0.0f;
        m_fadeDuration = fadeOutDuration;
        m_fadeStartVolume = m_currentVolume;
        m_fadeTargetVolume = 0.0f;
        m_fadeToStop = false;
        m_fadeToResume = false;
        m_state = MusicState::FadingOut;
    }
}

void MusicManager::resumeMusic(float fadeInDuration) {
    if (m_state != MusicState::Paused) {
        return;
    }
    
    if (fadeInDuration <= 0.0f) {
        // Immediate resume
        if (m_currentSource) {
            m_currentSource->play(m_currentTrack.loop);
        }
        m_state = MusicState::Playing;
    } else {
        // Fade in from pause
        if (m_currentSource) {
            m_currentSource->play(m_currentTrack.loop);
        }
        
        m_fadeTime = 0.0f;
        m_fadeDuration = fadeInDuration;
        m_fadeStartVolume = 0.0f;
        m_fadeTargetVolume = m_currentTrack.volume;
        m_fadeToStop = false;
        m_fadeToResume = true;
        m_state = MusicState::FadingIn;
    }
}

void MusicManager::queueMusic(const MusicTrack& track, MusicTransitionType transitionType,
                             float transitionDuration, std::function<void()> onComplete) {
    m_musicQueue.emplace(track, transitionType, transitionDuration, onComplete);
}

void MusicManager::clearQueue() {
    // Clear the queue
    while (!m_musicQueue.empty()) {
        m_musicQueue.pop();
    }
}

void MusicManager::skipToNext() {
    if (!m_musicQueue.empty()) {
        // Stop current music immediately and process next
        if (m_currentSource) {
            m_currentSource->stop();
            m_currentSource = nullptr;
        }
        if (m_nextSource) {
            m_nextSource->stop();
            m_nextSource = nullptr;
        }
        m_state = MusicState::Stopped;
        processQueue();
    }
}

const MusicTrack* MusicManager::getCurrentTrack() const {
    if (m_state == MusicState::Stopped) {
        return nullptr;
    }
    return &m_currentTrack;
}

void MusicManager::setVolume(float volume, float fadeDuration) {
    volume = std::max(0.0f, std::min(1.0f, volume));
    
    if (fadeDuration <= 0.0f) {
        // Immediate volume change
        m_currentVolume = volume;
        if (m_currentSource) {
            m_currentSource->setVolume(volume);
        }
    } else {
        // Fade to new volume
        m_fadeTime = 0.0f;
        m_fadeDuration = fadeDuration;
        m_fadeStartVolume = m_currentVolume;
        m_fadeTargetVolume = volume;
        m_fadeToStop = false;
        m_fadeToResume = false;
        
        // Don't change state if we're already playing
        if (m_state == MusicState::Playing) {
            m_state = MusicState::FadingIn; // Use FadingIn for volume changes
        }
    }
}

void MusicManager::processQueue() {
    if (m_musicQueue.empty()) {
        return;
    }
    
    auto entry = m_musicQueue.front();
    m_musicQueue.pop();
    
    switch (entry.transitionType) {
        case MusicTransitionType::Immediate: {
            // Stop current music immediately
            if (m_currentSource) {
                m_currentSource->stop();
            }
            
            // Play new music
            m_currentTrack = entry.track;
            m_currentSource = m_audioManager->playMusic(entry.track.resourceId, entry.track.volume, entry.track.loop);
            
            if (m_currentSource) {
                m_currentVolume = entry.track.volume;
                m_state = MusicState::Playing;
                
                if (entry.onComplete) {
                    entry.onComplete();
                }
            }
            break;
        }
        
        case MusicTransitionType::Crossfade: {
            if (m_currentSource && m_state == MusicState::Playing) {
                // Start crossfade
                m_nextTrack = entry.track;
                m_nextSource = m_audioManager->playMusic(entry.track.resourceId, 0.0f, entry.track.loop);
                
                if (m_nextSource) {
                    m_crossfadeTime = 0.0f;
                    m_crossfadeDuration = entry.transitionDuration;
                    m_crossfadeCallback = entry.onComplete;
                    m_state = MusicState::Crossfading;
                }
            } else {
                // No current music, just play new track
                m_currentTrack = entry.track;
                m_currentSource = m_audioManager->playMusic(entry.track.resourceId, entry.track.volume, entry.track.loop);
                
                if (m_currentSource) {
                    m_currentVolume = entry.track.volume;
                    m_state = MusicState::Playing;
                    
                    if (entry.onComplete) {
                        entry.onComplete();
                    }
                }
            }
            break;
        }
        
        case MusicTransitionType::FadeOut: {
            if (m_currentSource && m_state == MusicState::Playing) {
                // Fade out current, then play new
                m_nextTrack = entry.track;
                m_fadeTime = 0.0f;
                m_fadeDuration = entry.transitionDuration * 0.5f; // Half duration for fade out
                m_fadeStartVolume = m_currentVolume;
                m_fadeTargetVolume = 0.0f;
                m_fadeToStop = false;
                m_fadeToResume = false;
                m_crossfadeCallback = entry.onComplete;
                m_state = MusicState::FadingOut;
            } else {
                // No current music, just play new track
                m_currentTrack = entry.track;
                m_currentSource = m_audioManager->playMusic(entry.track.resourceId, entry.track.volume, entry.track.loop);
                
                if (m_currentSource) {
                    m_currentVolume = entry.track.volume;
                    m_state = MusicState::Playing;
                    
                    if (entry.onComplete) {
                        entry.onComplete();
                    }
                }
            }
            break;
        }
        
        case MusicTransitionType::FadeIn: {
            // Stop current music immediately, fade in new
            if (m_currentSource) {
                m_currentSource->stop();
            }
            
            m_currentTrack = entry.track;
            m_currentSource = m_audioManager->playMusic(entry.track.resourceId, 0.0f, entry.track.loop);
            
            if (m_currentSource) {
                m_fadeTime = 0.0f;
                m_fadeDuration = entry.transitionDuration;
                m_fadeStartVolume = 0.0f;
                m_fadeTargetVolume = entry.track.volume;
                m_fadeToStop = false;
                m_fadeToResume = false;
                m_crossfadeCallback = entry.onComplete;
                m_state = MusicState::FadingIn;
            }
            break;
        }
    }
}

void MusicManager::updateCrossfading(float deltaTime) {
    m_crossfadeTime += deltaTime;
    
    if (m_crossfadeTime >= m_crossfadeDuration) {
        // Crossfade complete
        if (m_currentSource) {
            m_currentSource->stop();
        }
        
        m_currentTrack = m_nextTrack;
        m_currentSource = m_nextSource;
        m_currentVolume = m_nextTrack.volume;
        m_nextSource = nullptr;
        
        if (m_currentSource) {
            m_currentSource->setVolume(m_currentVolume);
        }
        
        m_state = MusicState::Playing;
        
        if (m_crossfadeCallback) {
            m_crossfadeCallback();
            m_crossfadeCallback = nullptr;
        }
    } else {
        // Update crossfade volumes
        float t = m_crossfadeTime / m_crossfadeDuration;
        float curvedT = applyCrossfadeCurve(t);
        
        float currentVol = m_currentVolume * (1.0f - curvedT);
        float nextVol = m_nextTrack.volume * curvedT;
        
        if (m_currentSource) {
            m_currentSource->setVolume(currentVol);
        }
        if (m_nextSource) {
            m_nextSource->setVolume(nextVol);
        }
    }
}

void MusicManager::updateFading(float deltaTime) {
    m_fadeTime += deltaTime;
    
    if (m_fadeTime >= m_fadeDuration) {
        // Fade complete
        m_currentVolume = m_fadeTargetVolume;
        
        if (m_currentSource) {
            m_currentSource->setVolume(m_currentVolume);
        }
        
        if (m_fadeToStop) {
            // Stop after fade out
            if (m_currentSource) {
                m_currentSource->stop();
                m_currentSource = nullptr;
            }
            m_state = MusicState::Stopped;
        } else if (m_state == MusicState::FadingOut && !m_fadeToStop) {
            // Pause after fade out
            if (m_currentSource) {
                m_currentSource->pause();
            }
            m_state = MusicState::Paused;
        } else {
            // Fade in complete or volume change complete
            m_state = MusicState::Playing;
        }
        
        // Check if we need to start next track after fade out
        if (m_state == MusicState::Stopped && !m_nextTrack.resourceId.empty()) {
            m_currentTrack = m_nextTrack;
            m_currentSource = m_audioManager->playMusic(m_nextTrack.resourceId, 0.0f, m_nextTrack.loop);
            
            if (m_currentSource) {
                // Start fade in
                m_fadeTime = 0.0f;
                m_fadeDuration = m_fadeDuration; // Use same duration for fade in
                m_fadeStartVolume = 0.0f;
                m_fadeTargetVolume = m_nextTrack.volume;
                m_state = MusicState::FadingIn;
            }
            
            m_nextTrack = MusicTrack(); // Clear next track
        }
        
        if (m_crossfadeCallback) {
            m_crossfadeCallback();
            m_crossfadeCallback = nullptr;
        }
    } else {
        // Update fade volume
        float t = m_fadeTime / m_fadeDuration;
        m_currentVolume = m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * t;
        
        if (m_currentSource) {
            m_currentSource->setVolume(m_currentVolume);
        }
    }
}

float MusicManager::applyCrossfadeCurve(float t) {
    if (m_crossfadeCurve) {
        return m_crossfadeCurve(t);
    }
    return t; // Linear fallback
}

} // namespace Audio
} // namespace RPGEngine