#include "SpriteSheet.h"
#include "../Utils/Logger.h"
#include "../Utils/tinyxml2.h"

SpriteSheet::SpriteSheet()
    : m_texture(nullptr)
    , m_frameWidth(0)
    , m_frameHeight(0)
    , m_columns(0)
    , m_rows(0)
    , m_isXmlBased(false) {
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
    m_isXmlBased = false;
    m_namedFrames.clear();
    m_frames.clear();
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
    if (m_isXmlBased) {
        if (frameIndex < 0 || frameIndex >= static_cast<int>(m_frames.size())) {
            LOG_ERROR("Frame index out of range: " + std::to_string(frameIndex));
            return Rect();
        }
        return m_frames[frameIndex];
    }
    
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

int SpriteSheet::GetTotalFrames() const {
    if (m_isXmlBased) {
        return static_cast<int>(m_frames.size());
    }
    return m_columns * m_rows;
}

Rect SpriteSheet::GetFrameRectByName(const std::string& frameName) const {
    auto it = m_namedFrames.find(frameName);
    if (it != m_namedFrames.end()) {
        return it->second;
    }
    LOG_ERROR("Frame not found: " + frameName);
    return Rect();
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

bool SpriteSheet::LoadFromXML(LPDIRECT3DDEVICE9 device, const std::string& xmlPath) {
    if (!device) {
        LOG_ERROR("Invalid device passed to SpriteSheet::LoadFromXML");
        return false;
    }
    
    Release();
    
    // Parse XML file
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(xmlPath.c_str());
    
    if (error != tinyxml2::XML_SUCCESS) {
        LOG_ERROR("Failed to load XML file: " + xmlPath + " Error: " + doc.ErrorName());
        return false;
    }
    
    // Get root element
    tinyxml2::XMLElement* root = doc.FirstChildElement("TextureAtlas");
    if (!root) {
        LOG_ERROR("Invalid XML format: missing TextureAtlas element");
        return false;
    }
    
    // Get image path attribute
    const char* imagePath = root->Attribute("imagePath");
    if (!imagePath) {
        LOG_ERROR("Invalid XML format: missing imagePath attribute");
        return false;
    }
    
    // Extract directory from XML path
    std::string xmlDir = xmlPath;
    size_t lastSlash = xmlDir.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        xmlDir = xmlDir.substr(0, lastSlash + 1);
    } else {
        xmlDir = "";
    }
    
    // Load texture
    std::string texturePath = xmlDir + imagePath;
    m_texture = std::make_unique<Texture2D>();
    if (!m_texture->LoadFromFile(device, texturePath)) {
        LOG_ERROR("Failed to load sprite sheet texture: " + texturePath);
        m_texture.reset();
        return false;
    }
    
    // Parse sprite elements
    m_isXmlBased = true;
    int frameIndex = 0;
    
    for (tinyxml2::XMLElement* sprite = root->FirstChildElement("sprite"); 
         sprite != nullptr; 
         sprite = sprite->NextSiblingElement("sprite")) {
        
        const char* name = sprite->Attribute("n");
        if (!name) continue;
        
        int x = sprite->IntAttribute("x", 0);
        int y = sprite->IntAttribute("y", 0);
        int w = sprite->IntAttribute("w", 0);
        int h = sprite->IntAttribute("h", 0);
        
        // Check if sprite is rotated
        const char* rotated = sprite->Attribute("r");
        bool isRotated = (rotated && strcmp(rotated, "y") == 0);
        
        // For rotated sprites, swap width and height in the source rect
        // Note: You may need to handle rotation in rendering code
        Rect frameRect;
        if (isRotated) {
            frameRect = Rect(x, y, h, w); // Swap w and h for rotated sprites
        } else {
            frameRect = Rect(x, y, w, h);
        }
        
        // Store frame by name and index
        m_namedFrames[name] = frameRect;
        m_frames.push_back(frameRect);
        
        frameIndex++;
    }
    
    if (m_frames.empty()) {
        LOG_ERROR("No sprites found in XML file: " + xmlPath);
        Release();
        return false;
    }
    
    LOG_INFO("SpriteSheet loaded from XML: " + xmlPath + " (" + std::to_string(m_frames.size()) + " frames)");
    return true;
}

// Helper function to create animation clip from XML sprite sequence
// Automatically groups sprites by prefix (e.g., "idle-0.png", "idle-1.png" -> "idle")
void SpriteSheet::CreateAnimationFromXMLSequence(const std::string& prefix, float frameDuration) {
    if (!m_isXmlBased) {
        LOG_WARNING("CreateAnimationFromXMLSequence only works with XML-based sprite sheets");
        return;
    }
    
    AnimationClip* clip = new AnimationClip();
    int frameCount = 0;
    
    // Find all frames matching the prefix
    for (const auto& pair : m_namedFrames) {
        const std::string& frameName = pair.first;
        
        // Check if frame name starts with prefix
        if (frameName.find(prefix) == 0) {
            clip->AddFrame(pair.second, frameDuration);
            frameCount++;
        }
    }
    
    if (frameCount > 0) {
        // Remove file extension from prefix for animation name
        std::string animName = prefix;
        size_t dashPos = animName.find('-');
        if (dashPos != std::string::npos) {
            animName = animName.substr(0, dashPos);
        }
        
        AddAnimationClip(animName, clip);
        LOG_INFO("Created animation '" + animName + "' with " + std::to_string(frameCount) + " frames");
    } else {
        delete clip;
        LOG_WARNING("No frames found for prefix: " + prefix);
    }
}
