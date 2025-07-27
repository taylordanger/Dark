#pragma once
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

struct AnimationFrame {
  int textureId; 
  float duration;   
};

struct AnimationSequence {
    std::string name;
    std::vector<AnimationFrame> frames;
    bool loop;
};

struct AnimationTransition {
    std::string fromState;
    std::string toState;
    std::function<bool()> condition;
    float blendDuration = 0.0f; // seconds
    int priority = 0; // higher = preferred
};

class AnimationComponent {
    public:
        AnimationComponent(); //noarg

    void addTransition(const AnimationTransition& transition);
    void setState(const std::string& state, float blendDuration = 0.0f);
    void addSequence(const AnimationSequence& sequence);
    void play(const std::string& name, bool restart = false);
    void update(float deltaTime);
    int getCurrentTextureId() const;
    bool isFinished() const;

    bool isBlending() const;
    float getBlendProgress() const;

    // Event callbacks
    std::function<void(const std::string&)> onAnimationEnd;
    std::function<void(int)> onFrameChange;

private:
    std::unordered_map<std::string, AnimationSequence> sequences;
    std::string currentSequence;
    size_t currentFrame;
    float frameTimer;
    bool finished;
     std::vector<AnimationTransition> transitions;
    std::string previousSequence;
    float blendTimer = 0.0f;
    float blendDuration = 0.0f;
    bool blending = false;
};