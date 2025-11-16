#pragma once
#include "MathTypes.h"

// Axis-Aligned Bounding Box (AABB) for collision detection
// Represents a rectangular collision boundary for game entities
class CollisionBox {
public:
    CollisionBox();
    CollisionBox(float width, float height);
    CollisionBox(float offsetX, float offsetY, float width, float height);
    ~CollisionBox();
    
    // Collision box dimensions
    float width;
    float height;
    
    // Offset from entity position (allows collision box to be different from sprite position)
    Vector2 offset;
    
    // Get the bounds as a Rect given an entity position
    Rect GetBounds(const Vector2& position) const;
    
    // Check if this collision box intersects with another at given positions
    bool Intersects(const Vector2& thisPosition, const CollisionBox& other, const Vector2& otherPosition) const;
    
    // Check if a point is inside this collision box at given position
    bool Contains(const Vector2& thisPosition, const Vector2& point) const;
    
    // Get the center point of the collision box at given position
    Vector2 GetCenter(const Vector2& position) const;
    
    // Get the minimum and maximum points of the AABB
    Vector2 GetMin(const Vector2& position) const;
    Vector2 GetMax(const Vector2& position) const;
    
    // Calculate penetration depth when colliding with another box
    // Returns the minimum translation vector to separate the boxes
    Vector2 GetPenetration(const Vector2& thisPosition, const CollisionBox& other, const Vector2& otherPosition) const;
    
    // Utility methods
    float GetLeft(const Vector2& position) const;
    float GetRight(const Vector2& position) const;
    float GetTop(const Vector2& position) const;
    float GetBottom(const Vector2& position) const;
};
