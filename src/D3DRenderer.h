#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "Logger.h"
#include "MathTypes.h"

// Forward declarations
class Texture2D;
class Camera2D;
class SpriteBatch;

struct Color {
    DWORD value;
    Color() : value(0xFFFFFFFF) {}
    Color(DWORD argb) : value(argb) {}
    Color(BYTE r, BYTE g, BYTE b, BYTE a = 255) : value(D3DCOLOR_ARGB(a, r, g, b)) {}
};

class D3DRenderer {
public:
    D3DRenderer();
    ~D3DRenderer();
    
    // Core initialization and cleanup
    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();
    
    // Frame rendering
    void BeginFrame();
    void EndFrame();
    void Clear(Color clearColor = Color(0, 0, 64));
    
    // Device management
    bool IsDeviceLost() const;
    bool ResetDevice();
    LPDIRECT3DDEVICE9 GetDevice() const { return m_d3dDevice; }
    
    // Basic rendering
    void DrawSprite(Texture2D* texture, const Vector2& position, const Rect& sourceRect, const Color& tint = Color());
    void DrawSprite(Texture2D* texture, const Vector2& position, const Color& tint = Color());
    void DrawSprite(Texture2D* texture, const Vector2& position, const Rect& sourceRect, 
                    bool flipHorizontal, bool flipVertical = false, const Color& tint = Color());
    
    // Camera support
    void SetCamera(Camera2D* camera);
    Camera2D* GetCamera() const { return m_camera; }
    
    // Sprite batch access
    SpriteBatch* GetSpriteBatch() const { return m_spriteBatch; }
    
    // Viewport information
    int GetViewportWidth() const { return m_viewportWidth; }
    int GetViewportHeight() const { return m_viewportHeight; }
    
private:
    // DirectX components
    LPDIRECT3D9 m_d3d;
    LPDIRECT3DDEVICE9 m_d3dDevice;
    D3DPRESENT_PARAMETERS m_presentParams;
    
    // Window and viewport
    HWND m_hwnd;
    int m_viewportWidth;
    int m_viewportHeight;
    
    // Camera
    Camera2D* m_camera;
    
    // Sprite rendering
    SpriteBatch* m_spriteBatch;
    
    // Internal methods
    bool CreateDevice();
    void SetupRenderStates();
    void ApplyCameraTransform();
    void ResetCameraTransform();
};