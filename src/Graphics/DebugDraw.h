#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "../Utils/MathTypes.h"

// Simple debug drawing utility for colored rectangles
class DebugDraw {
public:
    DebugDraw();
    ~DebugDraw();
    
    bool Initialize(LPDIRECT3DDEVICE9 device);
    void Shutdown();
    
    // Draw a filled rectangle
    void DrawRect(const Rect& rect, D3DCOLOR color);
    
    // Draw a filled rectangle with float coordinates
    void DrawRect(float x, float y, float width, float height, D3DCOLOR color);
    
private:
    LPDIRECT3DDEVICE9 m_device;
    
    struct ColorVertex {
        float x, y, z, rhw;
        D3DCOLOR color;
        static const DWORD FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
    };
};
