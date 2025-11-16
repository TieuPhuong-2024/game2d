#include "CollisionBox.h"
#include <algorithm>
#include <cmath>

CollisionBox::CollisionBox()
    : width(16.0f)
    , height(16.0f)
    , offset(0.0f, 0.0f) {
}

CollisionBox::CollisionBox(float w, float h)
    : width(w)
    , height(h)
    , offset(0.0f, 0.0f) {
}

CollisionBox::CollisionBox(float offsetX, float offsetY, float w, float h)
    : width(w)
    , height(h)
    , offset(offsetX, offsetY) {
}

CollisionBox::~CollisionBox() {
}

Rect CollisionBox::GetBounds(const Vector2& position) const {
    int x = static_cast<int>(position.x + offset.x);
    int y = static_cast<int>(position.y + offset.y);
    int w = static_cast<int>(width);
    int h = static_cast<int>(height);
    return Rect(x, y, w, h);
}

bool CollisionBox::Intersects(const Vector2& thisPosition, const CollisionBox& other, const Vector2& otherPosition) const {
    float left1 = GetLeft(thisPosition);
    float right1 = GetRight(thisPosition);
    float top1 = GetTop(thisPosition);
    float bottom1 = GetBottom(thisPosition);
    
    float left2 = other.GetLeft(otherPosition);
    float right2 = other.GetRight(otherPosition);
    float top2 = other.GetTop(otherPosition);
    float bottom2 = other.GetBottom(otherPosition);
    
    // AABB intersection test
    return !(right1 <= left2 || left1 >= right2 || bottom1 <= top2 || top1 >= bottom2);
}

bool CollisionBox::Contains(const Vector2& thisPosition, const Vector2& point) const {
    float left = GetLeft(thisPosition);
    float right = GetRight(thisPosition);
    float top = GetTop(thisPosition);
    float bottom = GetBottom(thisPosition);
    
    return (point.x >= left && point.x < right && point.y >= top && point.y < bottom);
}

Vector2 CollisionBox::GetCenter(const Vector2& position) const {
    return Vector2(
        position.x + offset.x + width * 0.5f,
        position.y + offset.y + height * 0.5f
    );
}

Vector2 CollisionBox::GetMin(const Vector2& position) const {
    return Vector2(
        position.x + offset.x,
        position.y + offset.y
    );
}

Vector2 CollisionBox::GetMax(const Vector2& position) const {
    return Vector2(
        position.x + offset.x + width,
        position.y + offset.y + height
    );
}

Vector2 CollisionBox::GetPenetration(const Vector2& thisPosition, const CollisionBox& other, const Vector2& otherPosition) const {
    if (!Intersects(thisPosition, other, otherPosition)) {
        return Vector2::Zero();
    }
    
    Vector2 thisMin = GetMin(thisPosition);
    Vector2 thisMax = GetMax(thisPosition);
    Vector2 otherMin = other.GetMin(otherPosition);
    Vector2 otherMax = other.GetMax(otherPosition);
    
    // Calculate overlap on each axis
    float overlapX1 = thisMax.x - otherMin.x;  // Penetration from right
    float overlapX2 = otherMax.x - thisMin.x;  // Penetration from left
    float overlapY1 = thisMax.y - otherMin.y;  // Penetration from bottom
    float overlapY2 = otherMax.y - thisMin.y;  // Penetration from top
    
    // Find minimum overlap on each axis
    float minOverlapX = (std::min)(overlapX1, overlapX2);
    float minOverlapY = (std::min)(overlapY1, overlapY2);
    
    // Return the minimum translation vector (smallest penetration)
    if (minOverlapX < minOverlapY) {
        // Separate horizontally
        return Vector2((overlapX1 < overlapX2) ? -minOverlapX : minOverlapX, 0.0f);
    } else {
        // Separate vertically
        return Vector2(0.0f, (overlapY1 < overlapY2) ? -minOverlapY : minOverlapY);
    }
}

float CollisionBox::GetLeft(const Vector2& position) const {
    return position.x + offset.x;
}

float CollisionBox::GetRight(const Vector2& position) const {
    return position.x + offset.x + width;
}

float CollisionBox::GetTop(const Vector2& position) const {
    return position.y + offset.y;
}

float CollisionBox::GetBottom(const Vector2& position) const {
    return position.y + offset.y + height;
}
