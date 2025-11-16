#pragma once
#include <string>
#include <map>
#include <memory>
#include "../Graphics/Texture2D.h"
#include "AnimationClip.h"
#include "../Utils/MathTypes.h"

// Manages a sprite sheet texture and provides frame extraction
class SpriteSheet {
public:
    SpriteSheet();
    ~SpriteSheet();
    
    // Loading
    bool LoadFromFile(LPDIRECT3DDEVICE9 device, const std::string& filename);
    bool LoadFromFile(LPDIRECT3DDEVICE9 device, const std::string& filename, int frameWidth, int frameHeight);
    void Release();
    
    // Frame extraction - creates rectangles for a grid-based sprite sheet
    void SetupGrid(int frameWidth, int frameHeight);
    Rect GetFrameRect(int frameIndex) const;
    Rect GetFrameRect(int row, int col) const;
    
    // Animation clip management
    void AddAnimationClip(const std::string& name, AnimationClip* clip);
    AnimationClip* GetAnimationClip(const std::string& name) const;
    bool HasAnimationClip(const std::string& name) const;
    
    // Accessors
    Texture2D* GetTexture() const { return m_texture.get(); }
    int GetFrameWidth() const { return m_frameWidth; }
    int GetFrameHeight() const { return m_frameHeight; }
    int GetColumns() const { return m_columns; }
    int GetRows() const { return m_rows; }
    int GetTotalFrames() const { return m_columns * m_rows; }
    
    bool IsValid() const { return m_texture && m_texture->IsValid(); }
    
private:
    std::unique_ptr<Texture2D> m_texture;
    int m_frameWidth;
    int m_frameHeight;
    int m_columns;
    int m_rows;
    
    // Animation clips associated with this sprite sheet
    std::map<std::string, AnimationClip*> m_animationClips;
    
    void CalculateGridDimensions();
};
