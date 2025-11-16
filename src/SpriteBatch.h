#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "Texture2D.h"
#include "D3DRenderer.h"

class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();
    
    // Initialization
    bool Initialize(LPDIRECT3DDEVICE9 device);
    void Shutdown();
    
    // Batch rendering
    void Begin();
    void Begin(const D3DXMATRIX* transform);
    void End();
    
    // Drawing methods
    void Draw(Texture2D* texture, const Vector2& position, const Color& tint = Color());
    void Draw(Texture2D* texture, const Vector2& position, const Rect& sourceRect, const Color& tint = Color());
    void Draw(Texture2D* texture, const Vector2& position, const Rect& sourceRect,
              const Vector2& scale, float rotation, const Vector2& origin, const Color& tint = Color());
    
    // State
    bool IsInBatch() const { return m_inBatch; }
    
private:
    LPDIRECT3DDEVICE9 m_device;
    LPD3DXSPRITE m_sprite;
    bool m_inBatch;
    
    // Helper methods
    D3DXVECTOR3 Vector2ToD3DXVECTOR3(const Vector2& vec, float z = 0.0f);
    RECT RectangleToRECT(const Rect& rect);
};