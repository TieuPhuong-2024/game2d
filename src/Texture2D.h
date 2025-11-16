#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "Logger.h"

class Texture2D {
public:
    Texture2D();
    ~Texture2D();
    
    // Loading and creation
    bool LoadFromFile(LPDIRECT3DDEVICE9 device, const std::string& filename);
    bool CreateFromMemory(LPDIRECT3DDEVICE9 device, const void* data, UINT dataSize);
    void Release();
    
    // Accessors
    LPDIRECT3DTEXTURE9 GetTexture() const { return m_texture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    bool IsValid() const { return m_texture != nullptr; }
    
    // Utility
    D3DXVECTOR2 GetSize() const { return D3DXVECTOR2(static_cast<float>(m_width), static_cast<float>(m_height)); }
    
private:
    LPDIRECT3DTEXTURE9 m_texture;
    int m_width;
    int m_height;
    std::string m_filename; // For debugging purposes
};