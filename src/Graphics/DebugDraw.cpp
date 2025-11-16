#include "DebugDraw.h"
#include "../Utils/Logger.h"

DebugDraw::DebugDraw()
    : m_device(nullptr) {
}

DebugDraw::~DebugDraw() {
    Shutdown();
}

bool DebugDraw::Initialize(LPDIRECT3DDEVICE9 device) {
    if (!device) {
        LOG_ERROR("Invalid device passed to DebugDraw::Initialize");
        return false;
    }
    
    m_device = device;
    LOG_INFO("DebugDraw initialized");
    return true;
}

void DebugDraw::Shutdown() {
    m_device = nullptr;
}

void DebugDraw::DrawRect(const Rect& rect, D3DCOLOR color) {
    DrawRect(static_cast<float>(rect.x), static_cast<float>(rect.y), 
             static_cast<float>(rect.w), static_cast<float>(rect.h), color);
}

void DebugDraw::DrawRect(float x, float y, float width, float height, D3DCOLOR color) {
    if (!m_device) return;
    
    // Create vertices for a rectangle (two triangles)
    ColorVertex vertices[6];
    
    // Triangle 1 (top-left, top-right, bottom-left)
    vertices[0] = { x, y, 0.0f, 1.0f, color };
    vertices[1] = { x + width, y, 0.0f, 1.0f, color };
    vertices[2] = { x, y + height, 0.0f, 1.0f, color };
    
    // Triangle 2 (top-right, bottom-right, bottom-left)
    vertices[3] = { x + width, y, 0.0f, 1.0f, color };
    vertices[4] = { x + width, y + height, 0.0f, 1.0f, color };
    vertices[5] = { x, y + height, 0.0f, 1.0f, color };
    
    // Set the flexible vertex format
    m_device->SetFVF(ColorVertex::FVF);
    
    // Disable texture for colored rendering
    m_device->SetTexture(0, nullptr);
    
    // Draw the rectangle
    m_device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(ColorVertex));
}
