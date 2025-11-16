#include "Texture2D.h"

Texture2D::Texture2D()
    : m_texture(nullptr)
    , m_width(0)
    , m_height(0) {
}

Texture2D::~Texture2D() {
    Release();
}

bool Texture2D::LoadFromFile(LPDIRECT3DDEVICE9 device, const std::string& filename) {
    if (!device) {
        LOG_ERROR("Invalid device passed to Texture2D::LoadFromFile");
        return false;
    }
    
    // Release any existing texture
    Release();
    
    m_filename = filename;
    
    // Get image info first to retrieve dimensions
    D3DXIMAGE_INFO imageInfo;
    HRESULT hr = D3DXGetImageInfoFromFileA(filename.c_str(), &imageInfo);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get image info for: " + filename + ". HRESULT: " + std::to_string(hr));
        return false;
    }
    
    // Store dimensions
    m_width = imageInfo.Width;
    m_height = imageInfo.Height;
    
    // Load the texture
    hr = D3DXCreateTextureFromFileExA(
        device,
        filename.c_str(),
        imageInfo.Width,
        imageInfo.Height,
        1, // mip levels
        0, // usage
        D3DFMT_A8R8G8B8, // format
        D3DPOOL_MANAGED,
        D3DX_FILTER_POINT, // no filtering for pixel art
        D3DX_FILTER_POINT,
        0xFF000000, // color key for transparency (black)
        nullptr,
        nullptr,
        &m_texture
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to load texture from file: " + filename + ". HRESULT: " + std::to_string(hr));
        m_width = 0;
        m_height = 0;
        return false;
    }
    
    LOG_INFO("Texture loaded successfully: " + filename + " (" + std::to_string(m_width) + "x" + std::to_string(m_height) + ")");
    return true;
}

bool Texture2D::CreateFromMemory(LPDIRECT3DDEVICE9 device, const void* data, UINT dataSize) {
    if (!device || !data || dataSize == 0) {
        LOG_ERROR("Invalid parameters passed to Texture2D::CreateFromMemory");
        return false;
    }
    
    // Release any existing texture
    Release();
    
    m_filename = "[Memory]";
    
    // Get image info first to retrieve dimensions
    D3DXIMAGE_INFO imageInfo;
    HRESULT hr = D3DXGetImageInfoFromFileInMemory(data, dataSize, &imageInfo);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get image info from memory. HRESULT: " + std::to_string(hr));
        return false;
    }
    
    // Store dimensions
    m_width = imageInfo.Width;
    m_height = imageInfo.Height;
    
    // Load the texture from memory
    hr = D3DXCreateTextureFromFileInMemoryEx(
        device,
        data,
        dataSize,
        imageInfo.Width,
        imageInfo.Height,
        1, // mip levels
        0, // usage
        D3DFMT_A8R8G8B8, // format
        D3DPOOL_MANAGED,
        D3DX_FILTER_POINT, // no filtering for pixel art
        D3DX_FILTER_POINT,
        0xFF000000, // color key for transparency (black)
        nullptr,
        nullptr,
        &m_texture
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create texture from memory. HRESULT: " + std::to_string(hr));
        m_width = 0;
        m_height = 0;
        return false;
    }
    
    LOG_INFO("Texture created from memory successfully (" + std::to_string(m_width) + "x" + std::to_string(m_height) + ")");
    return true;
}

void Texture2D::Release() {
    if (m_texture) {
        m_texture->Release();
        m_texture = nullptr;
    }
    
    m_width = 0;
    m_height = 0;
    m_filename.clear();
}