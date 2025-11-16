#include "SpriteBatch.h"

SpriteBatch::SpriteBatch()
    : m_device(nullptr)
    , m_sprite(nullptr)
    , m_inBatch(false) {
}

SpriteBatch::~SpriteBatch() {
    Shutdown();
}

bool SpriteBatch::Initialize(LPDIRECT3DDEVICE9 device) {
    if (!device) {
        LOG_ERROR("Invalid device passed to SpriteBatch::Initialize");
        return false;
    }
    
    m_device = device;
    
    // Create the D3DX sprite interface
    HRESULT hr = D3DXCreateSprite(device, &m_sprite);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create D3DX sprite interface. HRESULT: " + std::to_string(hr));
        return false;
    }
    
    LOG_INFO("SpriteBatch initialized successfully");
    return true;
}

void SpriteBatch::Shutdown() {
    if (m_sprite) {
        m_sprite->Release();
        m_sprite = nullptr;
    }
    
    m_device = nullptr;
    m_inBatch = false;
    
    LOG_INFO("SpriteBatch shutdown complete");
}

void SpriteBatch::Begin() {
    Begin(nullptr);
}

void SpriteBatch::Begin(const D3DXMATRIX* transform) {
    if (!m_sprite) {
        LOG_ERROR("SpriteBatch not initialized");
        return;
    }
    
    if (m_inBatch) {
        LOG_WARNING("SpriteBatch::Begin called while already in batch");
        return;
    }
    
    HRESULT hr = m_sprite->Begin(D3DXSPRITE_ALPHABLEND);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to begin sprite batch. HRESULT: " + std::to_string(hr));
        return;
    }
    
    // Apply transform if provided
    if (transform) {
        m_sprite->SetTransform(transform);
    }
    
    m_inBatch = true;
}

void SpriteBatch::End() {
    if (!m_sprite) {
        LOG_ERROR("SpriteBatch not initialized");
        return;
    }
    
    if (!m_inBatch) {
        LOG_WARNING("SpriteBatch::End called while not in batch");
        return;
    }
    
    HRESULT hr = m_sprite->End();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to end sprite batch. HRESULT: " + std::to_string(hr));
    }
    
    m_inBatch = false;
}

void SpriteBatch::Draw(Texture2D* texture, const Vector2& position, const Color& tint) {
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Invalid texture passed to SpriteBatch::Draw");
        return;
    }
    
    if (!m_inBatch) {
        LOG_ERROR("SpriteBatch::Draw called outside of Begin/End block");
        return;
    }
    
    D3DXVECTOR3 pos = Vector2ToD3DXVECTOR3(position);
    
    HRESULT hr = m_sprite->Draw(
        texture->GetTexture(),
        nullptr, // source rectangle (null = entire texture)
        nullptr, // center point (null = top-left)
        &pos,    // position
        tint.value
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to draw sprite. HRESULT: " + std::to_string(hr));
    }
}

void SpriteBatch::Draw(Texture2D* texture, const Vector2& position, const Rect& sourceRect, const Color& tint) {
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Invalid texture passed to SpriteBatch::Draw");
        return;
    }
    
    if (!m_inBatch) {
        LOG_ERROR("SpriteBatch::Draw called outside of Begin/End block");
        return;
    }
    
    D3DXVECTOR3 pos = Vector2ToD3DXVECTOR3(position);
    RECT srcRect = RectangleToRECT(sourceRect);
    
    HRESULT hr = m_sprite->Draw(
        texture->GetTexture(),
        &srcRect, // source rectangle
        nullptr,  // center point (null = top-left)
        &pos,     // position
        tint.value
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to draw sprite with source rectangle. HRESULT: " + std::to_string(hr));
    }
}

void SpriteBatch::Draw(Texture2D* texture, const Vector2& position, const Rect& sourceRect,
                      const Vector2& scale, float rotation, const Vector2& origin, const Color& tint) {
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Invalid texture passed to SpriteBatch::Draw");
        return;
    }
    
    if (!m_inBatch) {
        LOG_ERROR("SpriteBatch::Draw called outside of Begin/End block");
        return;
    }
    
    // Create transformation matrix
    D3DXMATRIX transform;
    D3DXVECTOR2 scaling(scale.x, scale.y);
    D3DXVECTOR2 rotationCenter(origin.x, origin.y);
    D3DXVECTOR2 translation(position.x, position.y);
    
    D3DXMatrixTransformation2D(
        &transform,
        nullptr,        // scaling center
        0.0f,          // scaling rotation
        &scaling,      // scaling
        &rotationCenter, // rotation center
        rotation,      // rotation angle
        &translation   // translation
    );
    
    // Set the transform
    m_sprite->SetTransform(&transform);
    
    D3DXVECTOR3 pos = Vector2ToD3DXVECTOR3(Vector2(0, 0)); // Position is handled by transform
    RECT srcRect = RectangleToRECT(sourceRect);
    D3DXVECTOR3 center = Vector2ToD3DXVECTOR3(origin);
    
    HRESULT hr = m_sprite->Draw(
        texture->GetTexture(),
        &srcRect, // source rectangle
        &center,  // center point for rotation
        &pos,     // position (0,0 since transform handles it)
        tint.value
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to draw transformed sprite. HRESULT: " + std::to_string(hr));
    }
    
    // Reset transform to identity
    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    m_sprite->SetTransform(&identity);
}

D3DXVECTOR3 SpriteBatch::Vector2ToD3DXVECTOR3(const Vector2& vec, float z) {
    return D3DXVECTOR3(vec.x, vec.y, z);
}

RECT SpriteBatch::RectangleToRECT(const Rect& rect) {
    RECT r;
    r.left = rect.x;
    r.top = rect.y;
    r.right = rect.x + rect.width;
    r.bottom = rect.y + rect.height;
    return r;
}