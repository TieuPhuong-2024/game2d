#include "D3DRenderer.h"
#include "Camera2D.h"
#include "SpriteBatch.h"
#include "Texture2D.h"

D3DRenderer::D3DRenderer()
    : m_d3d(nullptr)
    , m_d3dDevice(nullptr)
    , m_hwnd(nullptr)
    , m_viewportWidth(0)
    , m_viewportHeight(0)
    , m_camera(nullptr)
    , m_spriteBatch(nullptr) {
    ZeroMemory(&m_presentParams, sizeof(m_presentParams));
}

D3DRenderer::~D3DRenderer() {
    Shutdown();
}

bool D3DRenderer::Initialize(HWND hwnd, int width, int height) {
    LOG_INFO("Initializing D3DRenderer...");
    
    m_hwnd = hwnd;
    m_viewportWidth = width;
    m_viewportHeight = height;
    
    // Create Direct3D interface
    m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_d3d) {
        LOG_ERROR("Failed to create Direct3D interface");
        return false;
    }
    
    // Create the device
    if (!CreateDevice()) {
        LOG_ERROR("Failed to create Direct3D device");
        return false;
    }
    
    // Set up render states for 2D rendering
    SetupRenderStates();
    
    // Create sprite batch for efficient 2D rendering
    m_spriteBatch = new SpriteBatch();
    if (!m_spriteBatch->Initialize(m_d3dDevice)) {
        LOG_ERROR("Failed to initialize SpriteBatch");
        delete m_spriteBatch;
        m_spriteBatch = nullptr;
        return false;
    }
    
    LOG_INFO("D3DRenderer initialized successfully");
    return true;
}

bool D3DRenderer::CreateDevice() {
    // Set up presentation parameters
    ZeroMemory(&m_presentParams, sizeof(m_presentParams));
    m_presentParams.Windowed = TRUE;
    m_presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_presentParams.BackBufferFormat = D3DFMT_UNKNOWN;
    m_presentParams.BackBufferCount = 1;
    m_presentParams.BackBufferWidth = m_viewportWidth;
    m_presentParams.BackBufferHeight = m_viewportHeight;
    m_presentParams.hDeviceWindow = m_hwnd;
    m_presentParams.EnableAutoDepthStencil = TRUE;
    m_presentParams.AutoDepthStencilFormat = D3DFMT_D16;
    m_presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // VSync for 60 FPS
    
    // Create Direct3D device
    HRESULT hr = m_d3d->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        m_hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &m_presentParams,
        &m_d3dDevice
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create Direct3D device. HRESULT: " + std::to_string(hr));
        return false;
    }
    
    return true;
}

void D3DRenderer::SetupRenderStates() {
    if (!m_d3dDevice) return;
    
    // Disable lighting for 2D rendering
    m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    
    // Enable alpha blending for sprite transparency
    m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    
    // Set up texture filtering
    m_d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    m_d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    m_d3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    
    // Set up texture addressing
    m_d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    m_d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

void D3DRenderer::Shutdown() {
    LOG_INFO("Shutting down D3DRenderer...");
    
    // Shutdown sprite batch
    if (m_spriteBatch) {
        m_spriteBatch->Shutdown();
        delete m_spriteBatch;
        m_spriteBatch = nullptr;
    }
    
    // Release DirectX resources
    if (m_d3dDevice) {
        m_d3dDevice->Release();
        m_d3dDevice = nullptr;
    }
    
    if (m_d3d) {
        m_d3d->Release();
        m_d3d = nullptr;
    }
    
    m_camera = nullptr; // Don't delete, just clear reference
    
    LOG_INFO("D3DRenderer shutdown complete");
}

void D3DRenderer::BeginFrame() {
    if (!m_d3dDevice) return;
    
    // Check for device loss before rendering
    if (IsDeviceLost()) {
        LOG_WARNING("Device lost at BeginFrame, attempting reset");
        if (!ResetDevice()) {
            LOG_ERROR("Failed to reset device in BeginFrame");
            return;
        }
    }
    
    // Begin the scene
    HRESULT hr = m_d3dDevice->BeginScene();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to begin scene. HRESULT: " + std::to_string(hr));
    }
}

void D3DRenderer::EndFrame() {
    if (!m_d3dDevice) return;
    
    // End the scene
    HRESULT hr = m_d3dDevice->EndScene();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to end scene. HRESULT: " + std::to_string(hr));
        return;
    }
    
    // Present the back buffer to the screen
    hr = m_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
    
    // Handle device loss during present
    if (hr == D3DERR_DEVICELOST) {
        LOG_WARNING("Device lost during present");
    } else if (FAILED(hr)) {
        LOG_ERROR("Failed to present frame. HRESULT: " + std::to_string(hr));
    }
}

void D3DRenderer::Clear(Color clearColor) {
    if (!m_d3dDevice) return;
    
    HRESULT hr = m_d3dDevice->Clear(
        0, nullptr, 
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
        clearColor.value, 
        1.0f, 0
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to clear render target. HRESULT: " + std::to_string(hr));
    }
}

bool D3DRenderer::IsDeviceLost() const {
    if (!m_d3dDevice) return true;
    
    HRESULT hr = m_d3dDevice->TestCooperativeLevel();
    return (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET);
}

bool D3DRenderer::ResetDevice() {
    if (!m_d3dDevice) return false;
    
    HRESULT hr = m_d3dDevice->TestCooperativeLevel();
    
    if (hr == D3DERR_DEVICENOTRESET) {
        LOG_INFO("Attempting to reset DirectX device...");
        
        // Reset device with current presentation parameters
        hr = m_d3dDevice->Reset(&m_presentParams);
        if (SUCCEEDED(hr)) {
            LOG_INFO("DirectX device reset successfully");
            
            // Restore render states after reset
            SetupRenderStates();
            
            // Reinitialize sprite batch after device reset
            if (m_spriteBatch) {
                m_spriteBatch->Shutdown();
                if (!m_spriteBatch->Initialize(m_d3dDevice)) {
                    LOG_ERROR("Failed to reinitialize SpriteBatch after device reset");
                    return false;
                }
            }
            
            return true;
        } else {
            LOG_ERROR("Failed to reset DirectX device. HRESULT: " + std::to_string(hr));
            return false;
        }
    } else if (hr == D3DERR_DEVICELOST) {
        // Device is still lost, can't reset yet
        return false;
    }
    
    // Device is not lost
    return true;
}

void D3DRenderer::DrawSprite(Texture2D* texture, const Vector2& position, const Rect& sourceRect, const Color& tint) {
    if (!m_spriteBatch) {
        LOG_ERROR("SpriteBatch not initialized");
        return;
    }
    
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Invalid texture passed to DrawSprite");
        return;
    }
    
    // Use sprite batch to draw with camera transform
    if (!m_spriteBatch->IsInBatch()) {
        if (m_camera) {
            D3DXMATRIX cameraTransform = m_camera->GetTransformMatrix();
            m_spriteBatch->Begin(&cameraTransform);
        } else {
            m_spriteBatch->Begin();
        }
        m_spriteBatch->Draw(texture, position, sourceRect, tint);
        m_spriteBatch->End();
    } else {
        m_spriteBatch->Draw(texture, position, sourceRect, tint);
    }
}

void D3DRenderer::DrawSprite(Texture2D* texture, const Vector2& position, const Color& tint) {
    if (!m_spriteBatch) {
        LOG_ERROR("SpriteBatch not initialized");
        return;
    }
    
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Invalid texture passed to DrawSprite");
        return;
    }
    
    // Use sprite batch to draw with camera transform
    if (!m_spriteBatch->IsInBatch()) {
        if (m_camera) {
            D3DXMATRIX cameraTransform = m_camera->GetTransformMatrix();
            m_spriteBatch->Begin(&cameraTransform);
        } else {
            m_spriteBatch->Begin();
        }
        m_spriteBatch->Draw(texture, position, tint);
        m_spriteBatch->End();
    } else {
        m_spriteBatch->Draw(texture, position, tint);
    }
}

void D3DRenderer::DrawSprite(Texture2D* texture, const Vector2& position, const Rect& sourceRect,
                             bool flipHorizontal, bool flipVertical, const Color& tint) {
    if (!m_spriteBatch) {
        LOG_ERROR("SpriteBatch not initialized");
        return;
    }
    
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Invalid texture passed to DrawSprite");
        return;
    }
    
    // Use sprite batch to draw with camera transform and flipping
    if (!m_spriteBatch->IsInBatch()) {
        if (m_camera) {
            D3DXMATRIX cameraTransform = m_camera->GetTransformMatrix();
            m_spriteBatch->Begin(&cameraTransform);
        } else {
            m_spriteBatch->Begin();
        }
        m_spriteBatch->Draw(texture, position, sourceRect, flipHorizontal, flipVertical, tint);
        m_spriteBatch->End();
    } else {
        m_spriteBatch->Draw(texture, position, sourceRect, flipHorizontal, flipVertical, tint);
    }
}

void D3DRenderer::SetCamera(Camera2D* camera) {
    m_camera = camera;
    if (camera) {
        LOG_INFO("Camera set on renderer");
    } else {
        LOG_INFO("Camera cleared from renderer");
    }
}

void D3DRenderer::ApplyCameraTransform() {
    if (!m_camera || !m_spriteBatch) return;
    
    // Get the camera's transformation matrix
    D3DXMATRIX cameraTransform = m_camera->GetTransformMatrix();
    
    // Apply the transform to the sprite batch
    if (m_spriteBatch->IsInBatch()) {
        // If we're in a batch, we need to end it, set transform, and begin again
        // This is not ideal for performance, but ensures correct rendering
        LOG_WARNING("ApplyCameraTransform called during sprite batch - this may impact performance");
    }
    
    // The transform will be applied when the sprite batch begins
    // For now, we'll store the camera reference and let SpriteBatch handle it
}

void D3DRenderer::ResetCameraTransform() {
    if (!m_spriteBatch) return;
    
    // Reset to identity matrix
    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    
    // This will be handled by the SpriteBatch when it begins/ends
}