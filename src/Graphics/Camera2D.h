#pragma once

// Prevent Windows headers from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "../Utils/MathTypes.h"

class Camera2D {
public:
    Camera2D();
    ~Camera2D();

    // Position and movement
    void SetPosition(const Vector2& position);
    void SetPosition(float x, float y);
    Vector2 GetPosition() const { return m_position; }

    void Move(const Vector2& offset);
    void Move(float deltaX, float deltaY);

    // Zoom and scale
    void SetZoom(float zoom);
    float GetZoom() const { return m_zoom; }

    // Viewport
    void SetViewport(int width, int height);
    int GetViewportWidth() const { return m_viewportWidth; }
    int GetViewportHeight() const { return m_viewportHeight; }

    // Following target
    void SetTarget(const Vector2* target);
    void ClearTarget();
    void SetFollowSpeed(float speed) { m_followSpeed = speed; }
    float GetFollowSpeed() const { return m_followSpeed; }

    // Boundaries
    void SetBounds(const Rect& bounds);
    void ClearBounds();
    bool HasBounds() const { return m_hasBounds; }
    Rect GetBounds() const { return m_bounds; }

    // Smooth movement
    void SetSmoothingEnabled(bool enabled) { m_smoothingEnabled = enabled; }
    bool IsSmoothingEnabled() const { return m_smoothingEnabled; }
    void SetSmoothingFactor(float factor) { m_smoothingFactor = factor; }
    float GetSmoothingFactor() const { return m_smoothingFactor; }

    // Update
    void Update(float deltaTime);

    // Transformation
    D3DXMATRIX GetTransformMatrix() const;
    Vector2 ScreenToWorld(const Vector2& screenPos) const;
    Vector2 WorldToScreen(const Vector2& worldPos) const;

    // Utility
    Rect GetVisibleArea() const;
    bool IsPointVisible(const Vector2& point) const;
    bool IsRectVisible(const Rect& rect) const;

private:
    // Position and movement
    Vector2 m_position;
    Vector2 m_targetPosition;
    float m_zoom;

    // Viewport
    int m_viewportWidth;
    int m_viewportHeight;

    // Following
    const Vector2* m_followTarget;
    float m_followSpeed;

    // Boundaries
    bool m_hasBounds;
    Rect m_bounds;

    // Smoothing
    bool m_smoothingEnabled;
    float m_smoothingFactor;

    // Internal methods
    void UpdateFollowTarget(float deltaTime);
    void ApplyBounds();
    Vector2 LerpVector2(const Vector2& a, const Vector2& b, float t);
};
