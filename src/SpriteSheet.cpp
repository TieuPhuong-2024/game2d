#include "SpriteSheet.h"
#include "Logger.h"

SpriteSheet::SpriteSheet()
    : m_texture(nullptr)
    , m_frameWidth(0)
    , m_frameHeight(0)
    , m_columns(0)
    , m_rows(0) {
}

SpriteSheet::~SpriteSheet() {
    Release();
}

bool SpriteSheet::LoadFromFile(LPDIRECT3DDEVICE9 device, const std::string& filename) {
    if (!device) {
        LOG_ERROR("Invalid device passed to SpriteSheet::LoadFromFile");
        return false;
    }
    
    Release();
    
    m_texture = std::make_unique<Texture2D>();
    if (!m_texture->LoadFromFile(device, filename)) {
        LOG_ERROR("Failed to load sprite sheet texture: " + filename);
        m_texture.reset();
        return false;
    }
    
    // Default to single frame (entire texture)
    m_frameWidth = m_texture->GetWidth();
    m_frameHeight = m_texture->GetHeight();
    m_columns = 1;
    m_rows = 1;
    
    LOG_INFO("SpriteSheet loaded: " + filename);
    return true;
}

bool SpriteSheet::LoadFromFile(LPDIRECT3DDEVICE9 device, const std::string& filename, int frameWidth, int frameHeight) {
    if (!LoadFromFile(device, filename)) {
        return false;
    }
    
    SetupGrid(frameWidth, frameHeight);
    return true;
}

void SpriteSheet::Release() {
    // Clean up animation clips (we own them)
    for (auto& pair : m_animationClips) {
        delete pair.second;
    }
    m_animationClips.clear();
    
    if (m_texture) {
        m_texture->Release();
        m_texture.reset();
    }
    
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_columns = 0;
    m_rows = 0;
}

void SpriteSheet::SetupGrid(int frameWidth, int frameHeight) {
    if (!m_texture || !m_texture->IsValid()) {
        LOG_ERROR("Cannot setup grid on invalid sprite sheet");
        return;
    }
    
    if (frameWidth <= 0 || frameHeight <= 0) {
        LOG_ERROR("Invalid frame dimensions for sprite sheet grid");
        return;
    }
    
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    
    CalculateGridDimensions();
    
    LOG_INFO("SpriteSheet grid setup: " + std::to_string(m_columns) + "x" + std::to_string(m_rows) + 
             " (" + std::to_string(m_frameWidth) + "x" + std::to_string(m_frameHeight) + " per frame)");
}

Rect SpriteSheet::GetFrameRect(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= GetTotalFrames()) {
        LOG_ERROR("Frame index out of range: " + std::to_string(frameIndex));
        return Rect();
    }
    
    int row = frameIndex / m_columns;
    int col = frameIndex % m_columns;
    
    return GetFrameRect(row, col);
}

Rect SpriteSheet::GetFrameRect(int row, int col) const {
    if (row < 0 || row >= m_rows || col < 0 || col >= m_columns) {
        LOG_ERROR("Frame row/col out of range: (" + std::to_string(row) + ", " + std::to_string(col) + ")");
        return Rect();
    }
    
    return Rect(
        col * m_frameWidth,
        row * m_frameHeight,
        m_frameWidth,
        m_frameHeight
    );
}

void SpriteSheet::AddAnimationClip(const std::string& name, AnimationClip* clip) {
    if (!clip) {
        LOG_ERROR("Cannot add null animation clip");
        return;
    }
    
    // Check if clip already exists
    if (HasAnimationClip(name)) {
        LOG_WARNING("Animation clip already exists, replacing: " + name);
        delete m_animationClips[name];
    }
    
    m_animationClips[name] = clip;
    LOG_INFO("Added animation clip: " + name);
}

AnimationClip* SpriteSheet::GetAnimationClip(const std::string& name) const {
    auto it = m_animationClips.find(name);
    if (it != m_animationClips.end()) {
        return it->second;
    }
    return nullptr;
}

bool SpriteSheet::HasAnimationClip(const std::string& name) const {
    return m_animationClips.find(name) != m_animationClips.end();
}

void SpriteSheet::CalculateGridDimensions() {
    if (!m_texture || !m_texture->IsValid()) {
        m_columns = 0;
        m_rows = 0;
        return;
    }
    
    m_columns = m_texture->GetWidth() / m_frameWidth;
    m_rows = m_texture->GetHeight() / m_frameHeight;
    
    if (m_columns <= 0) m_columns = 1;
    if (m_rows <= 0) m_rows = 1;
}
