#include "Camera2D.h"
#include "Logger.h"
#include "MathTypes.h"
#include <algorithm>
#include <cmath>

Camera2D::Camera2D()
    : m_position(0.0f, 0.0f)
    , m_targetPosition(0.0f, 0.0f)
    , m_zoom(1.0f)
    , m_viewportWidth(800)
    , m_viewportHeight(600)
    , m_followTarget(nullptr)
    , m_followSpeed(5.0f)
    , m_hasBounds(false)
    , m_bounds(0, 0, 0, 0)
    , m_smoothingEnabled(true)
    , m_smoothingFactor(0.1f) {
}

Camera2D::~Camera2D() {
    // Nothing to clean up
}

void Camera2D::SetPosition(const Vector2& position) {
    m_position = position;
    m_targetPosition = position;
    ApplyBounds();
}

void Camera2D::SetPosition(float x, float y) {
    SetPosition(Vector2(x, y));
}

void Camera2D::Move(const Vector2& offset) {
    m_position.x += offset.x;
    m_position.y += offset.y;
    m_targetPosition = m_position;
    ApplyBounds();
}

void Camera2D::Move(float deltaX, float deltaY) {
    Move(Vector2(deltaX, deltaY));
}

void Camera2D::SetZoom(float zoom) {
    // Clamp zoom to reasonable values
    m_zoom = std::max(0.1f, std::min(10.0f, zoom));
}

void Camera2D::SetViewport(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
    ApplyBounds(); // Reapply bounds with new viewport
}

void Camera2D::SetTarget(const Vector2* target) {
    m_followTarget = target;
}

void Camera2D::ClearTarget() {
    m_followTarget = nullptr;
}

void Camera2D::SetBounds(const Rect& bounds) {
    m_bounds = bounds;
    m_hasBounds = true;
    ApplyBounds();
}

void Camera2D::ClearBounds() {
    m_hasBounds = false;
}

void Camera2D::Update(float deltaTime) {
    // Update target following
    if (m_followTarget) {
        UpdateFollowTarget(deltaTime);
    }

    // Apply smooth movement
    if (m_smoothingEnabled) {
        float lerpFactor = 1.0f - std::pow(m_smoothingFactor, deltaTime);
        m_position = LerpVector2(m_position, m_targetPosition, lerpFactor);
    } else {
        m_position = m_targetPosition;
    }

    // Apply bounds
    ApplyBounds();
}

void Camera2D::UpdateFollowTarget(float deltaTime) {
    if (!m_followTarget) return;

    // Calculate desired camera position (center target on screen)
    Vector2 desiredPosition;
    desiredPosition.x = m_followTarget->x - (m_viewportWidth * 0.5f) / m_zoom;
    desiredPosition.y = m_followTarget->y - (m_viewportHeight * 0.5f) / m_zoom;

    if (m_smoothingEnabled) {
        // Smoothly move towards target
        m_targetPosition = desiredPosition;
    } else {
        // Immediately snap to target
        m_targetPosition = desiredPosition;
        m_position = desiredPosition;
    }
}

void Camera2D::ApplyBounds() {
    if (!m_hasBounds) return;

    // Calculate camera bounds based on viewport and zoom
    float halfViewportWidth = (m_viewportWidth * 0.5f) / m_zoom;
    float halfViewportHeight = (m_viewportHeight * 0.5f) / m_zoom;

    // Calculate the effective bounds for camera position
    float minX = static_cast<float>(m_bounds.x) + halfViewportWidth;
    float maxX = static_cast<float>(m_bounds.x + m_bounds.width) - halfViewportWidth;
    float minY = static_cast<float>(m_bounds.y) + halfViewportHeight;
    float maxY = static_cast<float>(m_bounds.y + m_bounds.height) - halfViewportHeight;

    // Only apply bounds if the level is larger than the viewport
    if (maxX > minX) {
        m_position.x = std::max(minX, std::min(maxX, m_position.x));
        m_targetPosition.x = std::max(minX, std::min(maxX, m_targetPosition.x));
    }

    if (maxY > minY) {
        m_position.y = std::max(minY, std::min(maxY, m_position.y));
        m_targetPosition.y = std::max(minY, std::min(maxY, m_targetPosition.y));
    }
}

D3DXMATRIX Camera2D::GetTransformMatrix() const {
    D3DXMATRIX transform;

    // Create transformation matrix for camera
    // 1. Scale by zoom
    // 2. Translate by negative camera position (to move world opposite to camera)
    D3DXVECTOR2 scaling(m_zoom, m_zoom);
    D3DXVECTOR2 translation(-m_position.x * m_zoom, -m_position.y * m_zoom);

    D3DXMatrixTransformation2D(
        &transform,
        nullptr,                                    // scaling center
        0.0f,                                      // scaling rotation
        &scaling,             // scaling
        nullptr,                                   // rotation center
        0.0f,                                     // rotation
        &translation // translation
    );

    return transform;
}

Vector2 Camera2D::ScreenToWorld(const Vector2& screenPos) const {
    Vector2 worldPos;
    worldPos.x = (screenPos.x / m_zoom) + m_position.x;
    worldPos.y = (screenPos.y / m_zoom) + m_position.y;
    return worldPos;
}

Vector2 Camera2D::WorldToScreen(const Vector2& worldPos) const {
    Vector2 screenPos;
    screenPos.x = (worldPos.x - m_position.x) * m_zoom;
    screenPos.y = (worldPos.y - m_position.y) * m_zoom;
    return screenPos;
}

Rect Camera2D::GetVisibleArea() const {
    Vector2 topLeft = ScreenToWorld(Vector2(0, 0));
    Vector2 bottomRight = ScreenToWorld(Vector2(static_cast<float>(m_viewportWidth), static_cast<float>(m_viewportHeight)));

    Rect visibleArea;
    visibleArea.x = static_cast<int>(topLeft.x);
    visibleArea.y = static_cast<int>(topLeft.y);
    visibleArea.width = static_cast<int>(bottomRight.x - topLeft.x);
    visibleArea.height = static_cast<int>(bottomRight.y - topLeft.y);

    return visibleArea;
}

bool Camera2D::IsPointVisible(const Vector2& point) const {
    Rect visibleArea = GetVisibleArea();
    return (point.x >= visibleArea.x &&
            point.x <= visibleArea.x + visibleArea.width &&
            point.y >= visibleArea.y &&
            point.y <= visibleArea.y + visibleArea.height);
}

bool Camera2D::IsRectVisible(const Rect& rect) const {
    Rect visibleArea = GetVisibleArea();

    // Check if rectangles intersect
    return !(rect.x + rect.width < visibleArea.x ||
             rect.x > visibleArea.x + visibleArea.width ||
             rect.y + rect.height < visibleArea.y ||
             rect.y > visibleArea.y + visibleArea.height);
}

Vector2 Camera2D::LerpVector2(const Vector2& a, const Vector2& b, float t) {
    // Clamp t to [0, 1]
    t = std::max(0.0f, std::min(1.0f, t));

    Vector2 result;
    result.x = a.x + (b.x - a.x) * t;
    result.y = a.y + (b.y - a.y) * t;
    return result;
}
