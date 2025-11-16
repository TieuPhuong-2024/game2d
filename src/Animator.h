#pragma once
#include <map>
#include <string>
#include <functional>
#include "AnimationClip.h"
#include "AnimationState.h"
#include "SpriteSheet.h"
#include "MathTypes.h"

// Forward declaration
class Texture2D;

// Animation event callback type
using AnimationEventCallback = std::function<void()>;

// Represents a transition condition between animation states
struct AnimationTransition {
    AnimationState fromState;
    AnimationState toState;
    std::function<bool()> condition;
    
    AnimationTransition()
        : fromState(AnimationState::None)
        , toState(AnimationState::None)
        , condition(nullptr) {
    }
    
    AnimationTransition(AnimationState from, AnimationState to, std::function<bool()> cond)
        : fromState(from)
        , toState(to)
        , condition(cond) {
    }
};

// Manages animation playback and state transitions
class Animator {
public:
    Animator();
    ~Animator();
    
    // Initialization
    void SetSpriteSheet(SpriteSheet* spriteSheet);
    SpriteSheet* GetSpriteSheet() const { return m_spriteSheet; }
    
    // Animation state management
    void AddAnimation(AnimationState state, AnimationClip* clip);
    void SetCurrentState(AnimationState state, bool forceRestart = false);
    AnimationState GetCurrentState() const { return m_currentState; }
    
    // Transitions
    void AddTransition(AnimationState fromState, AnimationState toState, std::function<bool()> condition);
    void ClearTransitions();
    
    // Animation events
    void AddAnimationEvent(AnimationState state, int frameIndex, AnimationEventCallback callback);
    void ClearAnimationEvents();
    
    // Update and rendering
    void Update(float deltaTime);
    Rect GetCurrentFrameRect() const;
    Vector2 GetCurrentFrameOffset() const;
    
    // Playback control
    void Play();
    void Pause();
    void Stop();
    void Reset();
    
    bool IsPlaying() const { return m_isPlaying; }
    bool IsPaused() const { return !m_isPlaying; }
    
    // Frame information
    int GetCurrentFrameIndex() const { return m_currentFrameIndex; }
    float GetCurrentTime() const { return m_currentTime; }
    float GetNormalizedTime() const; // 0.0 to 1.0
    
    // Animation properties
    void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }
    float GetPlaybackSpeed() const { return m_playbackSpeed; }
    
    bool HasAnimation(AnimationState state) const;
    AnimationClip* GetAnimation(AnimationState state) const;
    
private:
    SpriteSheet* m_spriteSheet;
    std::map<AnimationState, AnimationClip*> m_animations;
    std::vector<AnimationTransition> m_transitions;
    
    // Animation events: map of (state, frameIndex) -> callback
    std::map<std::pair<AnimationState, int>, AnimationEventCallback> m_animationEvents;
    
    AnimationState m_currentState;
    AnimationState m_previousState;
    int m_currentFrameIndex;
    float m_currentTime;
    float m_playbackSpeed;
    bool m_isPlaying;
    
    // Internal methods
    void UpdateTransitions();
    void UpdateAnimation(float deltaTime);
    void TriggerAnimationEvents();
    void ChangeState(AnimationState newState);
};
