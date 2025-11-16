#pragma once
#include <vector>
#include <string>
#include "MathTypes.h"

// Represents a single frame in an animation
struct AnimationFrame {
    Rect sourceRect;      // Rectangle in the sprite sheet
    float duration;       // Duration in seconds
    Vector2 offset;       // Optional offset for positioning
    
    AnimationFrame()
        : sourceRect()
        , duration(0.1f)
        , offset(0.0f, 0.0f) {
    }
    
    AnimationFrame(const Rect& rect, float dur = 0.1f, const Vector2& off = Vector2(0.0f, 0.0f))
        : sourceRect(rect)
        , duration(dur)
        , offset(off) {
    }
};

// Represents a complete animation clip with multiple frames
class AnimationClip {
public:
    AnimationClip();
    AnimationClip(const std::string& name, bool looping = true);
    ~AnimationClip();
    
    // Frame management
    void AddFrame(const AnimationFrame& frame);
    void AddFrame(const Rect& sourceRect, float duration = 0.1f, const Vector2& offset = Vector2(0.0f, 0.0f));
    void ClearFrames();
    
    // Accessors
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    
    bool IsLooping() const { return m_looping; }
    void SetLooping(bool looping) { m_looping = looping; }
    
    int GetFrameCount() const { return static_cast<int>(m_frames.size()); }
    const AnimationFrame& GetFrame(int index) const;
    
    float GetTotalDuration() const;
    
    // Validation
    bool IsValid() const { return !m_frames.empty(); }
    
private:
    std::string m_name;
    std::vector<AnimationFrame> m_frames;
    bool m_looping;
};
