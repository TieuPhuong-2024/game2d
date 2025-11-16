#include "AnimationClip.h"
#include "../Utils/Logger.h"

AnimationClip::AnimationClip()
    : m_name("")
    , m_looping(true) {
}

AnimationClip::AnimationClip(const std::string& name, bool looping)
    : m_name(name)
    , m_looping(looping) {
}

AnimationClip::~AnimationClip() {
    ClearFrames();
}

void AnimationClip::AddFrame(const AnimationFrame& frame) {
    m_frames.push_back(frame);
}

void AnimationClip::AddFrame(const Rect& sourceRect, float duration, const Vector2& offset) {
    m_frames.push_back(AnimationFrame(sourceRect, duration, offset));
}

void AnimationClip::ClearFrames() {
    m_frames.clear();
}

const AnimationFrame& AnimationClip::GetFrame(int index) const {
    if (index < 0 || index >= static_cast<int>(m_frames.size())) {
        LOG_ERROR("AnimationClip::GetFrame - Index out of range: " + std::to_string(index));
        static AnimationFrame emptyFrame;
        return emptyFrame;
    }
    return m_frames[index];
}

float AnimationClip::GetTotalDuration() const {
    float total = 0.0f;
    for (const auto& frame : m_frames) {
        total += frame.duration;
    }
    return total;
}
