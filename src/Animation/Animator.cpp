#include "Animator.h"
#include "../Utils/Logger.h"

Animator::Animator()
    : m_spriteSheet(nullptr)
    , m_currentState(AnimationState::None)
    , m_previousState(AnimationState::None)
    , m_currentFrameIndex(0)
    , m_currentTime(0.0f)
    , m_playbackSpeed(1.0f)
    , m_isPlaying(false) {
}

Animator::~Animator() {
    // Note: We don't own the sprite sheet or animation clips
    // They are managed by the SpriteSheet
}

void Animator::SetSpriteSheet(SpriteSheet* spriteSheet) {
    m_spriteSheet = spriteSheet;
}

void Animator::AddAnimation(AnimationState state, AnimationClip* clip) {
    if (!clip) {
        LOG_ERROR("Cannot add null animation clip");
        return;
    }
    
    m_animations[state] = clip;
}

void Animator::SetCurrentState(AnimationState state, bool forceRestart) {
    if (state == m_currentState && !forceRestart) {
        return;
    }
    
    if (!HasAnimation(state)) {
        LOG_WARNING("Animation state not found: " + AnimationStateHelper::ToString(state));
        return;
    }
    
    ChangeState(state);
}

void Animator::AddTransition(AnimationState fromState, AnimationState toState, std::function<bool()> condition) {
    if (!condition) {
        LOG_ERROR("Cannot add transition with null condition");
        return;
    }
    
    m_transitions.push_back(AnimationTransition(fromState, toState, condition));
}

void Animator::ClearTransitions() {
    m_transitions.clear();
}

void Animator::AddAnimationEvent(AnimationState state, int frameIndex, AnimationEventCallback callback) {
    if (!callback) {
        LOG_ERROR("Cannot add animation event with null callback");
        return;
    }
    
    m_animationEvents[std::make_pair(state, frameIndex)] = callback;
}

void Animator::ClearAnimationEvents() {
    m_animationEvents.clear();
}

void Animator::Update(float deltaTime) {
    if (!m_isPlaying) {
        return;
    }
    
    // Check for state transitions
    UpdateTransitions();
    
    // Update current animation
    UpdateAnimation(deltaTime * m_playbackSpeed);
    
    // Trigger any animation events for current frame
    TriggerAnimationEvents();
}

Rect Animator::GetCurrentFrameRect() const {
    AnimationClip* currentClip = GetAnimation(m_currentState);
    if (!currentClip || !currentClip->IsValid()) {
        return Rect();
    }
    
    const AnimationFrame& frame = currentClip->GetFrame(m_currentFrameIndex);
    return frame.sourceRect;
}

Vector2 Animator::GetCurrentFrameOffset() const {
    AnimationClip* currentClip = GetAnimation(m_currentState);
    if (!currentClip || !currentClip->IsValid()) {
        return Vector2(0.0f, 0.0f);
    }
    
    const AnimationFrame& frame = currentClip->GetFrame(m_currentFrameIndex);
    return frame.offset;
}

void Animator::Play() {
    m_isPlaying = true;
}

void Animator::Pause() {
    m_isPlaying = false;
}

void Animator::Stop() {
    m_isPlaying = false;
    m_currentFrameIndex = 0;
    m_currentTime = 0.0f;
}

void Animator::Reset() {
    m_currentFrameIndex = 0;
    m_currentTime = 0.0f;
}

float Animator::GetNormalizedTime() const {
    AnimationClip* currentClip = GetAnimation(m_currentState);
    if (!currentClip || !currentClip->IsValid()) {
        return 0.0f;
    }
    
    float totalDuration = currentClip->GetTotalDuration();
    if (totalDuration <= 0.0f) {
        return 0.0f;
    }
    
    return m_currentTime / totalDuration;
}

bool Animator::HasAnimation(AnimationState state) const {
    return m_animations.find(state) != m_animations.end();
}

AnimationClip* Animator::GetAnimation(AnimationState state) const {
    auto it = m_animations.find(state);
    if (it != m_animations.end()) {
        return it->second;
    }
    return nullptr;
}

void Animator::UpdateTransitions() {
    for (const auto& transition : m_transitions) {
        // Check if this transition applies to current state
        if (transition.fromState != m_currentState) {
            continue;
        }
        
        // Check if condition is met
        if (transition.condition && transition.condition()) {
            SetCurrentState(transition.toState);
            break; // Only process one transition per frame
        }
    }
}

void Animator::UpdateAnimation(float deltaTime) {
    AnimationClip* currentClip = GetAnimation(m_currentState);
    if (!currentClip || !currentClip->IsValid()) {
        return;
    }
    
    m_currentTime += deltaTime;
    
    // Calculate which frame we should be on
    float accumulatedTime = 0.0f;
    int frameCount = currentClip->GetFrameCount();
    
    for (int i = 0; i < frameCount; ++i) {
        const AnimationFrame& frame = currentClip->GetFrame(i);
        accumulatedTime += frame.duration;
        
        if (m_currentTime < accumulatedTime) {
            m_currentFrameIndex = i;
            return;
        }
    }
    
    // Animation finished
    if (currentClip->IsLooping()) {
        // Loop back to start
        float totalDuration = currentClip->GetTotalDuration();
        if (totalDuration > 0.0f) {
            m_currentTime = fmodf(m_currentTime, totalDuration);
        }
        m_currentFrameIndex = 0;
    } else {
        // Stay on last frame
        m_currentFrameIndex = frameCount - 1;
        m_currentTime = currentClip->GetTotalDuration();
    }
}

void Animator::TriggerAnimationEvents() {
    auto eventKey = std::make_pair(m_currentState, m_currentFrameIndex);
    auto it = m_animationEvents.find(eventKey);
    
    if (it != m_animationEvents.end() && it->second) {
        it->second();
    }
}

void Animator::ChangeState(AnimationState newState) {
    m_previousState = m_currentState;
    m_currentState = newState;
    m_currentFrameIndex = 0;
    m_currentTime = 0.0f;
    
    LOG_INFO("Animation state changed: " + AnimationStateHelper::ToString(m_previousState) + 
             " -> " + AnimationStateHelper::ToString(m_currentState));
}
