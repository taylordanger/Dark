#include "AnimationComponent.h"

AnimationComponent::AnimationComponent()
    : currentFrame(0), frameTimer(0.0f), finished(false) {}

void AnimationComponent::addTransition(const AnimationTransition& transition) {
    transitions.push_back(transition);
}
void AnimationComponent::addSequence(const AnimationSequence& sequence) {
    sequences[sequence.name] = sequence;
}
void AnimationComponent::setState(const std::string& state, float blendDur) {
    if (sequences.find(state) == sequences.end()) return;
    if (currentSequence != state) {
        previousSequence = currentSequence;
        currentSequence = state;
        currentFrame = 0;
        frameTimer = 0.0f;
        finished = false;
        blendDuration = blendDur;
        blendTimer = 0.0f;
        blending = blendDur > 0.0f;
    }
}
void AnimationComponent::play(const std::string& name, bool restart) {
    if (sequences.find(name) == sequences.end()) return;
    if (currentSequence != name || restart) {
        currentSequence = name;
        currentFrame = 0;
        frameTimer = 0.0f;
        finished = false;
    }
}

void AnimationComponent::update(float deltaTime) {
       // Check transitions (priority-based)
    AnimationTransition* best = nullptr;
    for (auto& t : transitions) {
        if (currentSequence == t.fromState && t.condition && t.condition()) {
            if (!best || t.priority > best->priority) best = &t;
        }
    }
    if (best) {
        setState(best->toState, best->blendDuration);
    }

    // Blending logic
    if (blending) {
        blendTimer += deltaTime;
        if (blendTimer >= blendDuration) {
            blending = false;
            blendTimer = 0.0f;
        }
    }

    if (finished || sequences.find(currentSequence) == sequences.end()) return;
    const AnimationSequence& seq = sequences.at(currentSequence);
    if (seq.frames.empty()) return;

    frameTimer += deltaTime;
    while (frameTimer >= seq.frames[currentFrame].duration) {
        frameTimer -= seq.frames[currentFrame].duration;
        currentFrame++;
        if (onFrameChange) onFrameChange(currentFrame);

        if (currentFrame >= seq.frames.size()) {
            if (seq.loop) {
                currentFrame = 0;
            } else {
                currentFrame = seq.frames.size() - 1;
                finished = true;
                if (onAnimationEnd) onAnimationEnd(currentSequence);
                break;
            }
        }
    }
}

bool AnimationComponent::isBlending() const { return blending; }

float AnimationComponent::getBlendProgress() const {
    return blendDuration > 0.0f ? blendTimer / blendDuration : 1.0f;
}

int AnimationComponent::getCurrentTextureId() const {
    if (sequences.find(currentSequence) == sequences.end()) return -1;
    const AnimationSequence& seq = sequences.at(currentSequence);
    if (seq.frames.empty()) return -1;
    return seq.frames[currentFrame].textureId;
}

bool AnimationComponent::isFinished() const {
    return finished;
}